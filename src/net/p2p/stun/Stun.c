/**
 * @file Stun.c
 * @Synopsis  Stun 节点客户端（RFC 5389 STUN + P2P 打洞，多会话 v3）
 *
 * 结构：
 *  - 节点(Stun)：一个 stun_id；一条与信令服务器的常驻连接(server_client)；
 *    一张会话表(Map: remote stun id -> stun_session_t)。
 *  - 会话(stun_session)：与某个对端的一条链路。每会话一个**独立 UDP data socket**
 *    (peer_client，绑 local_host + 随机口，不 connect、sendto)：
 *      采址(own) / 打洞 / 保活 / 业务数据都走各自会话 socket，彼此独立。
 *
 * 信令(文本，经 server_client)：
 *   SIGNIN <stun_id> -> SIGNIN_REPLY <stun_id>   上线登记(服务器记信令源地址)
 *   INVITE <my> <callee> <own_host> <own_port> [nat]   主叫发 INVITE(带本会话地址)
 *   INVITE <caller> <caller_host> <caller_port> [nat]  服务器原样投给被叫
 *   INVITE_REPLY <callee> <caller> accept <host> <port> [nat]  被叫接受(带本会话地址)
 *   INVITE_REPLY <callee> <caller> reject <reason>             被叫/服务器拒绝(见原因码)
 *   PUNCHOK <my> <peer>                          本端打洞成功上报
 *   CONNECTED <peer_id>                           服务器确认双方 ok
 * 免 GET：两端打洞目标(会话地址)靠 INVITE / INVITE_REPLY 一问一答交换。
 *
 * 事件驱动状态机（不在回调内阻塞）：
 *  建会话 -> 会话 peer_client 发 STUN Binding 采址 -> 收响应解析 own(own_ready)
 *    -> 主叫发 INVITE / 被叫回 INVITE_REPLY(accept) -> 得对端会话地址后互发 KEEPALIVE 打洞
 *       （被叫若资源不足则回 INVITE_REPLY(reject)，主叫直接失败，不进打洞）
 *    -> 收对端包上报 PUNCHOK -> 服务器 CONNECTED -> 置 connected。
 * 所有信令/采址响应/对端包都在事件线程回调推进，无额外线程、无线程锁。
 *
 * @author Zoo
 * @date 2026-09-09
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>      /* sockaddr_in */
#include <arpa/inet.h>
#include <netdb.h>
#include <libobject/core/io/Socket.h>
#include <libobject/concurrent/net/Client.h>
#include <libobject/concurrent/work_task.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/concurrent/worker_api.h>
#include "Stun.h"

static int __stun_signal_callback(void *task);
static int __on_session_recv(void *task);
static void __session_keepalive_timer_callback(void *opaque);
static int __close_session(Stun *stun, char *remote_id);

/* 会话保活定时器回调：周期向对端会话地址发 KEEPALIVE。 */
static void __session_keepalive_timer_callback(void *opaque)
{
    stun_session_t *s = (stun_session_t *)opaque;
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];
    char service[16];
    Socket *sock;
    int sr;

    if (s == NULL || s->peer_client == NULL || s->peer_host[0] == 0 ||
        !s->active) {
        return;
    }
    sock = s->peer_client->socket;
    if (sock == NULL || sock->sendto == NULL) {
        return;
    }
    msg = (stun_p2p_msg_t *)buf;
    msg->magic = htonl(STUN_P2P_MAGIC);
    msg->type = STUN_P2P_MSG_KEEPALIVE;
    msg->len = 0;
    snprintf(service, sizeof(service), "%d", s->peer_port);
    sr = sock->sendto(sock, buf, sizeof(buf), 0, s->peer_host, service);
    /* 建链/打洞阶段逐包打印便于观察；链路建立(connected)后不再刷屏，
     * 只在发送失败时告警（保活仍在后台继续发，只是不打日志）。
     * 判据用 connected：它由服务器 CONNECTED 置位，是实现里真正维护的状态。 */
    if (!s->connected) {
        dbg_str(DBG_INFO, "%s sent KEEPALIVE to %s:%s ret=%d (own %s:%d)",
                s->stun->stun_id, s->peer_host, service, sr,
                s->own_host, s->own_port);
    } else if (sr < 0) {
        dbg_str(DBG_ERROR, "%s KEEPALIVE to %s:%s failed ret=%d",
                s->stun->stun_id, s->peer_host, service, sr);
    }
}

static stun_session_t *__get_session(Stun *stun, char *remote_id)
{
    stun_session_t *s = NULL;

    if (stun == NULL || stun->sessions == NULL || remote_id == NULL) {
        return NULL;
    }
    stun->sessions->search(stun->sessions, (void *)remote_id, (void **)&s);

    return s;
}

/* ---------------- 本地端口池：local_service 支持 单值 / 逗号列表 / 范围 ----------------
 * 语义：配置的端口是给会话 data socket 用的**端口池**，每会话一个，容量就是
 * "本节点能用固定端口同时支撑多少条链路"；不配置 = 每会话随机口。
 * 为什么需要池：UDP 未开 SO_REUSEPORT，同一 host:port 第二次 bind 必失败，
 * 且 client() 会忽略该失败（静默落到未绑定 -> 内核首次 sendto 时给随机口，
 * 固定口形同虚设、安全组放行失效）。 */

/* 解析 local_service 为端口池（惰性，只做一次）："12346" / "12346,12347" / "12346-12350" */
static void __parse_ports(Stun *stun)
{
    char buf[512];
    char *tok, *save = NULL;
    int n = 0;

    stun->pool_num = 0;
    stun->pool_parsed = 1;
    /* 未配置，或显式写 "0"/"auto"/"-"（命令行/测试里表示"随机口"）：不进池 */
    if (stun->local_service == NULL || stun->local_service[0] == '\0' ||
        strcmp(stun->local_service, "0") == 0 ||
        strcmp(stun->local_service, "auto") == 0 ||
        strcmp(stun->local_service, "-") == 0) {
        return;                                  /* 不指定端口：每会话随机口 */
    }
    snprintf(buf, sizeof(buf), "%s", stun->local_service);
    for (tok = strtok_r(buf, ",", &save); tok != NULL;
         tok = strtok_r(NULL, ",", &save)) {
        char *dash = strchr(tok, '-');
        int lo, hi, p;

        if (dash != NULL) {                      /* 范围 "a-b"（含端点） */
            lo = atoi(tok);
            hi = atoi(dash + 1);
            if (lo <= 0 || hi < lo || hi > 65535) {
                dbg_str(DBG_WARN, "%s port pool: ignore bad range '%s'", stun->stun_id, tok);
                continue;
            }
            for (p = lo; p <= hi && n < STUN_SERVICE_POOL_MAX; p++) {
                stun->pool_ports[n++] = p;
            }
        } else {                                 /* 单端口 */
            p = atoi(tok);
            if (p <= 0 || p > 65535) {
                dbg_str(DBG_WARN, "%s port pool: ignore bad item '%s'", stun->stun_id, tok);
                continue;
            }
            if (n < STUN_SERVICE_POOL_MAX) {
                stun->pool_ports[n++] = p;
            }
        }
    }
    stun->pool_num = n;
    dbg_str(DBG_INFO, "%s local port pool '%s' -> %d port(s), max %d fixed-port links",
            stun->stun_id, stun->local_service, n, STUN_SERVICE_POOL_MAX);
}

/* 取一个空闲池端口并标记占用（端口号写入 out_port）。
 * 返回 **1 = 成功**，包含"端口池未配置"这一**正常**情况（此时 *out_port = 0，
 * 调用方拿它作 service 就是 "0"，由内核随机分配临时端口）；
 * 返回 **-1 = 失败**，只有"端口池已耗尽"（或参数为空）。
 * 调用方用 EXEC 接：负值才会被抛成错误 -> 建会话失败（被叫回 INVITE_REPLY reject）。 */
static int __alloc_port(Stun *stun, int *out_port)
{
    int i, ret = 1;

    TRY {
        THROW_IF(stun == NULL || out_port == NULL, -1);
        if (!stun->pool_parsed) {
            __parse_ports(stun);                              /* 惰性解析一次 */
        }
        if (stun->pool_num <= 0) {
            *out_port = 0;   /* 未配置端口池：用随机口（不是错误） */
            THROW(1);
        }
        for (i = 0; i < stun->pool_num; i++) {
            if (!stun->pool_used[i]) {                        /* 取到空闲端口 */
                stun->pool_used[i] = 1;
                if (out_port != NULL) {
                    *out_port = stun->pool_ports[i];
                }
                THROW(1);
            }
        }
        THROW(-1);                              /* 配了池却全占用：错误 */
    } CATCH (ret) {}

    return ret;
}

/* 归还端口（会话销毁时调用）。 */
static void __free_port(Stun *stun, int port)
{
    int i;

    if (stun == NULL || port == 0 || port < 0) {
        return;                                /* 随机口(未占池)无需归还 */
    }
    for (i = 0; i < stun->pool_num; i++) {
        if (stun->pool_ports[i] == port) {
            stun->pool_used[i] = 0;
            dbg_str(DBG_INFO, "%s return local port %d to port pool", stun->stun_id, port);
            return;
        }
    }
}

/* 释放会话内部资源（不删表项/不 free 结构体本身）。 */
static void __destroy_session(stun_session_t *s)
{
    if (s->keepalive_worker != NULL) {
        worker_destroy((Worker *)s->keepalive_worker);
        s->keepalive_worker = NULL;
    }
    if (s->peer_client != NULL) {
        client_destroy(s->peer_client);
        s->peer_client = NULL;
    }
    if (s->local_port != 0 && s->stun != NULL) {
        __free_port(s->stun, s->local_port);   /* 端口归还池，供后来对端复用 */
        s->local_port = 0;
    }
    if (s->req != NULL) {
        object_destroy(s->req);
        s->req = NULL;
    }
    if (s->response != NULL) {
        object_destroy(s->response);
        s->response = NULL;
    }
}

/* 清空会话表：逐个复用 __close_session（停保活/关 socket/移表/释放），避免重复拆除逻辑。 */
static void __clear_sessions(Stun *stun)
{
    Iterator *cur, *end;

    if (stun == NULL || stun->sessions == NULL) {
        return;
    }
    cur = stun->sessions->begin(stun->sessions);
    end = stun->sessions->end(stun->sessions);
    for (; !end->equal(end, cur); cur = stun->sessions->begin(stun->sessions),
                                  end = stun->sessions->end(stun->sessions)) {
        void *key = cur->get_kpointer(cur);
        __close_session(stun, (char *)key);
    }
}

/*
 * 建/复用一条会话并发起采址（合并原 call 与 create_session）：
 *  - role：0=caller 主叫，1=callee 被叫；
 *  - 分配会话(独立 data socket + req/response)入表；已存在同 remote 会话则复用；
 *  - 随后发起采址，own 就绪由回调按 role 调 request_session/reply_session_request；
 *  - out 非空回传会话指针。返回 0=成功；-1=失败。
 */
static int __create_session(Stun *stun, char *remote_id, int role,
                            stun_session_t **out)
{
    allocator_t *allocator = NULL;
    stun_session_t *s = NULL;
    int created = 0, added = 0;
    char *localhost;
    char service[16];
    int ret = 0;

    TRY {
        if (out != NULL) {
            *out = NULL;
        }

        THROW_IF(stun == NULL || remote_id == NULL, -1);
        allocator = stun->parent.allocator;

        s = __get_session(stun, remote_id);
        if (s == NULL) {
            localhost = (stun->local_host != NULL) ? stun->local_host
                                            : (char *)"0.0.0.0";
            s = (stun_session_t *)allocator_mem_alloc(allocator, sizeof(*s));
            THROW_IF(s == NULL, -1);
            created = 1;
            memset(s, 0, sizeof(*s));
            snprintf(s->remote_id, sizeof(s->remote_id), "%s", remote_id);
            s->role = role;
            s->active = 1;
            s->stun = stun;
            s->send_punch = 1;
            s->keepalive_interval_ms = stun->keepalive_interval_ms;

            /* 未配端口池 -> local_port = 0 -> service = "0"（内核随机口）；
             * 池已满 -> __alloc_port 返回 -1，EXEC 抛错 -> 建会话失败（被叫回 reject 1）。 */
            EXEC(__alloc_port(stun, &s->local_port));
            snprintf(service, sizeof(service), "%d", s->local_port);
            dbg_str(DBG_INFO, "%s session->%s local port %s (pool '%s')", stun->stun_id,
                    s->remote_id,
                    (s->local_port == 0) ? "random (kernel picked)"
                                                        : service,
                    (stun->local_service != NULL) ? stun->local_service : "-");
            s->peer_client = client(allocator, CLIENT_TYPE_INET_UDP, localhost, service);
            THROW_IF(s->peer_client == NULL, -1);
            client_trustee(s->peer_client, NULL, __on_session_recv, s);

            s->req = object_new(allocator, "Stun_Request", NULL);
            s->response = object_new(allocator, "Stun_Response", NULL);
            THROW_IF(s->req == NULL || s->response == NULL, -1);

            stun->sessions->add(stun->sessions, s->remote_id, s);
            added = 1;
        }

        /* 发起采址(异步)：own 就绪后回调按 role 调 request_session/reply_session_request */
        EXEC(stun->probe_session_addr(stun, remote_id));
        if (out != NULL) {
            *out = s;
        }
    } CATCH (ret) {
        ret = -1;
        if (created && s != NULL) {
            if (added) {
                __close_session(stun, remote_id);
            } else {
                __destroy_session(s);
                allocator_mem_free(allocator, s);
            }
            s = NULL;
        }
        dbg_str(DBG_ERROR, "stun create_session %s failed, ret=%d",
                (remote_id != NULL) ? remote_id : "?", ret);
    } FINALLY { }

    /* 非 JMP 的 TRY/CATCH：成功落底时 ret=1；抛错时 ret=错误码(<0 即失败) */
    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* 关闭并删除会话：停保活、关 socket、移出会话表并释放。 */
static int __close_session(Stun *stun, char *remote_id)
{
    stun_session_t *s = NULL;
    void *elem = NULL;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || stun->sessions == NULL || remote_id == NULL, -1);
        s = __get_session(stun, remote_id);
        THROW_IF(s == NULL, 0);         /* 无此会话：幂等，按成功提前结束（ret=0） */
        s->active = 0;
        __destroy_session(s);
        stun->sessions->remove(stun->sessions, (void *)remote_id, &elem);
        allocator_mem_free(stun->parent.allocator, s);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* ---------------- 采址 / 打洞 ---------------- */

/* 解析并打印 STUN 目标(域名->IP)，便于核对回包来源是否为同一台 STUN。 */
static void __stun_log_target(Stun *stun, const char *tag,
                              const char *host, const char *service)
{
    struct addrinfo hints, *res = NULL;
    char ip[64] = "?";

    if (stun == NULL || host == NULL || service == NULL) {
        return;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(host, service, &hints, &res) == 0 && res != NULL) {
        struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
        inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
        freeaddrinfo(res);
    }
    dbg_str(DBG_INFO, "%s %s target %s:%s -> %s",
            stun->stun_id, tag, host, service, ip);
}

/* 发起采址(STUN Binding，异步)：own 就绪后回调按角色 request_session/reply_session_request。 */
static int __probe_session_addr(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s = NULL;
    char *host = NULL, *service = NULL;
    Request *req = NULL;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || remote_stun_id == NULL, -1);
        s = __get_session(stun, remote_stun_id);
        THROW_IF(s == NULL || s->peer_client == NULL ||
                 s->peer_client->socket == NULL, -1);
        req = s->req;
        host = (stun->stun_host != NULL) ? stun->stun_host : stun->signal_host;
        service = (stun->stun_service != NULL) ? stun->stun_service
                                              : stun->signal_service;
        THROW_IF(req == NULL || host == NULL || service == NULL, -1);
        s->own_host[0] = 0;
        s->own_port = 0;
        s->own_ready = 0;
        req->set_head(req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE);
        EXEC(s->peer_client->socket->sendto(s->peer_client->socket, req->header,
                                           req->get_len(req), 0, host, service));
        __stun_log_target(stun, "stun1", host, service);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* 主叫采址完成(own 就绪)：发 INVITE <my> <callee> <own_host> <own_port> <nat>。 */
static int __request_session(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s = NULL;
    char line[192];
    char service[16];
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || remote_stun_id == NULL || stun->stun_id[0] == 0, -1);
        s = __get_session(stun, remote_stun_id);
        THROW_IF(s == NULL || !s->own_ready, -1);
        snprintf(service, sizeof(service), "%d", s->own_port);
        snprintf(line, sizeof(line), "INVITE %s %s %s %s %d",
                 stun->stun_id, s->remote_id, s->own_host, service, s->nat_type);
        client_connect(stun->server_client, stun->signal_host, stun->signal_service);
        client_send(stun->server_client, line, (int)strlen(line), 0);
        dbg_str(DBG_INFO, "%s sent INVITE to %s (own %s:%d nat=%d)",
                stun->stun_id, s->remote_id, s->own_host, s->own_port, s->nat_type);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* 被叫对 INVITE 的应答（accept/reject 二合一，用一个原因码参数区分两种语义）：
 *   reason == STUN_REJECT_NONE(0) -> 接受：INVITE_REPLY <my> <caller> accept <own_host>
 *                                     <own_port> <nat>（把本会话地址交给主叫，随后打洞）
 *   reason != 0                   -> 拒绝：INVITE_REPLY <my> <caller> reject <reason>
 *                                     （如 STUN_REJECT_NO_PORT：本地端口池已满，主叫立刻失败）
 * 服务器按 caller|callee 找到 pending，把这条 reply 原样转投给主叫。 */
static int __reply_session_request(Stun *stun, char *remote_stun_id, int reason)
{
    stun_session_t *s = NULL;
    char line[192];
    char service[16];
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || remote_stun_id == NULL ||
                 stun->server_client == NULL || stun->stun_id[0] == 0, -1);
        if (reason == STUN_REJECT_NONE) {
            /* 接受：带上本会话(打洞)地址 */
            s = __get_session(stun, remote_stun_id);
            THROW_IF(s == NULL || !s->own_ready, -1);  /* 未采址完成：等 own 就绪再调 */
            snprintf(service, sizeof(service), "%d", s->own_port);
            snprintf(line, sizeof(line), "INVITE_REPLY %s %s accept %s %s %d",
                     stun->stun_id, s->remote_id, s->own_host, service, s->nat_type);
            dbg_str(DBG_INFO, "%s sent INVITE_REPLY(accept) to %s (own %s:%d nat=%d)",
                    stun->stun_id, s->remote_id, s->own_host, s->own_port, s->nat_type);
        } else {
            /* 拒绝：只带原因码（如 STUN_REJECT_NO_PORT） */
            snprintf(line, sizeof(line), "INVITE_REPLY %s %s reject %d",
                     stun->stun_id, remote_stun_id, reason);
            dbg_str(DBG_ERROR, "%s sent INVITE_REPLY(reject %d) to %s", stun->stun_id,
                    reason, remote_stun_id);
        }
        client_connect(stun->server_client, stun->signal_host, stun->signal_service);
        client_send(stun->server_client, line, (int)strlen(line), 0);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* 开始打洞：设定对端会话地址并周期互发 KEEPALIVE（建链首包立即发一个）。 */
static int __punch_session(Stun *stun, char *remote_stun_id,
                           char *peer_host, int peer_port)
{
    stun_session_t *s = NULL;
    stun_p2p_msg_t *msg = NULL;
    char buf[sizeof(stun_p2p_msg_t)];
    char service[16];
    Socket *sock = NULL;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || remote_stun_id == NULL || peer_host == NULL, -1);
        s = __get_session(stun, remote_stun_id);
        THROW_IF(s == NULL || s->peer_client == NULL, -1);
        snprintf(s->peer_host, sizeof(s->peer_host), "%s", peer_host);
        s->peer_port = peer_port;
        s->send_punch = 1;   /* 一旦收到对端包上报 PUNCHOK */

        sock = s->peer_client->socket;
        msg = (stun_p2p_msg_t *)buf;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_KEEPALIVE;   /* 首包也打洞/保活统一 */
        msg->len = 0;
        snprintf(service, sizeof(service), "%d", peer_port);
        if (sock != NULL && sock->sendto != NULL) {
            sock->sendto(sock, buf, sizeof(buf), 0, peer_host, service);
        }

        /* 启动本会话保活定时器（首次打洞时；best-effort，失败不致命） */
        if (s->stun != NULL && s->keepalive_worker == NULL) {
            Worker *w = NULL;
            struct timeval tv;
            int interval = (s->keepalive_interval_ms > 0)
                               ? s->keepalive_interval_ms : 1000;

            s->keepalive_interval_ms = interval;
            tv.tv_sec = interval / 1000;
            tv.tv_usec = (interval % 1000) * 1000;
            w = timer_worker(s->stun->parent.allocator, EV_READ | EV_PERSIST,
                             &tv, __session_keepalive_timer_callback, s);
            if (w != NULL) {
                s->keepalive_worker = (void *)w;
            }
        }
        dbg_str(DBG_INFO, "%s punched %s at %s:%d",
                s->stun->stun_id, s->remote_id, peer_host, peer_port);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

static int __send_session_data(Stun *stun, char *remote_stun_id, void *buf, int len)
{
    stun_session_t *s;
    stun_p2p_msg_t *msg;
    char pkt[sizeof(stun_p2p_msg_t) + STUN_P2P_MAX_PAYLOAD];
    char service[16];
    Socket *sock;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || remote_stun_id == NULL || buf == NULL, -1);
        THROW_IF(len < 0 || len > STUN_P2P_MAX_PAYLOAD, -1);
        s = __get_session(stun, remote_stun_id);
        THROW_IF(s == NULL || !s->active, -1);
        THROW_IF(s->peer_host[0] == 0, -1);   /* 尚未拿到对端会话地址 */
        sock = s->peer_client->socket;
        THROW_IF(sock == NULL || sock->sendto == NULL, -1);
        msg = (stun_p2p_msg_t *)pkt;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_DATA;
        msg->len = htons(len);
        memcpy(msg->data, buf, len);
        snprintf(service, sizeof(service), "%d", s->peer_port);
        EXEC(sock->sendto(sock, pkt, (int)(sizeof(stun_p2p_msg_t) + len),
                          0, s->peer_host, service));
    } CATCH (ret) { }

    return ret;
}

/* 向第二 STUN(stun2_host)用本会话 data socket 发一路 Binding 做对称探测。
 * 未配置 stun2 或 socket 不可用时什么都不做。 */
static void __probe_nat2(stun_session_t *s)
{
    int ret;

    if (s == NULL || s->stun == NULL || s->req == NULL ||
        s->peer_client == NULL || s->peer_client->socket == NULL ||
        s->peer_client->socket->sendto == NULL ||
        s->stun->stun2_host == NULL || s->stun->stun2_service == NULL) {
        return;
    }
    s->req->set_head(s->req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE);
    ret = (int)s->peer_client->socket->sendto(s->peer_client->socket,
                                              s->req->header,
                                              s->req->get_len(s->req), 0,
                                              s->stun->stun2_host,
                                              s->stun->stun2_service);
    __stun_log_target(s->stun, "stun2",
                      s->stun->stun2_host, s->stun->stun2_service);
}

/* 会话 data socket 收包回调（独立于节点信令回调）：
 *  - STUN magic    -> 采址响应：解析本会话公网映射 own_host/own_port
 *  - STUN_P2P magic -> 对端打洞/保活/数据
 * opaque = session。 */
static int __on_session_recv(void *task)
{
    work_task_t *t = (work_task_t *)task;
    stun_session_t *s = (stun_session_t *)t->opaque;
    Response *resp;
    stun_header_t *h;
    stun_p2p_msg_t *msg;
    stun_attrib_t *attr = NULL;
    uint16_t rlen;

    if (s == NULL || t->buf_len <= 0 || s->stun == NULL) {
        return 0;
    }

    h = (stun_header_t *)t->buf;
    if (t->buf_len >= (int)sizeof(stun_header_t) &&
        ntohl(h->magic_cookie) == STUN_MAGIC_COOKIE) {
        /* STUN 回包：第一次=主采址(定 own)；配了第二 STUN 则随后向其再采一次址(定 nat)；
         * 等第二路回包后再 announce(带 nat 给对端)。 */
        int is_second = s->own_ready;
        char prev_host[64];
        int  prev_port = s->own_port;
        int  got = 0;

        snprintf(prev_host, sizeof(prev_host), "%s", s->own_host);
        resp = s->response;
        if (resp != NULL && resp->buffer != NULL &&
            resp->buffer->write(resp->buffer, t->buf, t->buf_len) > 0 &&
            resp->read(resp) >= 0 && resp->header != NULL &&
            resp->header->msgtype == STUN_BINDRESP) {
            attr = NULL;
            if (resp->attribs->search(resp->attribs,
                                      (void *)STUN_ATR_TYPE_XOR_MAPPED_ADDR,
                                      (void **)&attr) != 1) {
                attr = NULL;
            }
            if (attr == NULL) {
                resp->attribs->search(resp->attribs,
                                      (void *)STUN_ATR_TYPE_MAPPED_ADDR,
                                      (void **)&attr);
            }
            if (attr != NULL) {
                snprintf(s->own_host, sizeof(s->own_host), "%s",
                         attr->u.mapped_address.host);
                s->own_port = atoi(attr->u.mapped_address.service);
                got = 1;
            }
        }
        if (got) {
            if (is_second) {
                /* 第二 STUN 响应：与主 STUN 比较本端映射端口判 nat；恢复 own 后 announce */
                int  second_port = s->own_port;
                char second_host[64];

                snprintf(second_host, sizeof(second_host), "%s", s->own_host);
                if (s->nat_type == STUN_NAT_TYPE_UNKNOWN) {
                    s->nat_type = (second_port == prev_port &&
                                   strcmp(second_host, prev_host) == 0)
                                      ? STUN_NAT_TYPE_CONE
                                      : STUN_NAT_TYPE_SYMMETRIC;
                    dbg_str(DBG_INFO,
                        "%s stun2 reply from %s:%s (stun %s:%s) own=%s:%d",
                        s->stun->stun_id, t->remote_host, t->remote_service,
                        (s->stun->stun2_host != NULL) ? s->stun->stun2_host : "?",
                        (s->stun->stun2_service != NULL) ? s->stun->stun2_service : "?",
                        s->own_host, s->own_port);
                }
                snprintf(s->own_host, sizeof(s->own_host), "%s", prev_host);
                s->own_port = prev_port;
                if (s->role == 0) {
                    s->stun->request_session(s->stun, s->remote_id);
                } else {
                    s->stun->reply_session_request(s->stun, s->remote_id, STUN_REJECT_NONE);
                }
            } else {
                s->own_ready = 1;
                dbg_str(DBG_INFO,
                        "%s stun1 reply from %s:%s (stun %s:%s) own=%s:%d",
                        s->stun->stun_id, t->remote_host, t->remote_service,
                        (s->stun->stun_host != NULL) ? s->stun->stun_host : "?",
                        (s->stun->stun_service != NULL) ? s->stun->stun_service : "?",
                        s->own_host, s->own_port);
                if (s->stun->stun2_host != NULL &&
                    s->stun->stun2_service != NULL) {
                    __probe_nat2(s);   /* 向第二 STUN 采址，回包后再 call/accept */
                } else if (s->role == 0) {
                    s->stun->request_session(s->stun, s->remote_id);
                } else {
                    s->stun->reply_session_request(s->stun, s->remote_id, STUN_REJECT_NONE);
                }
            }
        }
    } else if (t->buf_len >= (int)sizeof(stun_p2p_msg_t) &&
               ntohl(((stun_p2p_msg_t *)t->buf)->magic) == STUN_P2P_MAGIC) {
        msg = (stun_p2p_msg_t *)t->buf;
        s->data_received = 1;
        rlen = ntohs(msg->len);
        /* 源地址学习：以对端包的真实源为准(回发都发到这里)。若与信令上报不同
         * (常见于对称/CGNAT、或 NAT 对某目的地单独换端口)，提示并更新目标。 */
        if (t->remote_host != NULL && t->remote_host[0] != 0) {
            if (s->peer_host[0] != 0 &&
                (strcmp(s->peer_host, t->remote_host) != 0 ||
                 s->peer_port != atoi(t->remote_service))) {
                dbg_str(DBG_INFO,
                        "%s found peer public IP address differs: told %s:%d -> actual %s:%s",
                        s->stun->stun_id, s->peer_host, s->peer_port,
                        t->remote_host, t->remote_service);
            }
            snprintf(s->peer_host, sizeof(s->peer_host), "%s", t->remote_host);
            s->peer_port = atoi(t->remote_service);
        }
        dbg_str(DBG_DETAIL, "%s received P2P type=%d len=%u from %s:%s",
                s->stun->stun_id, msg->type, rlen,
                t->remote_host, t->remote_service);
        switch (msg->type) {
        case STUN_P2P_MSG_KEEPALIVE:
            /* 收到对端任一包即可达：打洞成功后上报一次 PUNCHOK */
            if (s->send_punch && s->stun->server_client != NULL &&
                s->stun->signal_host != NULL) {
                char line[96];
                snprintf(line, sizeof(line), "PUNCHOK %s %s",
                         s->stun->stun_id, s->remote_id);
                client_connect(s->stun->server_client, s->stun->signal_host,
                               s->stun->signal_service);
                client_send(s->stun->server_client, line, (int)strlen(line), 0);
                s->send_punch = 0;
                dbg_str(DBG_INFO,
                        "%s received KEEPALIVE from %s (%s:%s), sent PUNCHOK",
                        s->stun->stun_id, s->remote_id,
                        t->remote_host, t->remote_service);
            }
            break;
        case STUN_P2P_MSG_DATA:
            if (s->stun->recv_callback != NULL) {
                s->stun->recv_callback(s->stun, s, msg->data, rlen);
            }
            break;
        default:
            break;
        }
    }

    return 0;
}

/* ---------------- 节点级接口 ---------------- */
static int __connect(Stun *stun, char *host, char *service)
{
    allocator_t *allocator = stun->parent.allocator;
    char *lh;
    int ret = 0;

    TRY {
        THROW_IF(stun->server_client != NULL, 1);
        THROW_IF(host == NULL || service == NULL, -1);
        lh = (stun->local_host != NULL) ? stun->local_host : (char *)"0.0.0.0";

        /* 信令口：绑临时端口，connect 常驻信令服务器 */
        stun->server_client = client(allocator, CLIENT_TYPE_INET_UDP, lh, (char *)"0");
        THROW_IF(stun->server_client == NULL, -1);
        EXEC(client_connect(stun->server_client, host, service));
        stun->signal_host = host;
        stun->signal_service = service;
        EXEC(client_trustee(stun->server_client, NULL, __stun_signal_callback, stun));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun connect failed, ret=%d", ret);
    }

    return ret;
}

static int __set_stun_server(Stun *stun, char *host, char *service)
{
    stun->stun_host = host;
    stun->stun_service = service;

    return 0;
}

static int __is_connected(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s = __get_session(stun, remote_stun_id);

    return (s != NULL && s->active && s->connected) ? 0 : -1;
}

static int __signin(Stun *stun, char *stun_id)
{
    char buf[64];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->server_client == NULL || stun_id == NULL, -1);
        THROW_IF(stun->signal_host == NULL || stun->signal_service == NULL, -1);
        EXEC(client_connect(stun->server_client, stun->signal_host, stun->signal_service));
        snprintf(stun->stun_id, sizeof(stun->stun_id), "%s", stun_id);
        snprintf(buf, sizeof(buf), "SIGNIN %s\n", stun_id);
        EXEC(client_send(stun->server_client, buf, (int)strlen(buf), 0));
        stun->register_done = 0;
        for (i = 0; i < 30 && !stun->register_done; i++) {
            usleep(100000);
        }
        THROW_IF(!stun->register_done, -1);
        dbg_str(DBG_INFO, "%s registered to signaling", stun_id);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun signin failed, ret=%d", ret);
    }

    return ret;
}

static int __signout(Stun *stun)
{
    if (stun->server_client != NULL && stun->signal_host != NULL &&
        stun->stun_id[0] != 0) {
        char buf[64];
        client_connect(stun->server_client, stun->signal_host, stun->signal_service);
        snprintf(buf, sizeof(buf), "SIGNOUT %s\n", stun->stun_id);
        client_send(stun->server_client, buf, (int)strlen(buf), 0);
        dbg_str(DBG_INFO, "%s sent SIGNOUT, offline", stun->stun_id);
    }

    return 0;
}

static int __set_recv_callback(Stun *stun,
                               int (*func)(Stun *stun, stun_session_t *session,
                                           uint8_t *buf, int len))
{
    stun->recv_callback = func;

    return 0;
}

static int __construct(Stun *stun, char *init_str)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0, trustee_flag = 1;
    int value_type = VALUE_TYPE_STRUCT_POINTER;

    TRY {
        stun->server_client = NULL;
        stun->sessions = NULL;
        stun->stun_id[0] = 0;
        stun->register_done = 0;
        stun->recv_callback = NULL;
        stun->local_host = NULL;
        stun->signal_host = NULL;
        stun->signal_service = NULL;
        stun->stun_host = NULL;
        stun->stun_service = NULL;
        stun->stun2_host = NULL;
        stun->stun2_service = NULL;
        stun->keepalive_interval_ms = 0;

        stun->sessions = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(stun->sessions == NULL, -1);
        stun->sessions->set_cmp_func(stun->sessions, string_key_cmp_func);
        stun->sessions->set(stun->sessions, "/Map/trustee_flag", &trustee_flag);
        stun->sessions->set(stun->sessions, "/Map/value_type", &value_type);
    } CATCH (ret) { }

    return ret;
}

static int __deconstruct(Stun *stun)
{
    if (stun->server_client != NULL) {
        client_destroy(stun->server_client);
        stun->server_client = NULL;
    }
    if (stun->sessions != NULL) {
        /* 先手动销毁各会话内部 io 并移出释放，再销毁 Map */
        __clear_sessions(stun);
        object_destroy(stun->sessions);
        stun->sessions = NULL;
    }

    return 0;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry( 0, Obj, parent),
    Init_Nfunc_Entry( 1, Stun, construct, __construct),
    Init_Nfunc_Entry( 2, Stun, deconstruct, __deconstruct),
    /* ---- 节点级（非会话） ---- */
    Init_Vfunc_Entry( 3, Stun, connect, __connect),
    Init_Vfunc_Entry( 4, Stun, signin, __signin),
    Init_Vfunc_Entry( 5, Stun, signout, __signout),
    Init_Vfunc_Entry( 6, Stun, set_stun_server, __set_stun_server),
    Init_Vfunc_Entry( 7, Stun, set_recv_callback, __set_recv_callback),
    /* ---- 会话级：名字含 session 的接口集中在此（顺序与 Stun.h 一致） ---- */
    Init_Vfunc_Entry( 8, Stun, create_session, __create_session),
    Init_Vfunc_Entry( 9, Stun, close_session, __close_session),
    Init_Vfunc_Entry(10, Stun, get_session, __get_session),
    Init_Vfunc_Entry(11, Stun, send_session_data, __send_session_data),
    Init_Vfunc_Entry(12, Stun, probe_session_addr, __probe_session_addr),
    Init_Vfunc_Entry(13, Stun, request_session, __request_session),
    Init_Vfunc_Entry(14, Stun, reply_session_request, __reply_session_request),
    Init_Vfunc_Entry(15, Stun, punch_session, __punch_session),
    /* 会话状态查询（名字不含 session，紧跟会话组） */
    Init_Vfunc_Entry(16, Stun, is_connected, __is_connected),
    Init_End___Entry(17, Stun),
};
REGISTER_CLASS(Stun, stun_class_info);

static int __parse_attrib_mapped_addr(stun_attrib_t *raw, stun_attrib_t *out)
{
    int ret;
    int i, len, family;
    uint8_t *p, *host;

    TRY {
        family = raw->u.mapped_address.family;
        SET_CATCH_INT_PARS(family, 0);
        THROW_IF(family != 0x1 && family != 2, -1);
        snprintf(out->u.mapped_address.service, 8, "%d", ntohs(raw->u.mapped_address.port));
        len = family == 0x1 ? 4 : 8;
        p = raw->u.mapped_address.ip;
        host = out->u.mapped_address.host;
        host[0] = 0;   /* 先清空，避免复用缓冲区时把上次地址追加进来 */
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __parse_attrib_changed_addr(stun_attrib_t *raw, stun_attrib_t *out)
{
    int ret;
    int i, len, family;
    uint8_t *p, *host;

    TRY {
        family = raw->u.changed_address.family;
        SET_CATCH_INT_PARS(family, 0);
        THROW_IF(family != 0x1 && family != 2, -1);
        snprintf(out->u.changed_address.service, 8, "%d", ntohs(raw->u.changed_address.port));
        len = family == 0x1 ? 4 : 8;
        p = raw->u.changed_address.ip;
        host = out->u.changed_address.host;
        host[0] = 0;   /* 先清空，避免复用缓冲区时把上次地址追加进来 */
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

attrib_parse_policy_t g_stun_parse_attr_policies[] = {
    {STUN_ATR_TYPE_MAPPED_ADDR,       __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_XOR_MAPPED_ADDR,   __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_CHANGED_ADDRESS,   __parse_attrib_changed_addr},
};
int g_stun_parse_attr_policies_count =
    sizeof(g_stun_parse_attr_policies) / sizeof(g_stun_parse_attr_policies[0]);


/* ---------------- 信令处理：表驱动分派（客户端侧，节点级） ----------------
 * 每个信令一个处理函数，签名统一 (stun, line)：返回 0=已处理 / -1=坏包。
 * 新增信令只需加一个处理函数 + 在 g_stun_cli_sig_table 加一行。分派要求关键字是
 * "整词"（后跟空格或行尾），所以 INVITE 不会误吃 INVITE_REPLY。 */
typedef int (*stun_cli_sig_handler_t)(Stun *stun, char *line);

typedef struct stun_cli_sig_entry_s {
    const char            *key;      /* 文本关键字 */
    const char            *name;     /* 名称，仅用于日志/排障 */
    stun_cli_sig_handler_t handle;   /* 处理函数 */
} stun_cli_sig_entry_t;


/* SIGNIN_REPLY <stun_id>：登记被服务器确认（signin 同步等待它）。 */
static int __sig_signin_reply(Stun *stun, char *line)
{
    (void)line;
    stun->register_done = 1;

    return 0;
}

/* INVITE <caller> <host> <port> [nat]（被叫）：建会话并打洞。
 * 建不起来（如本地数据口端口池已满）就回 INVITE_REPLY reject，让主叫立刻失败，
 * 而不是干等到超时。 */
static int __sig_request_session(Stun *stun, char *line)
{
    stun_session_t *ns = NULL;
    char from[32] = {0}, host[64] = {0};
    int port = 0, nat = STUN_NAT_TYPE_UNKNOWN;

    if (sscanf(line, "INVITE %31s %63s %d %d", from, host, &port, &nat) < 3) {
        return -1;
    }
    dbg_str(DBG_INFO, "%s received INVITE from %s (%s:%d nat=%d)", stun->stun_id,
            from, host, port, nat);
    if (stun->create_session(stun, from, 1, &ns) < 0 || ns == NULL) {
        stun->reply_session_request(stun, from, STUN_REJECT_NO_PORT);
    } else {
        ns->peer_nat_type = nat;
        stun->punch_session(stun, from, host, port);
    }

    return 0;
}

/* INVITE_REPLY <callee> <caller> <accept|reject> ...（主叫）：被叫对本次 INVITE 的应答
 * （一问一答，双 id 无歧义）：
 *   accept -> 记对端 nat，按对端会话地址打洞；
 *   reject -> 区分原因码（STUN_REJECT_*），标记本端该会话失败。 */
static int __sig_reply_session_request(Stun *stun, char *line)
{
    char callee[32] = {0}, rcaller[32] = {0}, action[16] = {0}, host[64] = {0};
    int port = 0, nat = STUN_NAT_TYPE_UNKNOWN, reason = STUN_REJECT_NONE;
    stun_session_t *ss = NULL;
    int ret = 0;

    TRY {
        THROW_IF(sscanf(line, "INVITE_REPLY %31s %31s %15s", callee, rcaller,
                        action) != 3, -1);
        if (strcmp(action, "accept") == 0) {
            THROW_IF(sscanf(line, "INVITE_REPLY %31s %31s %15s %63s %d %d", callee,
                            rcaller, action, host, &port, &nat) < 5, -1);
            dbg_str(DBG_INFO, "%s INVITE_REPLY(accept) from %s (%s:%d nat=%d)",
                    stun->stun_id, callee, host, port, nat);
            ss = __get_session(stun, callee);
            if (ss != NULL) {
                ss->peer_nat_type = nat;
                stun->punch_session(stun, callee, host, port);
            }
        } else {
            sscanf(line, "INVITE_REPLY %31s %31s %15s %d", callee, rcaller, action,
                   &reason);
            dbg_str(DBG_ERROR, "%s INVITE rejected by %s (reason=%d)", stun->stun_id,
                    callee, reason);
            if (stun->stun_id[0] != 0 && strcmp(rcaller, stun->stun_id) != 0) {
                dbg_str(DBG_WARN, "%s INVITE_REPLY not for us (caller=%s), ignored",
                        stun->stun_id, rcaller);
            } else {
                /* 只标记失败，不在这里释放：会话生命周期归上层(p2p_session_close) */
                ss = __get_session(stun, callee);
                if (ss != NULL) {
                    ss->state = STUN_SESSION_CLOSED;
                    ss->active = 0;
                    ss->connected = 0;
                }
            }
        }
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* CONNECTED <peer>：服务器确认双方打洞成功 -> 置会话 connected（此后可发业务数据）。 */
static int __sig_connected(Stun *stun, char *line)
{
    stun_session_t *ss = NULL;
    char peer[32] = {0};

    if (sscanf(line, "CONNECTED %31s", peer) != 1) {
        return -1;
    }
    dbg_str(DBG_INFO, "%s CONNECTED to %s, link up", stun->stun_id, peer);
    ss = __get_session(stun, peer);
    if (ss != NULL) {
        /* state 为设计上的权威状态，connected 是其等价缓存，两者同步置位
         * （此前只置 connected，导致按 state 判断的地方全部失效）。 */
        ss->state = STUN_SESSION_CONNECTED;
        ss->connected = 1;
        ss->send_punch = 0;
    }

    return 0;
}

static const stun_cli_sig_entry_t g_stun_cli_sig_table[] = {
    { "SIGNIN_REPLY", "SIGNIN_REPLY", __sig_signin_reply },
    { "INVITE",       "INVITE",       __sig_request_session },
    { "INVITE_REPLY", "INVITE_REPLY", __sig_reply_session_request },
    { "CONNECTED",    "CONNECTED",    __sig_connected },
};
#define STUN_CLI_SIG_TABLE_NUM \
    (sizeof(g_stun_cli_sig_table) / sizeof(g_stun_cli_sig_table[0]))

/* 信令回调（节点级，经 server_client）：裁掉行尾换行 -> 按关键字查表分派。 */
static int __stun_signal_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Stun *stun = (Stun *)t->opaque;
    char *s = NULL;
    size_t i;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL || t->buf_len <= 0, 1);   /* 空包：忽略，按成功结束 */
        s = (char *)t->buf;
        /* 协议文本以 \n 结尾：裁掉行尾换行/回车，仅影响日志显示(解析本就不受影响) */
        while (t->buf_len > 0 &&
               (s[t->buf_len - 1] == '\n' || s[t->buf_len - 1] == '\r')) {
            s[--t->buf_len] = 0;
        }
        for (i = 0; i < STUN_CLI_SIG_TABLE_NUM; i++) {
            size_t klen = strlen(g_stun_cli_sig_table[i].key);

            if (strncmp(s, g_stun_cli_sig_table[i].key, klen) != 0 ||
                (s[klen] != ' ' && s[klen] != '\0')) {
                continue;
            }
            dbg_str(DBG_DETAIL, "%s handle signal %s", stun->stun_id,
                    g_stun_cli_sig_table[i].name);
            THROW(g_stun_cli_sig_table[i].handle(stun, s));   /* 透传 handler 结果 */
        }
        dbg_str(DBG_WARN, "%s ignore unknown signal: %.32s", stun->stun_id, s);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}