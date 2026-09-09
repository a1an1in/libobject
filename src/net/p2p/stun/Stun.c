/**
 * @file Stun.c
 * @Synopsis  Peer 统一客户端（RFC 5389 STUN + P2P 打洞）
 *
 * 两个 UDP client（信令口与打洞口分开，避免“单口 connect 在 server/对端间来回切
 * 换导致收不到服务器 CONNECTED”）：
 *  - server_client：绑临时端口，connect() 常驻中心服务器，只收发服务器信令，
 *    收包回调 __stun_signal_callback（OK/WAIT/PEER/NOPEER/CONNECTED 文本）；
 *  - peer_client：绑 local_service（公告/打洞地址），保持【不 connect】，向对端/
 *    公共 STUN 的收发一律 sendto，收包回调 __stun_peer_callback（STUN Binding
 *    响应 + P2P 打洞/保活/数据 + 被叫侧服务器 INVITE/CONNECTED）。
 *
 * @author alan lin / Zoo
 * @version
 * @date 2019-06-19
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <libobject/core/io/Socket.h>
#include <libobject/concurrent/net/Client.h>
#include <libobject/concurrent/work_task.h>
#include <libobject/concurrent/event_api.h>
#include "Stun.h"

static int __stun_signal_callback(void *task);
static int __stun_peer_callback(void *task);
static void *__keepalive_routine(void *arg);
static int __keepalive_stop(Stun *stun);

static int __construct(Stun *stun, char *init_str)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0;

    TRY {
       stun->server_client = NULL;   /* 对象内存未必清零，关键句柄显式初始化 */
       stun->peer_client   = NULL;
       stun->req = object_new(allocator, "Stun_Request", NULL);
       stun->response = object_new(allocator, "Stun_Response", NULL);
       THROW_IF(stun->req == NULL || stun->response == NULL, -1);
       stun->stop = 1;   /* 保活线程未启动 */
       stun->connected = 0;
       stun->send_punch = 0;
       stun->dialing = 0;
       stun->id[0] = 0;
       stun->peer_id[0] = 0;

    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Stun *stun)
{
    if (stun->stop == 0) {
        __keepalive_stop(stun);
    }
    if (stun->peer_client != NULL) {
        client_destroy(stun->peer_client);
        stun->peer_client = NULL;
    }
    if (stun->server_client != NULL) {
        client_destroy(stun->server_client);
        stun->server_client = NULL;
    }
    object_destroy(stun->req);
    object_destroy(stun->response);
    return 0;
}

int __set_recv_callback(Stun *stun, int (*func)(Stun *stun, uint8_t *buf, int len))
{
    stun->recv_callback = func;

    return 1;
}

/*
 * send_peer(标准方法)：在 peer(打洞) client 上向指定地址 sendto 一包。
 * peer_client 保持【不 connect】（见头注释），既能收到对端 P2P 打洞/保活/数据，
 * 也能在采址阶段收到公共 STUN 的 Binding 响应。
 * 返回：0=成功；-1=失败。
 */
static int __send_peer(Stun *stun, char *host, char *service, void *buf, int len)
{
    Socket *s;

    if (stun == NULL || stun->peer_client == NULL || host == NULL ||
        service == NULL || buf == NULL) {
        return -1;
    }
    s = stun->peer_client->socket;
    if (s == NULL || s->sendto == NULL) {
        return -1;
    }
    return (s->sendto(s, buf, len, 0, host, service) >= 0) ? 0 : -1;
}

/*
 * 创建两个 UDP client：
 *  - peer_client：绑 local_service（打洞口），不 connect，注册 __stun_peer_callback；
 *  - server_client：绑临时端口，connect() 中心服务器(host:service)，注册信令回调。
 */
static int __connect(Stun *stun, char *host, char *service)
{
    allocator_t *allocator = stun->parent.allocator;
    char *lh;
    int ret = 0;

    TRY {
        THROW_IF(stun->server_client != NULL || stun->peer_client != NULL, 1);
        THROW_IF(host == NULL || service == NULL, -1);

        lh = (stun->local_host != NULL) ? stun->local_host : (char *)"0.0.0.0";

        /* P2P(打洞/数据) client：绑 local_service，保持不 connect */
        stun->peer_client = client(allocator, CLIENT_TYPE_INET_UDP, lh,
                         (stun->local_service != NULL) ? stun->local_service : (char *)"0");
        THROW_IF(stun->peer_client == NULL, -1);
        EXEC(client_trustee(stun->peer_client, NULL, __stun_peer_callback, stun));

        /* 信令(控制) client：绑临时端口，常驻 connect 中心服务器 */
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

/*
 * 用 peer(打洞) client 向公共 STUN 服务器发 Binding 采址（sendto，不 connect），
 * 取得本 peer 打洞口的公网映射地址（写入 mapped_host/mapped_port）。
 */
static int __discovery(Stun *stun, char *host, char *service)
{
    Request *req = stun->req;
    int ret = 0, i;

    TRY {
        THROW_IF(stun->peer_client == NULL || host == NULL || service == NULL, -1);

        /* 清空上次结果，确保本次是新响应 */
        stun->mapped_host[0] = 0;
        stun->mapped_port = 0;

        EXEC(req->set_head(req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE));
        /* peer_client 保持不 connect：sendto 发 Binding，响应从任何源都能收到 */
        EXEC(stun->send_peer(stun, host, service, req->header, req->get_len(req)));

        /* 等待 Binding 响应解析出公网映射地址 */
        for (i = 0; i < 50 && stun->mapped_port == 0; i++) {
            usleep(100000);
        }
        THROW_IF(stun->mapped_port == 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun discovery failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 经 server_client 向信令中心注册本 peer：把 peer_client 采址得到的公网映射地址
 * 主动上报（REG <id> <host> <port> <nat>）。服务器把它当作“公告/打洞地址”：
 * 对端据此打洞，服务器也把被叫侧 INVITE/CONNECTED 投递到该地址。
 */
static int __register_addr(Stun *stun, char *id)
{
    char buf[128];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->server_client == NULL || id == NULL, -1);
        THROW_IF(stun->signal_host == NULL || stun->signal_service == NULL, -1);
        THROW_IF(stun->mapped_host[0] == 0 || stun->mapped_port == 0, -1);

        /* server_client 常驻 connect 信令中心；再 connect 一次仅是保险 */
        EXEC(client_connect(stun->server_client, stun->signal_host, stun->signal_service));

        snprintf(buf, sizeof(buf), "REG %s %s %d %d\n",
                 id, stun->mapped_host, stun->mapped_port, stun->nat_type);
        EXEC(client_send(stun->server_client, buf, (int)strlen(buf), 0));
        stun->register_done = 0;
        for (i = 0; i < 30 && !stun->register_done; i++) {
            usleep(100000);
        }
        THROW_IF(!stun->register_done, -1);
        snprintf(stun->id, sizeof(stun->id), "%s", id);
        dbg_str(DBG_INFO, "[%s] registered public addr %s:%d nat_type=%d to signaling",
                id, stun->mapped_host, stun->mapped_port, stun->nat_type);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun register_addr failed, ret=%d", ret);
    }

    return ret;
}

/*
 * send_server(标准方法)：经 server_client 向信令中心发一行文本命令
 * （REG/CALL/ACCEPT/PUNCHOK/BYE 等，自动加 '\n'）。
 * 返回：0=成功；-1=失败。
 */
static int __send_server(Stun *stun, const char *line)
{
    char buf[128];

    if (stun->server_client == NULL || stun->signal_host == NULL ||
        stun->signal_service == NULL) {
        return -1;
    }
    if (client_connect(stun->server_client, stun->signal_host, stun->signal_service) < 0) {
        return -1;
    }
    snprintf(buf, sizeof(buf), "%s\n", line);
    if (client_send(stun->server_client, buf, (int)strlen(buf), 0) < 0) {
        return -1;
    }
    return 0;
}

/* 主叫(异步)：经 server_client 向 peer_id 发 CALL；之后收到 PEER 会自动打洞建链。 */
static int __connect_peer(Stun *stun, char *peer_id)
{
    char line[80];
    int ret = -1;

    TRY {
        THROW_IF(stun->server_client == NULL || peer_id == NULL || stun->id[0] == 0, -1);
        snprintf(stun->peer_id, sizeof(stun->peer_id), "%s", peer_id);
        snprintf(line, sizeof(line), "CALL %s %s", stun->id, stun->peer_id);
        EXEC(stun->send_server(stun, line));
        stun->dialing = 1;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "connect_peer CALL failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 探测 NAT 是否对称：同一 peer(打洞) socket 向两个不同 STUN 采址，比较外部映射端口。
 * 端口稳定 -> 非对称(CONE，可打洞)；端口改变 -> 对称型(SYMMETRIC，需 TURN)。
 * 结束后恢复以 hostA 作为公告地址的映射(mapped_*)。返回 1/0/-1，并写 stun->nat_type。
 */
static int __probe(Stun *stun, char *hostA, char *serviceA,
                   char *hostB, char *serviceB)
{
    char host[64] = {0};
    int port = 0, ret = -1;

    TRY {
        THROW_IF(stun->peer_client == NULL, -1);
        THROW_IF(hostA == NULL || serviceA == NULL ||
                 hostB == NULL || serviceB == NULL, -1);

        /* 第一个 STUN：保留其映射作为公告地址基准 */
        EXEC(stun->discovery(stun, hostA, serviceA));
        snprintf(host, sizeof(host), "%s", stun->mapped_host);
        port = stun->mapped_port;

        /* 第二个 STUN：比较外部映射端口是否改变 */
        EXEC(stun->discovery(stun, hostB, serviceB));

        if (stun->mapped_port == port) {
            stun->nat_type = STUN_NAT_TYPE_CONE;
            ret = 1;
            dbg_str(DBG_INFO, "NAT probe: external port stable %d==%d, NON-symmetric (can punch)",
                    port, stun->mapped_port);
        } else {
            stun->nat_type = STUN_NAT_TYPE_SYMMETRIC;
            ret = 0;
            dbg_str(DBG_ERROR, "NAT probe: external port changed %d!=%d, SYMMETRIC (need TURN)",
                    port, stun->mapped_port);
        }

        /* 恢复公告映射为 hostA(主 STUN) 的结果 */
        EXEC(stun->discovery(stun, hostA, serviceA));
    } CATCH (ret) {
        stun->nat_type = STUN_NAT_TYPE_UNKNOWN;
    }

    return ret;
}

/* 经 server_client 向信令中心查询对端地址（同步等待结果） */
static int __lookup_addr(Stun *stun, char *peer_id, char *host, int host_len, int *port)
{
    char buf[64];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->server_client == NULL || peer_id == NULL ||
                 host == NULL || port == NULL, -1);
        snprintf(buf, sizeof(buf), "GET %s\n", peer_id);
        EXEC(client_send(stun->server_client, buf, (int)strlen(buf), 0));
        stun->lookup_done = 0;
        for (i = 0; i < 30 && !stun->lookup_done; i++) {
            usleep(100000);
        }
        THROW_IF(!stun->lookup_done, -1);
        THROW_IF(stun->lookup_port == 0, -1);
        snprintf(host, host_len, "%s", stun->lookup_host);
        *port = stun->lookup_port;
    } CATCH (ret) {
    }

    return ret;
}

/* 经 peer(打洞) client 向对端“建链打洞”：
 *  1) 记录对端地址 stun->remote（保活线程发往的目标）；
 *  2) 复位 data_received、置 send_punch（收到对端包后上报一次 PUNCHOK）；
 *  3) 立刻发一个 KEEPALIVE（打洞/首包）；
 *  4) 启动保活线程周期发 KEEPALIVE。
 * 返回：0=成功；-1=失败。
 */
static int __punch(Stun *stun, char *host, char *service)
{
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];
    int interval_ms;
    int ret = 0;

    TRY {
        THROW_IF(stun->peer_client == NULL || host == NULL || service == NULL, -1);

        snprintf(stun->remote.host, sizeof(stun->remote.host), "%s", host);
        stun->remote.port = atoi(service);

        msg = (stun_p2p_msg_t *)buf;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_KEEPALIVE;   /* 首包也用 KEEPALIVE（同为打洞/保活） */
        msg->len = 0;
        EXEC(stun->send_peer(stun, host, service, buf, sizeof(buf)));
        dbg_str(DBG_INFO, "punch -> %s:%d", stun->remote.host, stun->remote.port);

        /* 进入建链：复位状态并启动保活（仅当线程未运行时） */
        stun->data_received = 0;
        stun->send_punch = 1;
        if (stun->stop && stun->keepalive_start != NULL) {
            interval_ms = (stun->keepalive_interval_ms > 0)
                          ? stun->keepalive_interval_ms : 1000;
            stun->keepalive_start(stun, interval_ms);
        }
    } CATCH (ret) {
    }

    return ret;
}

/* 经 peer(打洞) client 向当前对端(remote)发送业务数据（封装 P2P DATA 消息） */
static int __send(Stun *stun, void *buf, int len)
{
    char pkt[sizeof(stun_p2p_msg_t) + STUN_P2P_MAX_PAYLOAD];
    stun_p2p_msg_t *msg;
    char service[16];
    int ret = 0;

    TRY {
        THROW_IF(stun->peer_client == NULL, -1);
        THROW_IF(len < 0 || len > STUN_P2P_MAX_PAYLOAD, -1);
        THROW_IF(stun->remote.host[0] == 0, -1);

        msg = (stun_p2p_msg_t *)pkt;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_DATA;
        msg->len = htons(len);
        memcpy(msg->data, buf, len);
        snprintf(service, sizeof(service), "%d", stun->remote.port);
        EXEC(stun->send_peer(stun, stun->remote.host, service, pkt,
                             (int)(sizeof(stun_p2p_msg_t) + len)));
    } CATCH (ret) {
    }

    return ret;
}

/*
 * 保活线程：周期向对端(peer_client, sendto)发 PUNCH(建链期)/KEEPALIVE；
 * 收到对端包后，经 server_client 上报一次 PUNCHOK 并转 KEEPALIVE。
 */
static void *__keepalive_routine(void *arg)
{
    Stun *stun = (Stun *)arg;
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];
    char line[96], service[16];

    while (!stun->stop) {
        usleep(stun->keepalive_interval_ms * 1000);
        if (stun->stop) {
            break;
        }
        if (stun->send_punch && stun->data_received) {
            /* 本端打洞成功(收到对端包)：经 server_client 上报 PUNCHOK */
            stun->send_punch = 0;
            snprintf(line, sizeof(line), "PUNCHOK %s %s", stun->id, stun->peer_id);
            stun->send_server(stun, line);
            dbg_str(DBG_INFO, "[%s] punch ok, PUNCHOK sent to signaling", stun->id);
        }
        if (stun->remote.host[0] != 0) {
            msg = (stun_p2p_msg_t *)buf;
            msg->magic = htonl(STUN_P2P_MAGIC);
            /* 统一发 KEEPALIVE：它同时承担打洞(刷 NAT 映射)与保活；对端收到任一
             * P2P 包(KEEPALIVE/DATA)都算可达并据此上报 PUNCHOK。 */
            msg->type = STUN_P2P_MSG_KEEPALIVE;
            msg->len = 0;
            snprintf(service, sizeof(service), "%d", stun->remote.port);
            {
                int sr = stun->send_peer(stun, stun->remote.host, service,
                                         buf, sizeof(buf));
                dbg_str(DBG_INFO, "[%s] keepalive SEND type=%d -> %s:%s ret=%d",
                        stun->id, msg->type, stun->remote.host, service, sr);
            }
        }
    }

    return NULL;
}

static int __keepalive_start(Stun *stun, int interval_ms)
{
    int ret = 0;

    TRY {
        THROW_IF(interval_ms <= 0, -1);
        stun->stop = 0;
        stun->keepalive_interval_ms = interval_ms;
        THROW_IF(pthread_create(&stun->keepalive_thread, NULL,
                                __keepalive_routine, stun) != 0, -1);
    } CATCH (ret) {
    }

    return ret;
}

static int __keepalive_stop(Stun *stun)
{
    int ret = 0;

    TRY {
        if (stun->stop) {
            ret = 0;   /* 线程从未启动，直接返回，避免对无效线程 pthread_join */
        } else {
            stun->stop = 1;
            THROW_IF(pthread_join(stun->keepalive_thread, NULL) != 0, -1);
        }
    } CATCH (ret) {
    }

    return ret;
}

/*
 * server_client 统一收包回调：只处理中心服务器信令文本回复
 * （OK / PEER host port / NOPEER / WAIT / CONNECTED）。
 */
static int __stun_signal_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Stun *stun = (Stun *)t->opaque;
    char *s;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL, -1);
        THROW_IF(t->buf_len <= 0, -1);

        s = (char *)t->buf;
        dbg_str(DBG_INFO, "[%s] signal text recv from %s:%s len=%d: %.*s",
                stun->id, t->remote_host, t->remote_service, t->buf_len,
                t->buf_len < 60 ? t->buf_len : 60, s);

        if (strncmp(s, "OK", 2) == 0) {
            stun->register_done = 1;
        } else if (strncmp(s, "PEER", 4) == 0) {
            /* GET 查询结果：PEER <host> <port> [<nat>] —— 只登记查询结果，不自动打洞 */
            int n = sscanf(s, "PEER %63s %d %d",
                           stun->lookup_host, &stun->lookup_port, &stun->peer_nat_type);
            if (n >= 2) {
                if (n < 3) stun->peer_nat_type = STUN_NAT_TYPE_UNKNOWN;
                stun->lookup_done = 1;
            }
        } else if (strncmp(s, "MATCH", 5) == 0) {
            /* 撮合回执：对方已 ACCEPT，MATCH <host> <port> <nat> = 对端打洞地址，立即打洞 */
            char host[64] = {0};
            int port = 0, nat = 0;
            if (sscanf(s, "MATCH %63s %d %d", host, &port, &nat) >= 2) {
                char svc[16];
                stun->peer_nat_type = nat;
                if (!stun->connected) {
                    snprintf(svc, sizeof(svc), "%d", port);
                    stun->punch(stun, host, svc);   /* punch 内部启动保活周期发 KEEPALIVE */
                    dbg_str(DBG_INFO, "[%s] MATCH %s:%d, punching to peer",
                            stun->id, host, port);
                }
            }
        } else if (strncmp(s, "NOPEER", 6) == 0) {
            stun->lookup_done = 1;
            stun->lookup_port = 0;
            stun->peer_nat_type = STUN_NAT_TYPE_UNKNOWN;
            stun->dialing = 0;   /* 对端不在线/被拒：本次呼叫失败 */
        } else if (strncmp(s, "INVITE", 6) == 0) {
            /* 被叫：服务器把 INVITE 投到信令口 ->
             * 自动 ACCEPT(愿配合打洞)并按文本里 caller 的打洞地址开始打洞 */
            char from[32] = {0}, host[64] = {0}, line[64];
            int port = 0;
            if (!stun->connected &&
                sscanf(s, "INVITE %31s %63s %d", from, host, &port) >= 3) {
                char svc[16];
                snprintf(stun->peer_id, sizeof(stun->peer_id), "%s", from);
                snprintf(line, sizeof(line), "ACCEPT %s", from);
                stun->send_server(stun, line);
                snprintf(svc, sizeof(svc), "%d", port);
                stun->punch(stun, host, svc);   /* punch 内部启动保活周期发 KEEPALIVE */
                dbg_str(DBG_INFO, "[%s] INVITE from %s (%s:%d), auto ACCEPT, punching",
                        stun->id, from, host, port);
            }
        } else if (strncmp(s, "CONNECTED", 9) == 0) {
            /* 主叫：服务器确认双方打洞都成功 -> 本端可通信 */
            stun->connected = 1;
            dbg_str(DBG_VIP, "[%s] server CONNECTED (caller), link up", stun->id);
        }
    } CATCH (ret) {
    }

    return ret;
}

/*
 * peer_client 统一收包回调：
 *  - STUN magic 开头 → Binding 响应，解析公网映射地址到 mapped_host/mapped_port；
 *  - STUN_P2P_MAGIC 开头 → P2P 消息（PUNCH/KEEPALIVE/DATA，来自对端）；
 *  - 其它文本 → 被叫侧服务器信令（INVITE / CONNECTED，投递到公告地址即本口）。
 */
static int __stun_peer_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Stun *stun = (Stun *)t->opaque;
    Response *resp;
    stun_header_t *h;
    stun_p2p_msg_t *msg;
    stun_attrib_t *attr = NULL;
    int ret = 0;

    TRY {
        THROW_IF(stun == NULL, -1);
        resp = stun->response;
        THROW_IF(t->buf_len <= 0, -1);

        h = (stun_header_t *)t->buf;
        if (t->buf_len >= (int)sizeof(stun_header_t) &&
            ntohl(h->magic_cookie) == STUN_MAGIC_COOKIE) {
            /* STUN 响应（discovery/probe 采址） */
            dbg_str(DBG_INFO, "[%s] STUN recv len=%d from %s:%s msgtype=%04x",
                    stun->id, t->buf_len, t->remote_host, t->remote_service,
                    ntohs(h->msgtype));
            EXEC(resp->buffer->write(resp->buffer, t->buf, t->buf_len));
            ret = resp->read(resp);
            THROW_IF(ret < 0, -1);

            if (resp->header->msgtype == STUN_BINDRESP) {
                /* 优先 XOR-MAPPED-ADDRESS（RFC 5389），回退 MAPPED-ADDRESS
                 * 注意：Map.search 返回 1 表示找到 */
                attr = NULL;
                if (resp->attribs->search(resp->attribs, (void *)STUN_ATR_TYPE_XOR_MAPPED_ADDR,
                                          (void **)&attr) != 1) {
                    attr = NULL;
                }
                if (attr == NULL) {
                    resp->attribs->search(resp->attribs, (void *)STUN_ATR_TYPE_MAPPED_ADDR,
                                          (void **)&attr);
                }
                if (attr != NULL) {
                    snprintf(stun->mapped_host, sizeof(stun->mapped_host), "%s",
                             attr->u.mapped_address.host);
                    stun->mapped_port = atoi(attr->u.mapped_address.service);
                    dbg_str(DBG_INFO, "stun mapped address: %s:%d",
                            stun->mapped_host, stun->mapped_port);
                }
            }
        } else if (t->buf_len >= (int)sizeof(stun_p2p_msg_t) &&
                   ntohl(((stun_p2p_msg_t *)t->buf)->magic) == STUN_P2P_MAGIC) {
            /* P2P 消息（对端打洞/保活/数据） */
            msg = (stun_p2p_msg_t *)t->buf;
            switch (msg->type) {
            case STUN_P2P_MSG_KEEPALIVE:
                /* 收到对端任何 P2P 包(保活/数据)都证明“对端→本端”可达，
                 * 据此上报本端 PUNCHOK，否则服务器永远收不齐两端。 */
                stun->data_received = 1;
                break;
            case STUN_P2P_MSG_DATA:
                stun->data_received = 1; /* stun_peer_run 据此判定已互通 */
                if (stun->recv_callback != NULL) {
                    stun->recv_callback(stun, msg->data, ntohs(msg->len));
                }
                break;
            default:
                break;
            }
        }
    } CATCH (ret) {
    }

    return ret;
}

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
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
        dbg_str(DBG_DETAIL, "parse maped addr, host:%s, server:%s",
                out->u.mapped_address.host, out->u.mapped_address.service);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __parse_attrib_changed_addr(stun_attrib_t *raw, stun_attrib_t *out)
{
    int ret;
    int i, len,  family;
    uint8_t *p, *host;

    TRY {
        family = raw->u.changed_address.family;
        SET_CATCH_INT_PARS(family, 0);
        THROW_IF(family != 0x1 && family != 2, -1);
        snprintf(out->u.changed_address.service, 8, "%d", ntohs(raw->u.changed_address.port));
        len = family == 0x1 ? 4 : 8;
        p = raw->u.changed_address.ip;
        host = out->u.changed_address.host;
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
        dbg_str(DBG_DETAIL, "parse changed addr, host:%s, server:%s",
                out->u.changed_address.host, out->u.changed_address.service);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun, connect, __connect),
    Init_Vfunc_Entry(4, Stun, discovery, __discovery),
    Init_Vfunc_Entry(5, Stun, lookup_addr, __lookup_addr),
    Init_Vfunc_Entry(6, Stun, punch, __punch),
    Init_Vfunc_Entry(7, Stun, send, __send),
    Init_Vfunc_Entry(8, Stun, keepalive_start, __keepalive_start),
    Init_Vfunc_Entry(9, Stun, keepalive_stop, __keepalive_stop),
    Init_Vfunc_Entry(10, Stun, set_recv_callback, __set_recv_callback),
    Init_Vfunc_Entry(11, Stun, register_addr, __register_addr),
    Init_Vfunc_Entry(12, Stun, probe, __probe),
    Init_Vfunc_Entry(13, Stun, connect_peer, __connect_peer),
    Init_Vfunc_Entry(14, Stun, send_server, __send_server),
    Init_Vfunc_Entry(15, Stun, send_peer, __send_peer),
    Init_End___Entry(16, Stun),
};
REGISTER_CLASS(Stun, stun_class_info);

attrib_parse_policy_t g_stun_parse_attr_policies[] = {
    {STUN_ATR_TYPE_MAPPED_ADDR,       __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_XOR_MAPPED_ADDR,   __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_CHANGED_ADDRESS,   __parse_attrib_changed_addr},
};
int g_stun_parse_attr_policies_count =
    sizeof(g_stun_parse_attr_policies) / sizeof(g_stun_parse_attr_policies[0]);
