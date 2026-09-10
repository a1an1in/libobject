/**
 * @file P2p_Server.c
 * @Synopsis  STUN/信令中心服务器：Binding 回显 + 多会话撮合(会话地址交换)
 *
 * v3 模型：每个 stun 节点连上后 SIGNIN 登记自己（服务器记其信令源地址，用于投递
 * INVITE/MATCH/CONNECTED/NOPEER）。打洞目标(本会话地址)不提前上报，而在撮合信令中
 * 交换：
 *   CALL <caller> <callee> <caller_sess_host> <caller_sess_port>
 *      -> 服务器查 callee 在线，向 callee 信令地址发 INVITE(带 caller 会话地址)；
 *   ACCEPT <callee> <caller> <callee_sess_host> <callee_sess_port>
 *      -> 服务器向 caller 信令地址回 MATCH(带 callee 会话地址)；
 *   PUNCHOK <id> <peer_id>
 *      -> 服务器按 pending 判定主/被叫打洞成功，双 ok 回双方 CONNECTED；
 *   SIGNOUT <stun_id> 下线。
 * 多路：pending key = "caller|callee"，同一节点可同时作主叫连多个、作被叫被多个连。
 *
 * @author Zoo
 * @date 2026-09-09
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <libobject/concurrent/work_task.h>
#include "P2p_Server.h"

static int __stun_server_callback(void *task);

/* 撮合 key = "caller|callee" */
static void __call_key(char *out, int out_len, const char *caller, const char *callee)
{
    snprintf(out, out_len, "%s|%s", caller, callee);
}

static int __construct(P2p_Server *server, char *init_str)
{
    allocator_t *allocator = server->obj.allocator;
    int ret = 0, trustee_flag = 1;
    int value_type = VALUE_TYPE_STRUCT_POINTER;

    TRY {
        pthread_mutex_init(&server->lock, NULL);

        server->stuns = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(server->stuns == NULL, -1);
        server->stuns->set_cmp_func(server->stuns, string_key_cmp_func);
        server->stuns->set(server->stuns, "/Map/trustee_flag", &trustee_flag);
        server->stuns->set(server->stuns, "/Map/value_type", &value_type);

        server->pending = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(server->pending == NULL, -1);
        server->pending->set_cmp_func(server->pending, string_key_cmp_func);
        server->pending->set(server->pending, "/Map/trustee_flag", &trustee_flag);
        server->pending->set(server->pending, "/Map/value_type", &value_type);
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(P2p_Server *server)
{
    if (server->client != NULL) {
        client_destroy(server->client);
        server->client = NULL;
    }
    pthread_mutex_destroy(&server->lock);

    if (server->stuns != NULL) {
        object_destroy(server->stuns);   /* Map destroy 释放 value */
        server->stuns = NULL;
    }
    if (server->pending != NULL) {
        object_destroy(server->pending);
        server->pending = NULL;
    }
    return 0;
}

/*
 * UDP client 统一收包回调：处理 STUN Binding + 信令文本
 * (SIGNIN/SIGNOUT/CALL/ACCEPT/PUNCHOK)。
 */
static int __stun_server_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    P2p_Server *server = (P2p_Server *)t->opaque;
    allocator_t *allocator;
    Socket *socket;
    Map *stuns;
    Map *pending;
    stun_header_t *h;
    stun_node_t *n = NULL, *new_node;
    stun_node_t *callee = NULL;
    p2p_server_call_t *call = NULL, *new_call;
    uint8_t resp[512];
    int resp_len = 0;
    char id[32], from_id[32], to_id[32];
    char p1[32], p2[32];
    char key[96];
    char sport[16];
    char sess_host[64];
    int sess_port = 0, n_arg;
    int sess_nat = 0;

    if (server == NULL || t->buf_len <= 0) {
        return 0;
    }

    allocator = server->obj.allocator;
    socket = server->client->socket;
    stuns = server->stuns;
    pending = server->pending;
    h = (stun_header_t *)t->buf;

    dbg_str(DBG_DETAIL, "server received %d bytes from %s:%s", t->buf_len,
            t->remote_host, t->remote_service);

    if (t->buf_len >= (int)sizeof(stun_header_t) &&
        ntohl(h->magic_cookie) == STUN_MAGIC_COOKIE) {
        /* STUN Binding Request -> 回显公网映射地址 */
        stun_header_t *rh = (stun_header_t *)resp;
        uint8_t *attr = resp + sizeof(stun_header_t);
        uint16_t port;
        uint32_t cookie = ntohl(h->magic_cookie);
        uint32_t cookie_be = htonl(cookie);
        uint8_t ip[4];

        if (inet_pton(AF_INET, t->remote_host, ip) != 1) {
            return 0;
        }
        memset(resp, 0, sizeof(resp));
        rh->msgtype = htons(STUN_BINDRESP);
        rh->msglen = htons(12);
        rh->magic_cookie = h->magic_cookie;
        memcpy(rh->transaction_id, h->transaction_id, 12);
        attr[0] = 0x00; attr[1] = 0x20;
        attr[2] = 0x00; attr[3] = 0x08;
        attr[4] = 0x00; attr[5] = 0x01;
        port = (uint16_t)atoi(t->remote_service) ^ (cookie >> 16);
        attr[6] = (uint8_t)(port >> 8);
        attr[7] = (uint8_t)(port & 0xff);
        attr[8]  = ip[0] ^ ((uint8_t *)&cookie_be)[0];
        attr[9]  = ip[1] ^ ((uint8_t *)&cookie_be)[1];
        attr[10] = ip[2] ^ ((uint8_t *)&cookie_be)[2];
        attr[11] = ip[3] ^ ((uint8_t *)&cookie_be)[3];
        resp_len = (int)sizeof(stun_header_t) + 12;
        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
        return 0;
    }

    ((char *)t->buf)[t->buf_len] = 0;

    if (sscanf((char *)t->buf, "SIGNIN %31s", id) == 1) {
        /* 登记/更新 stun 节点：记其信令源地址（INVITE/CONNECTED/MATCH 投递用） */
        dbg_str(DBG_INFO, "SIGNIN: %s registered (src %s:%s)", id,
                t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        n = NULL;
        stuns->search(stuns, (void *)id, &n);
        if (n == NULL) {
            new_node = (stun_node_t *)allocator_mem_alloc(allocator,
                                                          sizeof(stun_node_t));
            if (new_node != NULL) {
                snprintf(new_node->id, sizeof(new_node->id), "%s", id);
                snprintf(new_node->signal_host, sizeof(new_node->signal_host),
                         "%s", t->remote_host);
                new_node->signal_port = atoi(t->remote_service);
                stuns->add(stuns, new_node->id, new_node);
            }
        } else {
            snprintf(n->signal_host, sizeof(n->signal_host), "%s", t->remote_host);
            n->signal_port = atoi(t->remote_service);
        }
        pthread_mutex_unlock(&server->lock);
        strcpy((char *)resp, "OK\n");
        resp_len = 3;
        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
    } else if (sscanf((char *)t->buf, "SIGNOUT %31s", id) == 1) {
        dbg_str(DBG_INFO, "SIGNOUT: %s offline (src %s:%s)", id,
                t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        n = NULL;
        stuns->search(stuns, (void *)id, &n);
        if (n != NULL) {
            stuns->del(stuns, (void *)id);
            allocator_mem_free(allocator, n);
        }
        pthread_mutex_unlock(&server->lock);
    } else if ((n_arg = sscanf((char *)t->buf, "CALL %31s %31s %63s %d %d",
                               from_id, to_id, sess_host, &sess_port,
                               &sess_nat)) >= 4) {
        if (n_arg < 5) {
            sess_nat = 0;
        }
        dbg_str(DBG_INFO, "CALL: %s calls %s session address=%s:%d nat=%d (src %s:%s)",
                from_id, to_id, sess_host, sess_port, sess_nat,
                t->remote_host, t->remote_service);

        pthread_mutex_lock(&server->lock);
        callee = NULL;
        stuns->search(stuns, (void *)to_id, &callee);
        if (callee == NULL) {
            /* 被叫不在线 -> 回主叫 NOPEER */
            pthread_mutex_unlock(&server->lock);
            snprintf((char *)resp, sizeof(resp), "NOPEER %s\n", to_id);
            resp_len = (int)strlen((char *)resp);
            socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
            return 0;
        }

        new_call = (p2p_server_call_t *)allocator_mem_alloc(allocator,
                                                            sizeof(p2p_server_call_t));
        if (new_call != NULL) {
            __call_key(key, sizeof(key), from_id, to_id);
            snprintf(new_call->key, sizeof(new_call->key), "%s", key);
            snprintf(new_call->from_id, sizeof(new_call->from_id), "%s", from_id);
            /* 主叫信令源地址：撮合回执投递 */
            snprintf(new_call->from_host, sizeof(new_call->from_host),
                     "%s", t->remote_host);
            new_call->from_port = atoi(t->remote_service);
            snprintf(new_call->to_id, sizeof(new_call->to_id), "%s", to_id);
            /* to_host/port 被叫会话地址，ACCEPT 后填 */
            new_call->to_host[0] = 0;
            new_call->to_port = 0;
            new_call->from_nat = sess_nat;
            new_call->to_nat = 0;
            new_call->caller_ok = 0;
            new_call->callee_ok = 0;

            call = NULL;
            pending->search(pending, (void *)new_call->key, &call);
            if (call != NULL) {
                pending->del(pending, (void *)new_call->key);
                allocator_mem_free(allocator, call);
            }
            pending->add(pending, new_call->key, new_call);
        }
        pthread_mutex_unlock(&server->lock);

        /* 通知被叫 INVITE：带主叫 id + 主叫会话(打洞目标)地址 */
        snprintf((char *)resp, sizeof(resp), "INVITE %s %s %d %d\n",
                 from_id, sess_host, sess_port, sess_nat);
        resp_len = (int)strlen((char *)resp);
        pthread_mutex_lock(&server->lock);
        callee = NULL;
        stuns->search(stuns, (void *)to_id, &callee);
        if (callee != NULL) {
            snprintf(sport, sizeof(sport), "%d", callee->signal_port);
            socket->sendto(socket, resp, resp_len, 0, callee->signal_host, sport);
        }
        pthread_mutex_unlock(&server->lock);
    } else if ((n_arg = sscanf((char *)t->buf, "ACCEPT %31s %31s %63s %d %d",
                               to_id, from_id, sess_host, &sess_port,
                               &sess_nat)) >= 4) {
        if (n_arg < 5) {
            sess_nat = 0;
        }
        dbg_str(DBG_INFO, "ACCEPT: %s accepts %s session address=%s:%d nat=%d",
                to_id, from_id, sess_host, sess_port, sess_nat);
        pthread_mutex_lock(&server->lock);
        __call_key(key, sizeof(key), from_id, to_id);
        call = NULL;
        pending->search(pending, (void *)key, &call);
        if (call != NULL) {
            snprintf(call->to_host, sizeof(call->to_host), "%s", sess_host);
            call->to_port = sess_port;
            call->to_nat = sess_nat;
            /* 撮合回执给主叫：MATCH 带被叫 id + 被叫会话地址 + 被叫 nat */
            snprintf(sport, sizeof(sport), "%d", call->from_port);
            snprintf((char *)resp, sizeof(resp), "MATCH %s %s %d %d\n",
                     call->to_id, call->to_host, call->to_port, call->to_nat);
            resp_len = (int)strlen((char *)resp);
            socket->sendto(socket, resp, resp_len, 0, call->from_host, sport);
        }
        pthread_mutex_unlock(&server->lock);
    } else if (sscanf((char *)t->buf, "PUNCHOK %31s %31s", p1, p2) == 2) {
        /* p1 上报已成功打洞到 p2；主/被叫都上报才判定两端 ok */
        pthread_mutex_lock(&server->lock);
        call = NULL;
        __call_key(key, sizeof(key), p1, p2);
        pending->search(pending, (void *)key, &call);
        if (call != NULL) {
            call->caller_ok = 1;   /* p1=主叫 */
        } else {
            call = NULL;
            __call_key(key, sizeof(key), p2, p1);
            pending->search(pending, (void *)key, &call);
            if (call != NULL) {
                call->callee_ok = 1;   /* p1=被叫 */
            }
        }
        if (call != NULL && call->caller_ok && call->callee_ok) {
            dbg_str(DBG_INFO, "CONNECTED: both punched ok (%s<->%s)",
                    call->from_id, call->to_id);
            /* 主叫 CONNECTED <to_id>，投主叫信令地址 */
            snprintf(sport, sizeof(sport), "%d", call->from_port);
            snprintf((char *)resp, sizeof(resp), "CONNECTED %s\n", call->to_id);
            socket->sendto(socket, resp, (int)strlen((char *)resp), 0,
                           call->from_host, sport);
            /* 被叫 CONNECTED <from_id>，投被叫登记信令地址 */
            n = NULL;
            stuns->search(stuns, (void *)call->to_id, &n);
            if (n != NULL) {
                snprintf(sport, sizeof(sport), "%d", n->signal_port);
                snprintf((char *)resp, sizeof(resp), "CONNECTED %s\n", call->from_id);
                socket->sendto(socket, resp, (int)strlen((char *)resp), 0,
                               n->signal_host, sport);
            }
            pending->del(pending, (void *)call->key);
            allocator_mem_free(allocator, call);
        }
        pthread_mutex_unlock(&server->lock);
    } else {
        /* 忽略未知命令；n_arg 引用避免未用告警 */
        n_arg = 0;
        (void)n_arg;
    }

    return 0;
}

static int __start(P2p_Server *server, char *host, char *service)
{
    allocator_t *allocator = server->obj.allocator;
    int ret = 0;

    TRY {
        server->client = client(allocator, CLIENT_TYPE_INET_UDP, host, service);
        THROW_IF(server->client == NULL, -1);
        EXEC(client_trustee(server->client, NULL, __stun_server_callback, server));
        dbg_str(NET_SUC, "stun server listening on %s:%s", host, service);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun server start failed, ret=%d", ret);
    }
    return ret;
}

static int __stop(P2p_Server *server)
{
    int ret = 0;

    TRY {
        if (server->client != NULL) {
            client_destroy(server->client);
            server->client = NULL;
        }
    } CATCH (ret) {
    }
    return ret;
}

static class_info_entry_t stun_server_class_info[] = {
    Init_Obj___Entry(0, Obj, obj),
    Init_Nfunc_Entry(1, P2p_Server, construct, __construct),
    Init_Nfunc_Entry(2, P2p_Server, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, P2p_Server, set, NULL),
    Init_Vfunc_Entry(4, P2p_Server, get, NULL),
    Init_Vfunc_Entry(5, P2p_Server, start, __start),
    Init_Vfunc_Entry(6, P2p_Server, stop, __stop),
    Init_End___Entry(7, P2p_Server),
};
REGISTER_CLASS(P2p_Server, stun_server_class_info);
