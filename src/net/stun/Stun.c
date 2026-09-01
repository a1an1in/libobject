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
        stun->stun_host = host;
        stun->stun_service = service;
        EXEC(client_trustee(stun->c, NULL, __stun_client_callback, stun));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "stun connect failed, ret=%d", ret);
    }

    return ret;
}

/*
 * 向中心服务器发起 STUN Binding Request（异步）。
 * 响应到达后由 __stun_client_callback 解析出公网映射地址写入 mapped_host/mapped_port。
 */
static int __discovery(Stun *stun)
{
    Request *req = stun->req;
    int ret = 0;

    TRY {
        THROW_IF(stun->c == NULL, -1);
        THROW_IF(stun->stun_host == NULL || stun->stun_service == NULL, -1);
        /* 若 punch 已切换对端，重新连接回中心服务器再查询 */
        EXEC(client_connect(stun->c, stun->stun_host, stun->stun_service));
        EXEC(req->set_head(req, STUN_BINDREQ, 0, STUN_MAGIC_COOKIE));
        EXEC(client_send(stun->c, req->header, req->get_len(req), 0));
    } CATCH (ret) {
    }

    return ret;
}

/* 向中心服务器注册本 peer（同步等待确认） */
static int __register_addr(Stun *stun, char *id)
{
    char buf[64];
    int ret = 0, i;

    TRY {
        THROW_IF(stun->c == NULL || id == NULL, -1);
        snprintf(buf, sizeof(buf), "REG %s\n", id);
        EXEC(client_send(stun->c, buf, (int)strlen(buf), 0));
        stun->register_done = 0;
        for (i = 0; i < 30 && !stun->register_done; i++) {
            usleep(100000);
        }
        THROW_IF(!stun->register_done, -1);
    } CATCH (ret) {
    }

    return ret;
}

/* 向中心服务器查询对端地址（同步等待结果） */
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
        dbg_str(NET_SUC, "punch -> %s:%d", stun->remote.host, stun->remote.port);
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
                    dbg_str(NET_SUC, "stun mapped address: %s:%d",
                            stun->mapped_host, stun->mapped_port);
                }
            }
        } else if (t->buf_len >= (int)sizeof(stun_p2p_msg_t) &&
                   ntohl(((stun_p2p_msg_t *)t->buf)->magic) == STUN_P2P_MAGIC) {
            /* P2P 消息 */
            msg = (stun_p2p_msg_t *)t->buf;
            switch (msg->type) {
            case STUN_P2P_MSG_PUNCH:
                dbg_str(NET_SUC, "peer punch received, p2p connected");
                break;
            case STUN_P2P_MSG_KEEPALIVE:
                break;
            case STUN_P2P_MSG_DATA:
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
            } else if (sscanf(s, "PEER %63s %d", stun->lookup_host, &stun->lookup_port) == 2) {
                stun->lookup_done = 1;
            } else if (strncmp(s, "NOPEER", 6) == 0) {
                stun->lookup_done = 1;
                stun->lookup_port = 0;
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

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun, connect, __connect),
    Init_Vfunc_Entry(4, Stun, discovery, __discovery),
    Init_Vfunc_Entry(5, Stun, register_addr, __register_addr),
    Init_Vfunc_Entry(6, Stun, lookup_addr, __lookup_addr),
    Init_Vfunc_Entry(7, Stun, punch, __punch),
    Init_Vfunc_Entry(8, Stun, send, __send),
    Init_Vfunc_Entry(9, Stun, keepalive_start, __keepalive_start),
    Init_Vfunc_Entry(10, Stun, keepalive_stop, __keepalive_stop),
    Init_Vfunc_Entry(11, Stun, set_recv_callback, __set_recv_callback),
    Init_End___Entry(12, Stun),
};
REGISTER_CLASS(Stun, stun_class_info);

attrib_parse_policy_t g_stun_parse_attr_policies[] = {
    {STUN_ATR_TYPE_MAPPED_ADDR,       __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_XOR_MAPPED_ADDR,   __parse_attrib_mapped_addr},
    {STUN_ATR_TYPE_CHANGED_ADDRESS,   __parse_attrib_changed_addr},
};
int g_stun_parse_attr_policies_count =
    sizeof(g_stun_parse_attr_policies) / sizeof(g_stun_parse_attr_policies[0]);
