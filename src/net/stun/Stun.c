/**
 * @file Stun.c
 * @Synopsis  Peer 统一客户端（RFC 5389 STUN + P2P 打洞）
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libobject/concurrent/work_task.h>
#include <libobject/concurrent/event_api.h>
#include "Stun.h"

static int __stun_client_callback(void *task);
static void *__keepalive_routine(void *arg);
static int __keepalive_stop(Stun *stun);

static int __construct(Stun *stun, char *init_str)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0;

    TRY {
       stun->req = object_new(allocator, "Stun_Request", NULL);
       stun->response = object_new(allocator, "Stun_Response", NULL);
       THROW_IF(stun->req == NULL || stun->response == NULL, -1);
       stun->stop = 1; /* 保活线程未启动 */

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Stun *stun)
{
    if (stun->stop == 0) {
        __keepalive_stop(stun);
    }
    if (stun->c != NULL) {
        client_destroy(stun->c);
        stun->c = NULL;
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
 * 创建 UDP client（本地绑定地址由 stun->local_host/local_service 决定），
 * 连接中心服务器（STUN+信令一体），注册统一收包回调。
 */
static int __connect(Stun *stun, char *host, char *service)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0;

    TRY {
        THROW_IF(stun->c != NULL, 1);

        stun->c = client(allocator, CLIENT_TYPE_INET_UDP,
                         (stun->local_host != NULL) ? stun->local_host : (char *)"0.0.0.0",
                         (stun->local_service != NULL) ? stun->local_service : (char *)"0");
        THROW_IF(stun->c == NULL, -1);

        EXEC(client_connect(stun->c, host, service));
        /* connect 指向的是信令中心 */
        stun->signal_host = host;
        stun->signal_service = service;
        EXEC(client_trustee(stun->c, NULL, __stun_client_callback, stun));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun connect failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 向指定的公共 STUN 服务器发 Binding（同一 UDP socket），
 * 取得本 peer 真实公网映射地址后自动切回信令中心。
 */
static int __discovery(Stun *stun, char *host, char *service)
{
    Request *req = stun->req;
    int ret = 0, i;

    TRY {
        THROW_IF(stun->c == NULL || host == NULL || service == NULL, -1);
        THROW_IF(stun->signal_host == NULL || stun->signal_service == NULL, -1);

        /* 清空上次结果，确保本次是新响应 */
        stun->mapped_host[0] = 0;
        stun->mapped_port = 0;

        /* 切到公共 STUN（同一 socket，NAT 映射端口不变） */
        EXEC(client_connect(stun->c, host, service));
        EXEC(req->set_head(req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE));
        EXEC(client_send(stun->c, req->header, req->get_len(req), 0));

        /* 等待 Binding 响应解析出公网映射地址 */
        for (i = 0; i < 50 && stun->mapped_port == 0; i++) {
            usleep(100000);
        }
        THROW_IF(stun->mapped_port == 0, -1);

        /* 切回信令中心，供后续 REG/GET 使用 */
        EXEC(client_connect(stun->c, stun->signal_host, stun->signal_service));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun discovery failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 向信令中心注册本 peer：把 discovery/discovery_to 得到的公网映射地址
 * 主动上报（REG <id> <host> <port>），登记公告地址而非观测源地址。
 */
static int __register_addr(Stun *stun, char *id)
{
    char buf[128];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->c == NULL || id == NULL, -1);
        THROW_IF(stun->signal_host == NULL || stun->signal_service == NULL, -1);
        THROW_IF(stun->mapped_host[0] == 0 || stun->mapped_port == 0, -1);

        /* 确保 socket 指向信令中心 */
        EXEC(client_connect(stun->c, stun->signal_host, stun->signal_service));

        snprintf(buf, sizeof(buf), "REG %s %s %d %d\n",
                 id, stun->mapped_host, stun->mapped_port, stun->nat_type);
        EXEC(client_send(stun->c, buf, (int)strlen(buf), 0));
        stun->register_done = 0;
        for (i = 0; i < 30 && !stun->register_done; i++) {
            usleep(100000);
        }
        THROW_IF(!stun->register_done, -1);
        dbg_str(DBG_INFO, "[%s] registered public addr %s:%d nat_type=%d to signaling",
                id, stun->mapped_host, stun->mapped_port, stun->nat_type);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun register_addr failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 探测 NAT 是否对称：同一 UDP socket 向两个不同 STUN 采址，比较外部映射端口。
 * 端口稳定 -> 非对称(CONE，可打洞)；端口改变 -> 对称型(SYMMETRIC，需 TURN)。
 * 结束后恢复以 hostA 作为公告地址的映射(mapped_*)。返回 1/0/-1，并写 stun->nat_type。
 */
static int __probe(Stun *stun, char *hostA, char *serviceA,
                   char *hostB, char *serviceB)
{
    char host[64] = {0};
    int port = 0, ret = -1;

    TRY {
        THROW_IF(stun->c == NULL || stun->signal_host == NULL, -1);
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

/* 向信令中心查询对端地址（同步等待结果） */
static int __lookup_addr(Stun *stun, char *peer_id, char *host, int host_len, int *port)
{
    char buf[64];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->c == NULL || peer_id == NULL || host == NULL || port == NULL, -1);
        snprintf(buf, sizeof(buf), "GET %s\n", peer_id);
        EXEC(client_send(stun->c, buf, (int)strlen(buf), 0));
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

/*
 * 向对端打洞：UDP connect 切换到对端地址，发送打洞包。
 * 之后 send() 会发往该对端。
 */
static int __punch(Stun *stun, char *host, char *service)
{
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];
    int ret = 0;

    TRY {
        THROW_IF(stun->c == NULL, -1);
        THROW_IF(host == NULL || service == NULL, -1);
        EXEC(client_connect(stun->c, host, service));

        snprintf(stun->remote.host, sizeof(stun->remote.host), "%s", host);
        stun->remote.port = atoi(service);

        msg = (stun_p2p_msg_t *)buf;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_PUNCH;
        msg->len = 0;
        EXEC(client_send(stun->c, buf, sizeof(stun_p2p_msg_t), 0));
        dbg_str(DBG_INFO, "punch -> %s:%d", stun->remote.host, stun->remote.port);
    } CATCH (ret) {
    }

    return ret;
}

/* 向当前对端发送业务数据（封装 P2P DATA 消息） */
static int __send(Stun *stun, void *buf, int len)
{
    char pkt[sizeof(stun_p2p_msg_t) + STUN_P2P_MAX_PAYLOAD];
    stun_p2p_msg_t *msg;
    int ret = 0;

    TRY {
        THROW_IF(stun->c == NULL, -1);
        THROW_IF(len < 0 || len > STUN_P2P_MAX_PAYLOAD, -1);

        msg = (stun_p2p_msg_t *)pkt;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_DATA;
        msg->len = htons(len);
        memcpy(msg->data, buf, len);
        EXEC(client_send(stun->c, pkt, (int)(sizeof(stun_p2p_msg_t) + len), 0));
    } CATCH (ret) {
    }

    return ret;
}

static void *__keepalive_routine(void *arg)
{
    Stun *stun = (Stun *)arg;
    stun_p2p_msg_t *msg;
    char buf[sizeof(stun_p2p_msg_t)];

    while (!stun->stop) {
        usleep(stun->keepalive_interval_ms * 1000);
        if (stun->stop) {
            break;
        }
        msg = (stun_p2p_msg_t *)buf;
        msg->magic = htonl(STUN_P2P_MAGIC);
        msg->type = STUN_P2P_MSG_KEEPALIVE;
        msg->len = 0;
        client_send(stun->c, buf, sizeof(stun_p2p_msg_t), 0);
    }

    return NULL;
}

static int __keepalive_start(Stun *stun, int interval_ms)
{
    int ret = 0;

    TRY {
        THROW_IF(stun->c == NULL, -1);
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
        stun->stop = 1;
        THROW_IF(pthread_join(stun->keepalive_thread, NULL) != 0, -1);
    } CATCH (ret) {
    }

    return ret;
}

/*
 * UDP client 统一收包回调：
 *  - STUN magic 开头 → Binding 响应，解析公网映射地址到 mapped_host/mapped_port
 *  - STUN_P2P_MAGIC 开头 → P2P 消息（PUNCH/KEEPALIVE/DATA）
 *  - 其它文本 → 中心服务器信令回复（OK / PEER host port / NOPEER）
 */
static int __stun_client_callback(void *task)
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
            /* STUN 响应 */
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
            /* P2P 消息 */
            msg = (stun_p2p_msg_t *)t->buf;
            switch (msg->type) {
            case STUN_P2P_MSG_PUNCH:
                dbg_str(DBG_INFO, "peer punch received, p2p connected");
                break;
            case STUN_P2P_MSG_KEEPALIVE:
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
        } else {
            /* 中心服务器信令回复（文本） */
            char *s = (char *)t->buf;
            if (strncmp(s, "OK", 2) == 0) {
                stun->register_done = 1;
            } else if (strncmp(s, "PEER", 4) == 0) {
                /* PEER <host> <port> [<nat_type>]，nat_type 为对端 NAT 类型 */
                int n = sscanf(s, "PEER %63s %d %d",
                               stun->lookup_host, &stun->lookup_port, &stun->peer_nat_type);
                if (n >= 2) {
                    if (n < 3) stun->peer_nat_type = STUN_NAT_TYPE_UNKNOWN;
                    stun->lookup_done = 1;
                }
            } else if (strncmp(s, "NOPEER", 6) == 0) {
                stun->lookup_done = 1;
                stun->lookup_port = 0;
                stun->peer_nat_type = STUN_NAT_TYPE_UNKNOWN;
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
        family = raw->u.mapped_address.family;
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

/*
 * 一键运行一个 peer 会话（通用入口）：
 *  - 把 local_host/local_service、opaque、recv_callback 应用到 stun；
 *  - connect(信令)；
 *  - NAT 探测：cfg->stun2_host 非空则 stun->probe(stun_host, stun2_host) 两次
 *    采址判断是否对称并写 stun->nat_type；否则 discovery(公共 STUN)，nat_type 置 UNKNOWN；
 *  - register_addr(id)（REG 上报含 nat_type）-> lookup_addr(peer_id, 带重试，
 *    对端 nat_type 写 stun->peer_nat_type)；
 *  - 本端与对端都 SYMMETRIC -> 不打洞，返回 -2(需 TURN)；
 *  - 否则 punch -> 周期发送 payload，收到对端 DATA 后再续发约 interval_ms
 *    让对端完成，或直到 timeout_ms。
 * 返回：1=已互通；0=找到对端但未收到其数据；-1=失败；-2=双对称 NAT 需 TURN。
 */
int stun_peer_run(Stun *stun, const stun_peer_cfg_t *cfg)
{
    char peer_host[64] = {0};
    char peer_port_str[16];
    char *service;
    struct event_base *event_base = NULL;
    int ret = 0, i, peer_port = 0, got = -1, step_ms, it, max_it;
    const uint8_t *data;
    int data_len;

    TRY {
        THROW_IF(stun == NULL || cfg == NULL, -1);
        THROW_IF(cfg->signal_host == NULL || cfg->signal_service == NULL, -1);
        THROW_IF(cfg->stun_host == NULL || cfg->stun_service == NULL, -1);
        THROW_IF(cfg->id == NULL || cfg->peer_id == NULL, -1);

        /* Ctrl+C(SIGINT) 时框架把默认 event base 的 break_flag 置 1，
         * 下面的等待/重试循环据此提前退出，进程才能响应中断退出。 */
        event_base = event_base_get_default_instance();

        if (cfg->local_host != NULL)    stun->local_host    = (char *)cfg->local_host;
        if (cfg->local_service != NULL) stun->local_service = (char *)cfg->local_service;
        if (cfg->opaque != NULL)        stun->opaque        = cfg->opaque;
        if (cfg->recv_callback != NULL) {
            EXEC(stun->set_recv_callback(stun, cfg->recv_callback));
        }

        stun->data_received = 0;
        stun->nat_type      = STUN_NAT_TYPE_UNKNOWN;
        stun->peer_nat_type = STUN_NAT_TYPE_UNKNOWN;

        /* 1) 连信令中心 */
        EXEC(stun->connect(stun, (char *)cfg->signal_host, (char *)cfg->signal_service));

        /* 2) NAT 探测 + 采址：
         *    给了第二 STUN(stun2) -> probe(主, 第二) 两次采址比较外部端口写 nat_type；
         *    否则 -> discovery(主 STUN) 采真实公网映射，nat_type 置 UNKNOWN。 */
        if (cfg->stun2_host != NULL && cfg->stun2_service != NULL) {
            EXEC(stun->probe(stun, (char *)cfg->stun_host, (char *)cfg->stun_service,
                             (char *)cfg->stun2_host, (char *)cfg->stun2_service));
        } else {
            EXEC(stun->discovery(stun, (char *)cfg->stun_host, (char *)cfg->stun_service));
        }
        dbg_str(DBG_INFO, "[%s] stun_peer_run mapped: %s:%d, nat_type=%d",
                cfg->id, stun->mapped_host, stun->mapped_port, stun->nat_type);

        /* 3) 公告注册（带上 nat_type） */
        EXEC(stun->register_addr(stun, (char *)cfg->id));

        /* 4) 查对端（带重试，step 500ms） */
        step_ms = 500;
        max_it = cfg->timeout_ms > 0 ? cfg->timeout_ms / step_ms : 30;
        for (i = 0; i < max_it && event_base->eb->break_flag == 0; i++) {
            ret = stun->lookup_addr(stun, (char *)cfg->peer_id,
                                    peer_host, sizeof(peer_host), &peer_port);
            if (ret >= 0) {
                break;
            }
            usleep(step_ms * 1000);
        }
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "[%s] peer %s address: %s:%d, nat_type=%d",
                cfg->id, cfg->peer_id, peer_host, peer_port, stun->peer_nat_type);

        if (stun->nat_type == STUN_NAT_TYPE_SYMMETRIC &&
            stun->peer_nat_type == STUN_NAT_TYPE_SYMMETRIC) {
            /* 5) 两端都对称：打洞必败，直接转 TURN */
            dbg_str(DBG_ERROR,
                    "[%s] local & peer both SYMMETRIC NAT, hole punching impossible, need TURN",
                    cfg->id);
            ret = -2;
        } else {
            /* 6) 打洞 */
            snprintf(peer_port_str, sizeof(peer_port_str), "%d", peer_port);
            EXEC(stun->punch(stun, peer_host, peer_port_str));
            usleep(200000);

            /* 7) 周期发送 payload，直到收到对端数据后再续发几轮或超时 */
            service = peer_port_str; /* 占位避免未使用告警 */
            (void)service;
            data     = cfg->payload;
            data_len = cfg->payload_len;
            step_ms  = cfg->interval_ms > 0 ? cfg->interval_ms : 1000;
            max_it   = cfg->timeout_ms > 0 ? cfg->timeout_ms / step_ms : 30;
            for (i = 0; i < max_it && event_base->eb->break_flag == 0; i++) {
                if (data != NULL && data_len > 0) {
                    EXEC(stun->send(stun, (void *)data, data_len));
                    dbg_str(DBG_INFO, "[%s] stun_peer_run send(%d)", cfg->id, i + 1);
                }
                if (stun->data_received) {
                    if (got < 0) {
                        got = i;
                        dbg_str(DBG_INFO, "[%s] stun_peer_run: received peer data", cfg->id);
                    }
                    if (i - got >= 5) { /* 收到后再发约 5 轮让对端也收到 */
                        break;
                    }
                }
                usleep(step_ms * 1000);
            }

            ret = (got >= 0) ? 1 : 0;
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun_peer_run failed, ret=%d", ret);
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
    Init_End___Entry(13, Stun),
};
REGISTER_CLASS(Stun, stun_class_info);

attrib_parse_policy_t g_stun_parse_attr_policies[] = {
    {STUN_ATR_TYPE_MAPPED_ADDR,       __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_XOR_MAPPED_ADDR,   __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_CHANGED_ADDRESS,   __parse_attrib_changed_addr},
};
int g_stun_parse_attr_policies_count =
    sizeof(g_stun_parse_attr_policies) / sizeof(g_stun_parse_attr_policies[0]);
