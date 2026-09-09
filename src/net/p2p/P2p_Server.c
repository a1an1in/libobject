/**
 * @file P2p_Server.c
 * @Synopsis  STUN 服务器（RFC 5389 Binding + 地址簿/信令）
 *
 * 说明：当前 P2p_Server 一台同时支持两类能力：
 *   1) STUN：收到 Binding 请求时回显观测到的源地址(XOR-MAPPED-ADDRESS)，供
 *      peer 做“我是谁”的公网地址发现(discovery)；
 *   2) 信令：文本 REG/GET 地址簿，REG <id> [<host> <port>] 登记(支持公告地址
 *      与观测源地址两种)，GET <id> 返回对端地址供打洞。
 *
 * 使用现状（备注）：在“公共 STUN 采址 + 轻量信令交换”的拆分架构中，本服务器
 * 目前只被当作【信令(REG/GET)】使用，discovery(公网地址发现)走公共 STUN——
 * 原因是 hairpin 问题：若本服务器与某个 peer 在同一个 NAT 之后，
 *   - peer 访问服务器的外网映射地址需该 NAT 支持 hairpin(多数不支持，连不上)；
 *   - peer 访问服务器的内网地址则服务器只看到 peer 的内网源，取不到其外网地址。
 * 因此同 NAT 的 peer 无法用“与它同 NAT 的服务器”发现自己的公网地址，只有让
 * discovery 指向真正在公网、peer 能外拨到达的 STUN(如公共 STUN)才拿得到真实
 * 公网映射。故本服务器在拆分模式下仅承担 REG/GET 地址交换。
 *
 * @author Zoo
 * @date 2026-08-13
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <libobject/concurrent/work_task.h>
#include "P2p_Server.h"
#include "stun/Stun.h"   /* STUN_NAT_TYPE_* 枚举 */

static int __stun_server_callback(void *task);

static int __construct(P2p_Server *server, char *init_str)
{
    allocator_t *allocator = server->obj.allocator;
    int ret = 0, trustee_flag = 1;
    int value_type = VALUE_TYPE_STRUCT_POINTER;

    TRY {
        pthread_mutex_init(&server->lock, NULL);

        server->peers = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(server->peers == NULL, -1);
        server->peers->set_cmp_func(server->peers, string_key_cmp_func);
        server->peers->set(server->peers, "/Map/trustee_flag", &trustee_flag);
        server->peers->set(server->peers, "/Map/value_type", &value_type);

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

    if (server->peers != NULL) {
        /* Map destroy 会释放 value（VALUE_TYPE_STRUCT_POINTER + trustee_flag） */
        object_destroy(server->peers);
        server->peers = NULL;
    }

    if (server->pending != NULL) {
        object_destroy(server->pending);
        server->pending = NULL;
    }

    return 0;
}

/*
 * UDP client 统一收包回调（client_trustee）：
 * 处理来自任意 peer 的报文，源地址由 work_task 的 remote_host/remote_service 提供。
 *  - STUN Binding Request → 回显 XOR-MAPPED-ADDRESS（请求者源地址）
 *  - "REG <id>" → 记录请求者源地址到地址簿，回复 "OK"
 *  - "GET <id>" → 返回对端地址 "PEER host port" 或 "NOPEER"
 */
static int __stun_server_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    P2p_Server *server = (P2p_Server *)t->opaque;
    allocator_t *allocator;
    Socket *socket;
    Map *peers;
    Map *pending;
    stun_header_t *h;
    stun_server_peer_t *p = NULL, *new_peer;
    stun_server_peer_t *pc = NULL;
    p2p_server_call_t *call = NULL, *new_call;
    uint8_t resp[512];
    int resp_len = 0;
    char id[32], from_id[32], to_id[32];
    char p1[32], p2[32];
    char sport[16], aport[16];

    if (server == NULL || t->buf_len <= 0) {
        return 0;
    }

    allocator = server->obj.allocator;
    socket = server->client->socket;
    peers = server->peers;
    pending = server->pending;
    h = (stun_header_t *)t->buf;

    dbg_str(DBG_INFO, "[server] recv %d bytes from %s:%s", t->buf_len,
            t->remote_host, t->remote_service);

    if (t->buf_len >= (int)sizeof(stun_header_t) &&
        ntohl(h->magic_cookie) == STUN_MAGIC_COOKIE) {
        /* STUN Binding Request → 回显公网映射地址 */
        dbg_str(DBG_INFO, "[server] STUN Binding from %s:%s", t->remote_host, t->remote_service);
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
        rh->msglen = htons(12); /* XOR-MAPPED-ADDRESS 属性: 4 头 + 8 值 */
        rh->magic_cookie = h->magic_cookie;
        memcpy(rh->transaction_id, h->transaction_id, 12);

        attr[0] = 0x00; attr[1] = 0x20;          /* type = 0x0020 */
        attr[2] = 0x00; attr[3] = 0x08;          /* len = 8 */
        attr[4] = 0x00;                          /* reserved */
        attr[5] = 0x01;                          /* family = IPv4 */
        port = (uint16_t)atoi(t->remote_service) ^ (cookie >> 16);
        attr[6] = (uint8_t)(port >> 8);
        attr[7] = (uint8_t)(port & 0xff);
        attr[8] = ip[0] ^ ((uint8_t *)&cookie_be)[0];
        attr[9] = ip[1] ^ ((uint8_t *)&cookie_be)[1];
        attr[10] = ip[2] ^ ((uint8_t *)&cookie_be)[2];
        attr[11] = ip[3] ^ ((uint8_t *)&cookie_be)[3];
        resp_len = (int)sizeof(stun_header_t) + 12;

        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
        return 0;
    }

    /* 地址簿文本命令（信令） */
    ((char *)t->buf)[t->buf_len] = 0;
    if (sscanf((char *)t->buf, "REG %31s", id) == 1) {
        char adv_host[64] = {0};
        int adv_port = 0, adv_nat = STUN_NAT_TYPE_UNKNOWN, n, rec_port, rec_nat;
        const char *rec_host;

        /*
         * REG <id>                          : 记录观测源地址（旧流程，STUN+信令一体）
         * REG <id> <host> <port>            : 记录 peer 上报(公告)的公网地址，nat=UNKNOWN
         * REG <id> <host> <port> <nat_type> : 同上，并记录 peer 的 NAT 类型（当前版本）
         */
        n = sscanf((char *)t->buf, "REG %31s %63s %d %d",
                   id, adv_host, &adv_port, &adv_nat);
        if (n >= 3 && adv_host[0] != 0 && adv_port > 0) {
            rec_host = adv_host;
            rec_port = adv_port;
            rec_nat  = (n >= 4) ? adv_nat : STUN_NAT_TYPE_UNKNOWN;
        } else {
            rec_host = t->remote_host;
            rec_port = atoi(t->remote_service);
            rec_nat  = STUN_NAT_TYPE_UNKNOWN;
        }
        dbg_str(DBG_INFO, "[server] REG id=%s record %s:%d nat=%d (src %s:%s)",
                id, rec_host, rec_port, rec_nat, t->remote_host, t->remote_service);

        pthread_mutex_lock(&server->lock);
        p = NULL;
        peers->search(peers, (void *)id, &p);
        if (p == NULL) {
            new_peer = (stun_server_peer_t *)allocator_mem_alloc(allocator,
                                                                 sizeof(stun_server_peer_t));
            if (new_peer != NULL) {
                snprintf(new_peer->id, sizeof(new_peer->id), "%s", id);
                /* 打洞(公告公网)地址：取 REG 报文里 peer 上报的映射 */
                snprintf(new_peer->punch_host, sizeof(new_peer->punch_host), "%s", rec_host);
                new_peer->punch_port = rec_port;
                new_peer->nat_type   = rec_nat;
                /* 信令地址：取 REG 这个包的源（peer 的信令口，服务器必可达） */
                snprintf(new_peer->signal_host, sizeof(new_peer->signal_host),
                         "%s", t->remote_host);
                new_peer->signal_port = atoi(t->remote_service);
                peers->add(peers, new_peer->id, new_peer);
            }
        } else {
            /* 已存在：更新两个地址 */
            snprintf(p->punch_host, sizeof(p->punch_host), "%s", rec_host);
            p->punch_port = rec_port;
            p->nat_type   = rec_nat;
            snprintf(p->signal_host, sizeof(p->signal_host), "%s", t->remote_host);
            p->signal_port = atoi(t->remote_service);
        }
        pthread_mutex_unlock(&server->lock);

        strcpy((char *)resp, "OK\n");
        resp_len = 3;
        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
    } else if (sscanf((char *)t->buf, "GET %31s", id) == 1) {
        dbg_str(DBG_INFO, "[server] GET id=%s from %s:%s", id, t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        p = NULL;
        peers->search(peers, (void *)id, &p);
        if (p != NULL) {
            /* GET 返回打洞(公网)地址：让请求者据它向该 peer 打洞 */
            snprintf((char *)resp, sizeof(resp), "PEER %s %d %d\n",
                     p->punch_host, p->punch_port, p->nat_type);
            resp_len = (int)strlen((char *)resp);
        } else {
            strcpy((char *)resp, "NOPEER\n");
            resp_len = 7;
        }
        pthread_mutex_unlock(&server->lock);
        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
    } else if (sscanf((char *)t->buf, "CALL %31s %31s", from_id, to_id) == 2) {
        const char *caller_host = t->remote_host;
        int caller_port = atoi(t->remote_service);

        dbg_str(DBG_INFO, "[server] CALL from=%s to=%s (src %s:%s)",
                from_id, to_id, t->remote_host, t->remote_service);

        pthread_mutex_lock(&server->lock);
        p = NULL;
        peers->search(peers, (void *)to_id, &p);
        if (p == NULL) {
            pthread_mutex_unlock(&server->lock);
            strcpy((char *)resp, "NOPEER\n");
            resp_len = 7;
            socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);
            return 0;
        }
        /* 发起方打洞(公告)地址（优先取其在册地址，保证对端可 punch 到它） */
        pc = NULL;
        peers->search(peers, (void *)from_id, &pc);
        if (pc != NULL) {
            caller_host = pc->punch_host;
            caller_port = pc->punch_port;
        }
        new_call = (p2p_server_call_t *)allocator_mem_alloc(allocator,
                                                            sizeof(p2p_server_call_t));
        if (new_call != NULL) {
            snprintf(new_call->from_id, sizeof(new_call->from_id), "%s", from_id);
            snprintf(new_call->from_host, sizeof(new_call->from_host), "%s",
                     t->remote_host);
            new_call->from_port = atoi(t->remote_service);
            snprintf(new_call->to_id, sizeof(new_call->to_id), "%s", to_id);
            /* to_* = 被叫打洞地址快照（ACCEPT 后回给发起方打洞用） */
            snprintf(new_call->to_host, sizeof(new_call->to_host), "%s", p->punch_host);
            new_call->to_port = p->punch_port;
            new_call->to_nat = p->nat_type;
            new_call->caller_ok = 0;
            new_call->callee_ok = 0;

            call = NULL;
            pending->search(pending, (void *)from_id, &call);
            if (call != NULL) {
                pending->del(pending, (void *)from_id);
                allocator_mem_free(allocator, call);   /* 替换旧呼叫：释放旧记录 */
            }
            pending->add(pending, new_call->from_id, new_call);
        }
        strcpy((char *)resp, "WAIT\n");
        resp_len = 5;
        socket->sendto(socket, resp, resp_len, 0, t->remote_host, t->remote_service);

        /* 通知被叫(INVITE)走信令地址（服务器必可达，如与服务器同机/LAN）；
         * 文本里给被叫的 caller 地址仍用发起方的打洞(公告公网)地址 */
        snprintf((char *)resp, sizeof(resp), "INVITE %s %s %d\n",
                 from_id, caller_host, caller_port);
        resp_len = (int)strlen((char *)resp);
        if (p->signal_host[0] != 0) {
            snprintf(sport, sizeof(sport), "%d", p->signal_port);
            socket->sendto(socket, resp, resp_len, 0, p->signal_host, sport);
        } else {   /* 旧单口兜底 */
            snprintf(sport, sizeof(sport), "%d", p->punch_port);
            socket->sendto(socket, resp, resp_len, 0, p->punch_host, sport);
        }
        pthread_mutex_unlock(&server->lock);
    } else if (sscanf((char *)t->buf, "ACCEPT %31s", from_id) == 1) {
        /* 被叫 B 接受邀请(ACCEPT)，但≠成功：服务器把 B 的打洞地址用 MATCH 回发起方 A，
         * A 收到 MATCH 即去打 B；pending 保留，等双方 PUNCHOK 上报后再判 CONNECTED。
         * （MATCH 与 GET 的 PEER 区分开：PEER 只是查询结果，不触发打洞。） */
        dbg_str(DBG_INFO, "[server] ACCEPT from=%s (src %s:%s)",
                from_id, t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        call = NULL;
        pending->search(pending, (void *)from_id, &call);
        if (call != NULL) {
            snprintf(aport, sizeof(aport), "%d", call->from_port);
            snprintf((char *)resp, sizeof(resp), "MATCH %s %d %d\n",
                     call->to_host, call->to_port, call->to_nat);
            resp_len = (int)strlen((char *)resp);
            socket->sendto(socket, resp, resp_len, 0, call->from_host, aport);
        }
        pthread_mutex_unlock(&server->lock);
    } else if (sscanf((char *)t->buf, "PUNCHOK %31s %31s", p1, p2) == 2) {
        /* p1 上报：已成功打洞到 p2。发起方/被叫方都上报后，服务器才判定两端 ok */
        pthread_mutex_lock(&server->lock);
        call = NULL;
        pending->search(pending, (void *)p1, &call);
        if (call != NULL && strcmp(call->to_id, p2) == 0) {
            call->caller_ok = 1;   /* p1 为发起方 */
            dbg_str(DBG_INFO, "[server] punch ok: caller %s -> %s", p1, p2);
        } else {
            call = NULL;
            pending->search(pending, (void *)p2, &call);
            if (call != NULL && strcmp(call->to_id, p1) == 0) {
                call->callee_ok = 1;   /* p1 为被叫方 */
                dbg_str(DBG_INFO, "[server] punch ok: callee %s -> %s", p1, p2);
            }
        }
        if (call != NULL && call->caller_ok && call->callee_ok) {
            /* 双方都打洞成功：判定两端 ok，回双方 CONNECTED */
            dbg_str(DBG_VIP, "[server] both punch ok %s<->%s, send CONNECTED",
                    call->from_id, call->to_id);
            snprintf(sport, sizeof(sport), "%d", call->from_port);
            snprintf((char *)resp, sizeof(resp), "CONNECTED %s\n", call->to_id);
            resp_len = (int)strlen((char *)resp);
            socket->sendto(socket, resp, resp_len, 0, call->from_host, sport);
            /* 给被叫的 CONNECTED 投到其信令地址（服务器可达，兼容同机/LAN） */
            snprintf((char *)resp, sizeof(resp), "CONNECTED %s\n", call->from_id);
            resp_len = (int)strlen((char *)resp);
            p = NULL;
            peers->search(peers, (void *)call->to_id, &p);
            if (p != NULL && p->signal_host[0] != 0) {
                snprintf(sport, sizeof(sport), "%d", p->signal_port);
                socket->sendto(socket, resp, resp_len, 0, p->signal_host, sport);
            } else {   /* 旧单口兜底 */
                snprintf(sport, sizeof(sport), "%d", call->to_port);
                socket->sendto(socket, resp, resp_len, 0, call->to_host, sport);
            }
            pending->del(pending, (void *)call->from_id);
            allocator_mem_free(allocator, call);   /* CONNECTED 后释放呼叫记录 */
        }
        pthread_mutex_unlock(&server->lock);
    } else if (sscanf((char *)t->buf, "REJECT %31s", from_id) == 1) {
        dbg_str(DBG_INFO, "[server] REJECT from=%s (src %s:%s)",
                from_id, t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        call = NULL;
        pending->search(pending, (void *)from_id, &call);
        if (call != NULL) {
            snprintf(aport, sizeof(aport), "%d", call->from_port);
            strcpy((char *)resp, "NOPEER\n");
            resp_len = 7;
            socket->sendto(socket, resp, resp_len, 0, call->from_host, aport);
            pending->del(pending, (void *)from_id);
            allocator_mem_free(allocator, call);   /* REJECT 后释放呼叫记录 */
        }
        pthread_mutex_unlock(&server->lock);
    } else if (sscanf((char *)t->buf, "BYE %31s", id) == 1) {
        /* 下线：peer 正常退出时通知服务器从在线表删除自身。
         * 之后他人 CALL/GET 该 id 查不到即视为不在线(NOPEER)。 */
        dbg_str(DBG_INFO, "[server] BYE id=%s (src %s:%s)",
                id, t->remote_host, t->remote_service);
        pthread_mutex_lock(&server->lock);
        p = NULL;
        peers->search(peers, (void *)id, &p);
        if (p != NULL) {
            peers->del(peers, (void *)id);
            allocator_mem_free(allocator, p);   /* 释放 peer 记录 */
        }
        pthread_mutex_unlock(&server->lock);
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
