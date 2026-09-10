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
 *   SIGNIN <stun_id> -> OK                   上线登记(服务器记信令源地址)
 *   CALL <my> <callee> <own_host> <own_port> 主叫发 CALL(带本会话地址)
 *   INVITE <caller> <caller_host> <caller_port>  服务器投给被叫
 *   ACCEPT <my> <caller> <own_host> <own_port>   被叫回 ACCEPT(带本会话地址)
 *   MATCH <callee_id> <host> <port>              服务器撮合回执给主叫(带被叫会话地址)
 *   PUNCHOK <my> <peer>                         本端打洞成功上报
 *   CONNECTED <peer_id>                          服务器确认双方 ok
 * 免 GET：两端打洞目标(会话地址)经 CALL/INVITE/ACCEPT/MATCH 交换。
 *
 * 事件驱动状态机（不在回调内阻塞）：
 *  建会话 -> 会话 peer_client 发 STUN Binding 采址 -> 收响应解析 own(own_ready)
 *    -> 主叫发 CALL / 被叫发 ACCEPT -> 得对端会话地址后互发 KEEPALIVE 打洞
 *    -> 收对端包上报 PUNCHOK -> 服务器 CONNECTED -> 置 connected。
 * 所有信令/采址响应/对端包都在事件线程回调推进，无额外线程、无线程锁。
 *
 * @author Zoo
 * @date 2026-09-09
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
    dbg_str(DBG_INFO, "%s sent KEEPALIVE to %s:%s ret=%d (own %s:%d)",
            s->stun->stun_id, s->peer_host, service, sr, s->own_host, s->own_port);
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
 *  - 随后发起采址，own 就绪由回调按 role 调 call_session/accept_session；
 *  - out 非空回传会话指针。返回 0=成功；-1=失败。
 */
static int __create_session(Stun *stun, char *remote_id, int role,
                            stun_session_t **out)
{
    allocator_t *allocator = NULL;
    stun_session_t *s = NULL;
    int created = 0, added = 0;
    char *localhost;
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

            /* peer/data socket：默认随机口；设置 local_service 则用固定口 */
            s->peer_client = client(allocator, CLIENT_TYPE_INET_UDP, localhost,
                                    (stun->local_service != NULL)
                                        ? stun->local_service : (char *)"0");
            THROW_IF(s->peer_client == NULL, -1);
            client_trustee(s->peer_client, NULL, __on_session_recv, s);

            s->req = object_new(allocator, "Stun_Request", NULL);
            s->response = object_new(allocator, "Stun_Response", NULL);
            THROW_IF(s->req == NULL || s->response == NULL, -1);

            stun->sessions->add(stun->sessions, s->remote_id, s);
            added = 1;
        }

        /* 发起采址(异步)：own 就绪后回调按 role 调 call/accept_session */
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
    } FINALLY {
    }

    /* 非 JMP 的 TRY/CATCH：成功会令 ret=1，这里归一化为 0(≥0 即成功) */
    return (ret < 0) ? ret : 0;
}

/* 关闭并删除会话：停保活、关 socket、移出会话表并释放。 */
static int __close_session(Stun *stun, char *remote_id)
{
    stun_session_t *s = NULL;
    void *elem = NULL;

    if (stun == NULL || stun->sessions == NULL || remote_id == NULL) {
        return -1;
    }
    s = __get_session(stun, remote_id);
    if (s == NULL) {
        return 0;   /* 无此会话，幂等 */
    }
    s->active = 0;
    __destroy_session(s);
    stun->sessions->remove(stun->sessions, (void *)remote_id, &elem);
    allocator_mem_free(stun->parent.allocator, s);
    return 0;
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

/* 发起采址(STUN Binding，异步)：own 就绪后回调按角色 call_session/accept_session。 */
static int __probe_session_addr(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s;
    char *host, *service;
    Request *req;

    if (stun == NULL || remote_stun_id == NULL) {
        return -1;
    }
    s = __get_session(stun, remote_stun_id);
    if (s == NULL || s->peer_client == NULL || s->peer_client->socket == NULL) {
        return -1;
    }
    req = s->req;
    host = (stun->stun_host != NULL) ? stun->stun_host : stun->signal_host;
    service = (stun->stun_service != NULL) ? stun->stun_service : stun->signal_service;
    if (req == NULL || host == NULL || service == NULL) {
        return -1;
    }
    s->own_host[0] = 0;
    s->own_port = 0;
    s->own_ready = 0;
    req->set_head(req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE);
    if (s->peer_client->socket->sendto(s->peer_client->socket, req->header,
                                       req->get_len(req), 0, host, service) < 0) {
        return -1;
    }
    __stun_log_target(stun, "stun1", host, service);
    return 0;
}

/* 主叫采址完成(own 就绪)：发 CALL <my> <callee> <own_host> <own_port> <nat>。 */
static int __call_session(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s;
    char line[192];
    char service[16];

    if (stun == NULL || remote_stun_id == NULL || stun->stun_id[0] == 0) {
        return -1;
    }
    s = __get_session(stun, remote_stun_id);
    if (s == NULL || !s->own_ready) {
        return -1;
    }
    snprintf(service, sizeof(service), "%d", s->own_port);
    snprintf(line, sizeof(line), "CALL %s %s %s %s %d",
             stun->stun_id, s->remote_id, s->own_host, service, s->nat_type);
    client_connect(stun->server_client, stun->signal_host, stun->signal_service);
    client_send(stun->server_client, line, (int)strlen(line), 0);
    dbg_str(DBG_INFO, "%s sent CALL to %s (own %s:%d nat=%d)",
            stun->stun_id, s->remote_id, s->own_host, s->own_port, s->nat_type);
    return 0;
}

/* 被叫采址完成(own 就绪)：发 ACCEPT <my> <caller> <own_host> <own_port> <nat>。 */
static int __accept_session(Stun *stun, char *remote_stun_id)
{
    stun_session_t *s;
    char line[192];
    char service[16];

    if (stun == NULL || remote_stun_id == NULL || stun->stun_id[0] == 0) {
        return -1;
    }
    s = __get_session(stun, remote_stun_id);
    if (s == NULL || !s->own_ready) {
        return -1;
    }
    snprintf(service, sizeof(service), "%d", s->own_port);
    snprintf(line, sizeof(line), "ACCEPT %s %s %s %s %d",
             stun->stun_id, s->remote_id, s->own_host, service, s->nat_type);
    client_connect(stun->server_client, stun->signal_host, stun->signal_service);
    client_send(stun->server_client, line, (int)strlen(line), 0);
    dbg_str(DBG_INFO, "%s sent ACCEPT to %s (own %s:%d nat=%d)",
            stun->stun_id, s->remote_id, s->own_host, s->own_port, s->nat_type);
    return 0;
}

/* 开始打洞：设定对端会话地址并周期互发 KEEPALIVE（建链首包立即发一个）。 */
static int __punch_session(Stun *stun, char *remote_stun_id,
                           char *peer_host, int peer_port)
{
    stun_session_t *s;
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];
    char service[16];
    Socket *sock;

    if (stun == NULL || remote_stun_id == NULL || peer_host == NULL) {
        return -1;
    }
    s = __get_session(stun, remote_stun_id);
    if (s == NULL || s->peer_client == NULL) {
        return -1;
    }
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
    return 0;
}

static int __send_session_data(Stun *stun, char *remote_stun_id, void *buf, int len)
{
    stun_session_t *s;
    stun_p2p_msg_t *msg;
    char pkt[sizeof(stun_p2p_msg_t) + STUN_P2P_MAX_PAYLOAD];
    char service[16];
    Socket *sock;
    int ret = -1;

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
        EXEC((ret = (sock->sendto(sock, pkt, (int)(sizeof(stun_p2p_msg_t) + len),
                                  0, s->peer_host, service) >= 0) ? 0 : -1));
    } CATCH (ret) {
    }
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
                    s->stun->call_session(s->stun, s->remote_id);
                } else {
                    s->stun->accept_session(s->stun, s->remote_id);
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
                    s->stun->call_session(s->stun, s->remote_id);
                } else {
                    s->stun->accept_session(s->stun, s->remote_id);
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

/* ---------------- 信令回调（节点级，经 server_client） ---------------- */
static int __stun_signal_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Stun *stun = (Stun *)t->opaque;
    stun_session_t *ss;
    char *s;
    char from[32], host[64], line[96], peer[32];
    int port = 0;

    if (stun == NULL || t->buf_len <= 0) {
        return 0;
    }
    s = (char *)t->buf;
    /* 协议文本以 \n 结尾：裁掉行尾换行/回车，仅影响日志显示(解析本就不受影响) */
    while (t->buf_len > 0 &&
           (s[t->buf_len - 1] == '\n' || s[t->buf_len - 1] == '\r')) {
        s[--t->buf_len] = 0;
    }

    if (strncmp(s, "OK", 2) == 0) {
        stun->register_done = 1;
    } else if (strncmp(s, "NOPEER", 6) == 0) {
        /* 对端不在线：关闭本端对应 caller 会话 */
        if (sscanf(s, "NOPEER %31s", peer) == 1) {
            __close_session(stun, peer);
            dbg_str(DBG_ERROR, "%s call to %s failed (NOPEER)", stun->stun_id, peer);
        }
    } else if (strncmp(s, "INVITE", 6) == 0) {
        /* 被叫：服务器把主叫 id + 主叫本会话地址 + 主叫 nat 投过来 */
        stun_session_t *ns = NULL;
        int nat = STUN_NAT_TYPE_UNKNOWN;
        if (sscanf(s, "INVITE %31s %63s %d %d", from, host, &port, &nat) >= 3) {
            dbg_str(DBG_INFO, "%s received INVITE from %s (%s:%d nat=%d)",
                    stun->stun_id, from, host, port, nat);
            /* 建被叫会话(内部已发起采址)；主叫地址已知，随即打洞 */
            if (stun->create_session(stun, from, 1, &ns) == 0 && ns != NULL) {
                ns->peer_nat_type = nat;
                stun->punch_session(stun, from, host, port);
            }
        }
    } else if (strncmp(s, "MATCH", 5) == 0) {
        /* 主叫：撮合回执带被叫 id + 被叫本会话地址 + 被叫 nat，据此打洞 */
        int nat = STUN_NAT_TYPE_UNKNOWN;
        if (sscanf(s, "MATCH %31s %63s %d %d", peer, host, &port, &nat) >= 3) {
            dbg_str(DBG_INFO, "%s received MATCH from %s (%s:%d nat=%d)",
                    stun->stun_id, peer, host, port, nat);
            ss = __get_session(stun, peer);
            if (ss) {
                ss->peer_nat_type = nat;
                stun->punch_session(stun, peer, host, port);
            }
        }
    } else if (strncmp(s, "CONNECTED", 9) == 0) {
        if (sscanf(s, "CONNECTED %31s", peer) == 1) {
            dbg_str(DBG_INFO, "%s CONNECTED to %s, link up", stun->stun_id, peer);
            ss = __get_session(stun, peer);
            if (ss != NULL) {
                ss->connected = 1;
                ss->send_punch = 0;
            }
        }
    }
    (void)line;
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
    } CATCH (ret) {
    }
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
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun, connect, __connect),
    Init_Vfunc_Entry(4, Stun, signin, __signin),
    Init_Vfunc_Entry(5, Stun, set_stun_server, __set_stun_server),
    Init_Vfunc_Entry(6, Stun, create_session, __create_session),
    Init_Vfunc_Entry(7, Stun, send_session_data, __send_session_data),
    Init_Vfunc_Entry(8, Stun, is_connected, __is_connected),
    Init_Vfunc_Entry(9, Stun, close_session, __close_session),
    Init_Vfunc_Entry(10, Stun, signout, __signout),
    Init_Vfunc_Entry(11, Stun, set_recv_callback, __set_recv_callback),
    Init_Vfunc_Entry(12, Stun, get_session, __get_session),
    Init_Vfunc_Entry(13, Stun, probe_session_addr, __probe_session_addr),
    Init_Vfunc_Entry(14, Stun, call_session, __call_session),
    Init_Vfunc_Entry(15, Stun, accept_session, __accept_session),
    Init_Vfunc_Entry(16, Stun, punch_session, __punch_session),
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
