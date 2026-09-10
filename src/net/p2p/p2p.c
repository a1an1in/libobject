/**
 * @file p2p.c
 * @Synopsis  P2P 对外接口实现（node + session 两层，多会话）——薄层封装 Stun
 *
 * 概念：会话与信令全部由 Stun 管理。p2p 只：
 *  - p2p_node：持有 Stun；create 建 Stun+连信令+SIGNIN(stun_id)，close 下线+销毁。
 *  - p2p_session：句柄 = Stun 内 stun_session（Stun 管理其生命周期与并发多路）；
 *    p2p 不持有会话表、不分配会话内存，只做中转：create 转 Stun.call、send/close/
 *    is_connected 转 Stun。
 *  - 收包统一经 Stun 节点级 recv_callback(stun, session, data, len) 上抛，p2p 桥接
 *    为带 session 句柄的用户 recv；主叫与被叫(自动)会话都能被覆盖。
 *
 * @author Zoo
 * @date 2026-09-09
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/net/p2p/p2p.h>
#include "stun/Stun.h"
#include "P2p_Server.h"

struct p2p_node_s {
    Stun *stun;
    p2p_recv_fn recv;      /* 节点默认业务收包回调 */
    void *opaque;          /* 传给 recv 的上下文 */
    char stun_id[32];
    int  alive;
};

/* Stun 业务收包回调（带 session） -> 用户 recv：p2p_session 句柄即 stun_session */
static int __p2p_recv(Stun *stun, stun_session_t *session,
                      uint8_t *buf, int len)
{
    p2p_node_t *node = (p2p_node_t *)stun->opaque;

    if (node == NULL || node->recv == NULL || buf == NULL || len < 0) {
        return 0;
    }
    return node->recv(node->opaque, (p2p_session_t *)session, buf, len);
}

int p2p_node_create(p2p_node_t **out, p2p_recv_fn recv,
                    const p2p_cfg_t *cfg, void *opaque)
{
    allocator_t *allocator = allocator_get_default_instance();
    p2p_node_t *node = NULL;
    Stun *stun = NULL;

    if (out == NULL || cfg == NULL || cfg->stun_id == NULL ||
        cfg->signal_host == NULL || cfg->signal_service == NULL) {
        return -1;
    }

    node = (p2p_node_t *)calloc(1, sizeof(*node));
    if (node == NULL) {
        return -1;
    }
    node->recv = recv;
    node->opaque = opaque;
    snprintf(node->stun_id, sizeof(node->stun_id), "%s", cfg->stun_id);

    stun = object_new(allocator, "Stun", NULL);
    if (stun == NULL) {
        free(node);
        return -1;
    }
    node->stun = stun;
    stun->opaque = node;
    if (cfg->local_host != NULL)    stun->local_host    = (char *)cfg->local_host;
    if (cfg->local_service != NULL) stun->local_service = (char *)cfg->local_service;
    if (cfg->interval_ms > 0)       stun->keepalive_interval_ms = cfg->interval_ms;

    if (stun->connect(stun, (char *)cfg->signal_host,
                      (char *)cfg->signal_service) < 0) {
        goto fail;
    }
    if (cfg->stun_host != NULL && cfg->stun_service != NULL) {
        stun->set_stun_server(stun, (char *)cfg->stun_host,
                              (char *)cfg->stun_service);
    }
    if (cfg->stun2_host != NULL && cfg->stun2_service != NULL) {
        stun->stun2_host    = (char *)cfg->stun2_host;
        stun->stun2_service = (char *)cfg->stun2_service;
    }
    if (stun->set_recv_callback(stun, __p2p_recv) < 0) {
        goto fail;
    }
    if (stun->signin(stun, (char *)cfg->stun_id) < 0) {
        goto fail;
    }

    node->alive = 1;
    *out = node;
    dbg_str(DBG_VIP, "%s node online", node->stun_id);
    return 0;

fail:
    object_destroy(stun);
    node->stun = NULL;
    free(node);
    return -1;
}

int p2p_node_close(p2p_node_t *node)
{
    if (node == NULL) {
        return -1;
    }
    if (!node->alive) {
        free(node);
        return 0;
    }
    node->alive = 0;
    if (node->stun != NULL) {
        if (node->stun->signout != NULL) {
            node->stun->signout(node->stun);
        }
        object_destroy(node->stun);   /* 析构：清空全部会话 + 关信令口 */
        node->stun = NULL;
    }
    free(node);
    return 0;
}

int p2p_node_is_alive(p2p_node_t *node)
{
    return (node != NULL && node->alive) ? 0 : -1;
}

int p2p_session_create(p2p_node_t *node, const char *remote_stun_id,
                       p2p_session_t **out)
{
    stun_session_t *ss = NULL;

    if (node == NULL || node->stun == NULL || !node->alive ||
        remote_stun_id == NULL || out == NULL) {
        dbg_str(DBG_ERROR, "p2p_session_create: bad state");
        return -1;
    }
    if (node->stun->create_session(node->stun, (char *)remote_stun_id, 0,
                                   &ss) < 0 || ss == NULL) {
        return -1;
    }
    *out = (p2p_session_t *)ss;
    dbg_str(DBG_INFO, "%s created session to %s",
            node->stun_id, remote_stun_id);
    return 0;
}

int p2p_session_config(p2p_session_t *s, p2p_recv_fn recv, void *opaque)
{
    stun_session_t *ss = (stun_session_t *)s;
    p2p_node_t *node;

    if (ss == NULL || ss->stun == NULL) {
        return -1;
    }
    node = (p2p_node_t *)ss->stun->opaque;
    if (node == NULL) {
        return -1;
    }
    if (recv != NULL) {
        node->recv = recv;
    }
    if (opaque != NULL) {
        node->opaque = opaque;
    }
    return 0;
}

int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len)
{
    stun_session_t *ss = (stun_session_t *)s;

    if (ss == NULL || ss->stun == NULL || data == NULL || len < 0) {
        return -1;
    }
    return ss->stun->send_session_data(ss->stun, ss->remote_id, (void *)data, len);
}

int p2p_session_is_connected(p2p_session_t *s)
{
    stun_session_t *ss = (stun_session_t *)s;

    if (ss == NULL || ss->stun == NULL) {
        return -1;
    }
    return ss->stun->is_connected(ss->stun, ss->remote_id);
}

int p2p_session_close(p2p_session_t *s)
{
    stun_session_t *ss = (stun_session_t *)s;

    if (ss == NULL || ss->stun == NULL) {
        return -1;
    }
    return ss->stun->close_session(ss->stun, ss->remote_id);
}

/*
 * 运行中心服务器（P2p_Server）：信令 SIGNIN/SIGNOUT 登记 + CALL/INVITE/ACCEPT/PUNCHOK 撮合
 * + STUN 回显（预留 TURN 中继），阻塞直到 Ctrl+C(SIGINT)。
 */
int p2p_server_run(const char *host, const char *service)
{
    allocator_t *allocator = allocator_get_default_instance();
    P2p_Server *server = NULL;
    struct event_base *event_base;
    int ret = -1;

    if (host == NULL || service == NULL) {
        return -1;
    }

    server = object_new(allocator, "P2p_Server", NULL);
    if (server == NULL) {
        return -1;
    }
    ret = server->start(server, (char *)host, (char *)service);
    dbg_str(DBG_INFO, "p2p server start ret=%d", ret);
    if (ret < 0) {   /* >=0 即成功（P2p_Server.__start 用 TRY/CATCH，成功返回 1） */
        dbg_str(DBG_ERROR, "p2p server start failed on %s:%s", host, service);
        object_destroy(server);
        return -1;
    }
    dbg_str(DBG_INFO, "p2p server running on %s:%s, Ctrl+C to stop", host, service);

    event_base = event_base_get_default_instance();
    while (event_base != NULL && event_base->eb->break_flag == 0) {
        sleep(1);
    }
    dbg_str(DBG_INFO, "p2p server stopped by Ctrl+C");

    object_destroy(server);
    return 0;
}
