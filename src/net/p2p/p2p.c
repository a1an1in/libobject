/**
 * @file p2p.c
 * @Synopsis  P2P 对外接口实现：薄层封装 Stun(peer 客户端) + 中心服务器 p2p_server_run
 *
 * 建链/撮合逻辑集中在 Stun（stun/Stun.c）：上线(REG)、被叫收到 INVITE 自动
 * ACCEPT、主叫 connect_peer 发 CALL、收到 PEER/INVITE 后自动打洞（复用保活线程发
 * PUNCH，收到对端包后内部上报 PUNCHOK），服务器确认双方打洞都成功回 CONNECTED，
 * Stun 置 connected。本层只：
 *  - open：创建 Stun、连信令、采址/探测 NAT、REG 上线；
 *  - connect_peer(异步)：发起 CALL；结果查询 p2p_session_is_connected；
 *  - recv：仅 connected 后的业务数据才上抛（打洞前消息由 Stun 内置消化）；
 *  - close：停保活 + BYE 下线。
 *
 * 本文件是 p2p 模块内部实现，打洞客户端头(stun/Stun.h)只在 .c 里引用。
 *
 * @author Zoo
 * @date 2026-08-13
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

/* 薄层节点：持有打洞客户端(Stun)，recv 仅业务数据 */
struct p2p_session_s {
    Stun *stun;
    p2p_recv_fn recv;
    void *opaque;
    char id[32];
    char peer_id[32];
    int  alive;
};

/* Stun 业务收包回调 -> 用户 recv（仅在 connected 后；打洞前消息不外发） */
static int __p2p_recv(Stun *stun, uint8_t *buf, int len)
{
    p2p_session_t *s = (p2p_session_t *)stun->opaque;

    if (s != NULL && stun->connected && s->recv != NULL) {
        return s->recv(s->opaque, buf, len);
    }
    return 0;
}

int p2p_session_open(p2p_session_t **out, p2p_recv_fn recv,
                     const p2p_cfg_t *cfg, void *opaque)
{
    allocator_t *allocator = allocator_get_default_instance();
    p2p_session_t *s = NULL;
    Stun *stun = NULL;
    int ret = -1;

    if (out == NULL || recv == NULL || cfg == NULL || cfg->id == NULL ||
        cfg->signal_host == NULL || cfg->signal_service == NULL ||
        cfg->stun_host == NULL || cfg->stun_service == NULL) {
        return -1;
    }

    s = (p2p_session_t *)calloc(1, sizeof(*s));
    if (s == NULL) {
        return -1;
    }
    s->recv = recv;
    s->opaque = opaque;
    snprintf(s->id, sizeof(s->id), "%s", cfg->id);
    if (cfg->peer_id != NULL) {
        snprintf(s->peer_id, sizeof(s->peer_id), "%s", cfg->peer_id);
    }

    stun = object_new(allocator, "Stun", NULL);
    if (stun == NULL) {
        free(s);
        return -1;
    }
    s->stun = stun;
    stun->opaque = s;
    if (cfg->local_host != NULL)    stun->local_host    = (char *)cfg->local_host;
    if (cfg->local_service != NULL) stun->local_service = (char *)cfg->local_service;
    if (cfg->interval_ms > 0)       stun->keepalive_interval_ms = cfg->interval_ms;
    if (stun->set_recv_callback(stun, __p2p_recv) <= 0) {
        goto fail;
    }

    if (stun->connect(stun, (char *)cfg->signal_host, (char *)cfg->signal_service) < 0) {
        goto fail;
    }
    if (cfg->stun2_host != NULL && cfg->stun2_service != NULL) {
        if (stun->probe(stun, (char *)cfg->stun_host, (char *)cfg->stun_service,
                        (char *)cfg->stun2_host, (char *)cfg->stun2_service) < 0) {
            goto fail;
        }
    } else {
        if (stun->discovery(stun, (char *)cfg->stun_host, (char *)cfg->stun_service) < 0) {
            goto fail;
        }
    }
    if (stun->register_addr(stun, (char *)cfg->id) < 0) {
        goto fail;
    }

    s->alive = 1;
    *out = s;
    dbg_str(DBG_VIP, "[%s] online, peer_id=%s", s->id, s->peer_id);
    return 0;

fail:
    object_destroy(stun);
    s->stun = NULL;
    free(s);
    return -1;
}

int p2p_session_connect(p2p_session_t *s)
{
    int ret;

    if (s == NULL || s->stun == NULL || !s->alive ||
        s->id[0] == 0 || s->peer_id[0] == 0) {
        dbg_str(DBG_ERROR, "p2p_session_connect: bad state (alive=%d id=%s peer=%s)",
                s ? s->alive : -9, s ? s->id : "", s ? s->peer_id : "");
        return -1;
    }
    if (s->stun->connect_peer == NULL) {
        dbg_str(DBG_ERROR, "p2p_session_connect: connect_peer not bound");
        return -1;
    }
    ret = s->stun->connect_peer(s->stun, s->peer_id);
    dbg_str(DBG_INFO, "[%s] session_connect ret=%d", s->id, ret);
    return ret;
}

int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len)
{
    if (s == NULL || s->stun == NULL || data == NULL || len < 0) {
        return -1;
    }
    if (!s->alive || !s->stun->connected) {
        return -1;   /* 尚未打通/已关闭 */
    }
    return s->stun->send(s->stun, (void *)data, len);
}

int p2p_session_is_connected(p2p_session_t *s)
{
    if (s == NULL || s->stun == NULL || !s->alive) {
        return -1;
    }
    return (s->stun->connected) ? 0 : -1;
}

int p2p_session_is_alive(p2p_session_t *s)
{
    if (s == NULL || !s->alive || s->stun == NULL) {
        return -1;
    }
    return 0;
}

int p2p_session_close(p2p_session_t *s)
{
    char buf[64];

    if (s == NULL) {
        return -1;
    }
    s->alive = 0;
    if (s->stun != NULL) {
        if (s->stun->keepalive_stop != NULL) {
            s->stun->keepalive_stop(s->stun);
        }
        /* 优雅下线：经信令 client(server_client) 通知服务器删除本端在线表项 */
        if (s->id[0] != 0 && s->stun->server_client != NULL &&
            s->stun->signal_host != NULL && s->stun->signal_service != NULL) {
            client_connect(s->stun->server_client, s->stun->signal_host,
                           s->stun->signal_service);
            snprintf(buf, sizeof(buf), "BYE %s\n", s->id);
            client_send(s->stun->server_client, buf, (int)strlen(buf), 0);
            dbg_str(DBG_INFO, "[%s] BYE sent, offline from server", s->id);
        }
        object_destroy(s->stun);
        s->stun = NULL;
    }
    free(s);

    return 0;
}

/*
 * 运行中心服务器（P2p_Server）：信令 REG/GET/BYE + 撮合 CALL/ACCEPT/REJECT/
 * PUNCHOK/CONNECTED + STUN 回显（预留 TURN 中继），阻塞直到 Ctrl+C(SIGINT)。
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
