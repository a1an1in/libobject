/**
 * @file P2p_Server.c
 * @Synopsis  STUN/信令中心服务器：Binding 回显 + 多会话撮合(会话地址交换)
 *
 * v3 模型：每个 stun 节点连上后 SIGNIN 登记自己（服务器记其信令源地址，用于投递
 * INVITE/INVITE_REPLY/CONNECTED）。打洞目标(本会话地址)不提前上报，而由 INVITE /
 * INVITE_REPLY 一问一答交换：
 *   INVITE <caller> <callee> <caller_sess_host> <caller_sess_port> [nat]
 *      -> 服务器查 callee 在线，向 callee 信令地址转发 INVITE(带 caller 会话地址)；
 *         callee 不在线则直接回主叫 INVITE_REPLY <callee> <caller> reject 3(offline)；
 *   INVITE_REPLY <callee> <caller> accept <callee_sess_host> <port> [nat]
 *      -> 服务器原样转给 caller(带 callee 会话地址)；
 *   INVITE_REPLY <callee> <caller> reject <reason>
 *      -> 服务器转给 caller 并清掉 pending（如被叫本地端口池已满）；
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
#include <stdarg.h>          /* __server_reply 的可变参数 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <libobject/concurrent/work_task.h>
#include "stun/Stun.h"      /* STUN_REJECT_*（原因码）/ STUN_NAT_TYPE_* */
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
    } CATCH (ret) { }

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

/* ---------------- 信令处理：表驱动分派 ----------------
 * 每个信令一个处理函数，签名统一 (server, task)，返回 0=已处理 / -1=坏包。
 * 新增信令只需加一个处理函数 + 在 g_stun_sig_table 加一行。分派要求关键字是"整词"
 * （后面跟空格或行尾），所以 INVITE 不会误吃 INVITE_REPLY。 */
typedef int (*stun_sig_handler_t)(P2p_Server *server, work_task_t *t);

typedef struct stun_sig_entry_s {
    const char        *key;      /* 文本关键字（单行信令的第一个词） */
    const char        *name;     /* 名称，仅用于日志/排障 */
    stun_sig_handler_t handle;   /* 处理函数 */
} stun_sig_entry_t;

static int __sig_signin(P2p_Server *server, work_task_t *t);
static int __sig_signout(P2p_Server *server, work_task_t *t);
static int __sig_invite(P2p_Server *server, work_task_t *t);
static int __sig_invite_reply(P2p_Server *server, work_task_t *t);
static int __sig_punchok(P2p_Server *server, work_task_t *t);

static const stun_sig_entry_t g_stun_sig_table[] = {
    { "SIGNIN",       "SIGNIN",       __sig_signin },
    { "SIGNOUT",      "SIGNOUT",      __sig_signout },
    { "INVITE",       "INVITE",       __sig_invite },
    { "INVITE_REPLY", "INVITE_REPLY", __sig_invite_reply },
    { "PUNCHOK",      "PUNCHOK",      __sig_punchok },
};
#define STUN_SIG_TABLE_NUM \
    (sizeof(g_stun_sig_table) / sizeof(g_stun_sig_table[0]))

/* 组一条文本发给 host:service（信令口；fmt 自带 '\n'）。
 * 注意：STUN Binding 响应是二进制（含 NUL），不能走这里。 */
static void __server_reply(P2p_Server *server, char *host, char *service,
                           const char *fmt, ...)
{
    char buf[512];
    va_list ap;

    if (server == NULL || server->client == NULL || host == NULL || service == NULL) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    server->client->socket->sendto(server->client->socket, buf, (int)strlen(buf), 0,
                                   host, service);
}

/*
 * UDP client 统一收包回调，两条路径：
 *   1) STUN Binding 请求（二进制）-> 回显公网映射地址；
 *   2) 文本信令 -> 查 g_stun_sig_table 分派给对应处理函数。
 */
static int __stun_server_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    P2p_Server *server = (P2p_Server *)t->opaque;
    stun_header_t *h;
    uint8_t resp[512];
    char line[512];
    size_t i;
    int ret = 0;

    TRY {
        THROW_IF(server == NULL || server->client == NULL || t->buf_len <= 0, 1);
        dbg_str(DBG_DETAIL, "server received %d bytes from %s:%s", t->buf_len,
                t->remote_host, t->remote_service);

        /* ---- 1) STUN Binding Request -> 回显请求者的公网映射地址 ---- */
        h = (stun_header_t *)t->buf;
        if (t->buf_len >= (int)sizeof(stun_header_t) &&
            ntohl(h->magic_cookie) == STUN_MAGIC_COOKIE) {
            Socket *socket = server->client->socket;
            stun_header_t *rh = (stun_header_t *)resp;
            uint8_t *attr = resp + sizeof(stun_header_t);
            uint16_t port;
            uint32_t cookie = ntohl(h->magic_cookie);
            uint32_t cookie_be = htonl(cookie);
            uint8_t ip[4];
            int resp_len;

            THROW_IF(inet_pton(AF_INET, t->remote_host, ip) != 1, 1);
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
            socket->sendto(socket, resp, resp_len, 0, t->remote_host,
                           t->remote_service);
            THROW(1);      /* Binding 已处理：单一出口 */
        }

        /* ---- 2) 文本信令：按关键字查表分派 ---- */
        ((char *)t->buf)[t->buf_len] = 0;
        snprintf(line, sizeof(line), "%s", (char *)t->buf);
        for (i = 0; i < STUN_SIG_TABLE_NUM; i++) {
            size_t klen = strlen(g_stun_sig_table[i].key);

            if (strncmp(line, g_stun_sig_table[i].key, klen) != 0 ||
                (line[klen] != ' ' && line[klen] != '\0')) {
                continue;
            }
            dbg_str(DBG_DETAIL, "server handle signal %s", g_stun_sig_table[i].name);
            THROW(g_stun_sig_table[i].handle(server, t));   /* 透传 handler 结果 */
        }
        dbg_str(DBG_DETAIL, "server ignore unknown signal: %.32s", line);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* SIGNIN <stun_id>：登记/更新节点信令地址（INVITE 投递用），回 SIGNIN_REPLY <stun_id>。 */
static int __sig_signin(P2p_Server *server, work_task_t *t)
{
    char id[32];
    Map *stuns = server->stuns;
    allocator_t *allocator = server->obj.allocator;
    stun_node_t *n = NULL, *new_node;

    if (sscanf((char *)t->buf, "SIGNIN %31s", id) != 1) {
        return -1;
    }
    dbg_str(DBG_INFO, "SIGNIN: %s registered (src %s:%s)", id, t->remote_host,
            t->remote_service);
    pthread_mutex_lock(&server->lock);
    stuns->search(stuns, (void *)id, (void **)&n);
    if (n == NULL) {
        new_node = (stun_node_t *)allocator_mem_alloc(allocator, sizeof(stun_node_t));
        if (new_node != NULL) {
            snprintf(new_node->id, sizeof(new_node->id), "%s", id);
            snprintf(new_node->signal_host, sizeof(new_node->signal_host), "%s",
                     t->remote_host);
            new_node->signal_port = atoi(t->remote_service);
            stuns->add(stuns, new_node->id, new_node);
        }
    } else {
        snprintf(n->signal_host, sizeof(n->signal_host), "%s", t->remote_host);
        n->signal_port = atoi(t->remote_service);
    }
    pthread_mutex_unlock(&server->lock);
    __server_reply(server, t->remote_host, t->remote_service, "SIGNIN_REPLY %s\n", id);
    return 0;
}

/* SIGNOUT <stun_id>：删除登记（无应答）。 */
static int __sig_signout(P2p_Server *server, work_task_t *t)
{
    char id[32];
    stun_node_t *n = NULL;

    if (sscanf((char *)t->buf, "SIGNOUT %31s", id) != 1) {
        return -1;
    }
    dbg_str(DBG_INFO, "SIGNOUT: %s offline (src %s:%s)", id, t->remote_host,
            t->remote_service);
    pthread_mutex_lock(&server->lock);
    server->stuns->search(server->stuns, (void *)id, (void **)&n);
    if (n != NULL) {
        server->stuns->del(server->stuns, (void *)id);
        allocator_mem_free(server->obj.allocator, n);
    }
    pthread_mutex_unlock(&server->lock);
    return 0;
}

/* INVITE <caller> <callee> <host> <port> [nat]（主叫发起呼叫）：
 *  - 被叫不在线 -> 回主叫 INVITE_REPLY reject(OFFLINE)；
 *  - 在线：建/覆盖 pending(caller|callee)，并把 INVITE 转投被叫（带主叫会话地址）。 */
static int __sig_invite(P2p_Server *server, work_task_t *t)
{
    allocator_t *allocator = server->obj.allocator;
    char from_id[32], to_id[32], sess_host[64], key[96], sport[16], callee_host[64];
    int sess_port = 0, sess_nat = 0, n_arg, callee_port = 0;
    stun_node_t *callee = NULL;
    p2p_server_call_t *call = NULL, *new_call;
    int ret = 0;

    TRY {
        n_arg = sscanf((char *)t->buf, "INVITE %31s %31s %63s %d %d", from_id, to_id,
                       sess_host, &sess_port, &sess_nat);
        THROW_IF(n_arg < 4, -1);
        if (n_arg < 5) {
            sess_nat = 0;
        }
        dbg_str(DBG_INFO, "INVITE: %s invites %s address=%s:%d nat=%d (src %s:%s)",
                from_id, to_id, sess_host, sess_port, sess_nat, t->remote_host,
                t->remote_service);

        pthread_mutex_lock(&server->lock);
        callee = NULL;
        server->stuns->search(server->stuns, (void *)to_id, (void **)&callee);
        if (callee == NULL) {
            pthread_mutex_unlock(&server->lock);
            /* 被叫不在线 -> 明确回 reject(offline)，不让主叫干等 */
            __server_reply(server, t->remote_host, t->remote_service,
                           "INVITE_REPLY %s %s reject %d\n", to_id, from_id,
                           STUN_REJECT_OFFLINE);
            THROW(1);      /* 已回复主叫：单一出口 */
        }

        new_call = (p2p_server_call_t *)allocator_mem_alloc(allocator,
                                                            sizeof(p2p_server_call_t));
        if (new_call != NULL) {
            __call_key(key, sizeof(key), from_id, to_id);
            snprintf(new_call->key, sizeof(new_call->key), "%s", key);
            snprintf(new_call->from_id, sizeof(new_call->from_id), "%s", from_id);
            /* 主叫信令源地址：INVITE_REPLY(accept/reject)/CONNECTED 都投它 */
            snprintf(new_call->from_host, sizeof(new_call->from_host), "%s",
                     t->remote_host);
            new_call->from_port = atoi(t->remote_service);
            snprintf(new_call->to_id, sizeof(new_call->to_id), "%s", to_id);
            new_call->to_host[0] = 0;  /* 被叫会话地址，等 INVITE_REPLY(accept) 填 */
            new_call->to_port = 0;
            new_call->from_nat = sess_nat;
            new_call->to_nat = 0;
            new_call->caller_ok = 0;
            new_call->callee_ok = 0;

            call = NULL;
            server->pending->search(server->pending, (void *)new_call->key,
                                    (void **)&call);
            if (call != NULL) {          /* 同一次呼叫重发：替换旧的 */
                server->pending->del(server->pending, (void *)new_call->key);
                allocator_mem_free(allocator, call);
            }
            server->pending->add(server->pending, new_call->key, new_call);
        }
        snprintf(callee_host, sizeof(callee_host), "%s", callee->signal_host);
        callee_port = callee->signal_port;
        pthread_mutex_unlock(&server->lock);

        /* 通知被叫 INVITE：带主叫 id + 主叫会话(打洞目标)地址 */
        snprintf(sport, sizeof(sport), "%d", callee_port);
        __server_reply(server, callee_host, sport, "INVITE %s %s %d %d\n", from_id,
                       sess_host, sess_port, sess_nat);
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* INVITE_REPLY <callee> <caller> <accept|reject> ...：被叫对 INVITE 的应答，转投主叫。
 * 两种动作共用一个表项（同一关键字），函数内按动作字段二次分派。 */
static int __sig_invite_reply(P2p_Server *server, work_task_t *t)
{
    char *line = (char *)t->buf;
    char to_id[32], from_id[32], sess_host[64], key[96], sport[16];
    char from_host[64];
    int sess_port = 0, sess_nat = 0, n_arg, from_port = 0;
    int reason = STUN_REJECT_NONE;
    p2p_server_call_t *call = NULL;
    int ret = 0;

    TRY {
        if (strstr(line, " accept ") != NULL) {
            /* 接受：记下被叫会话(打洞)地址，再把 reply 转给主叫（原 MATCH 的角色） */
            n_arg = sscanf(line, "INVITE_REPLY %31s %31s accept %63s %d %d", to_id,
                           from_id, sess_host, &sess_port, &sess_nat);
            THROW_IF(n_arg < 4, -1);
            if (n_arg < 5) {
                sess_nat = 0;
            }
            dbg_str(DBG_INFO,
                    "INVITE_REPLY(accept): %s accepts %s address=%s:%d nat=%d", to_id,
                    from_id, sess_host, sess_port, sess_nat);
            pthread_mutex_lock(&server->lock);
            __call_key(key, sizeof(key), from_id, to_id);
            call = NULL;
            server->pending->search(server->pending, (void *)key, (void **)&call);
            if (call != NULL) {
                snprintf(call->to_host, sizeof(call->to_host), "%s", sess_host);
                call->to_port = sess_port;
                call->to_nat = sess_nat;
                from_port = call->from_port;
                snprintf(from_host, sizeof(from_host), "%s", call->from_host);
                pthread_mutex_unlock(&server->lock);
                snprintf(sport, sizeof(sport), "%d", from_port);
                __server_reply(server, from_host, sport,
                               "INVITE_REPLY %s %s accept %s %d %d\n", to_id, from_id,
                               sess_host, sess_port, sess_nat);
            } else {
                pthread_mutex_unlock(&server->lock);   /* 无 pending：忽略 */
            }
            THROW(1);      /* accept 已处理：单一出口 */
        }

        /* 拒绝（如被叫本地数据口端口池已满）：把原因码转给主叫，并清掉这次 pending */
        THROW_IF(sscanf(line, "INVITE_REPLY %31s %31s reject %d", to_id, from_id,
                        &reason) < 2, -1);
        dbg_str(DBG_INFO, "INVITE_REPLY(reject): %s rejects %s reason=%d", to_id,
                from_id, reason);
        pthread_mutex_lock(&server->lock);
        __call_key(key, sizeof(key), from_id, to_id);
        call = NULL;
        server->pending->search(server->pending, (void *)key, (void **)&call);
        if (call != NULL) {
            from_port = call->from_port;
            snprintf(from_host, sizeof(from_host), "%s", call->from_host);
            server->pending->del(server->pending, (void *)key);
            allocator_mem_free(server->obj.allocator, call);
            pthread_mutex_unlock(&server->lock);
            snprintf(sport, sizeof(sport), "%d", from_port);
            __server_reply(server, from_host, sport,
                           "INVITE_REPLY %s %s reject %d\n", to_id, from_id, reason);
        } else {
            pthread_mutex_unlock(&server->lock);       /* 无 pending：忽略 */
        }
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
}

/* PUNCHOK <id> <peer>：双方都上报才判定打通，回双方 CONNECTED（并清掉 pending）。 */
static int __sig_punchok(P2p_Server *server, work_task_t *t)
{
    char p1[32], p2[32], key[96], sport[16], csport[16];
    char caller_host[64], callee_host[64], caller_id[32], callee_id[32];
    int caller_port = 0, callee_port = 0, have_callee = 0;
    p2p_server_call_t *call = NULL;
    stun_node_t *n = NULL;
    int ret = 0;

    TRY {
        THROW_IF(sscanf((char *)t->buf, "PUNCHOK %31s %31s", p1, p2) != 2, -1);
        pthread_mutex_lock(&server->lock);
        call = NULL;
        __call_key(key, sizeof(key), p1, p2);
        server->pending->search(server->pending, (void *)key, (void **)&call);
        if (call != NULL) {
            call->caller_ok = 1;              /* p1 = 主叫 */
        } else {
            call = NULL;
            __call_key(key, sizeof(key), p2, p1);
            server->pending->search(server->pending, (void *)key, (void **)&call);
            if (call != NULL) {
                call->callee_ok = 1;          /* p1 = 被叫 */
            }
        }
        if (call == NULL || !call->caller_ok || !call->callee_ok) {
            pthread_mutex_unlock(&server->lock);
            THROW(1);      /* 只有一侧上报：等另一侧，按成功结束 */
        }
        dbg_str(DBG_INFO, "CONNECTED: both punched ok (%s<->%s)", call->from_id,
                call->to_id);
        /* 先把投递目标拷出来，解锁后不再引用 Map 里的对象 */
        caller_port = call->from_port;
        snprintf(caller_host, sizeof(caller_host), "%s", call->from_host);
        snprintf(caller_id, sizeof(caller_id), "%s", call->to_id);
        snprintf(callee_id, sizeof(callee_id), "%s", call->from_id);
        n = NULL;
        server->stuns->search(server->stuns, (void *)call->to_id, (void **)&n);
        if (n != NULL) {
            snprintf(callee_host, sizeof(callee_host), "%s", n->signal_host);
            callee_port = n->signal_port;
            have_callee = 1;
        }
        server->pending->del(server->pending, (void *)call->key);
        allocator_mem_free(server->obj.allocator, call);
        pthread_mutex_unlock(&server->lock);

        /* 主叫 CONNECTED <被叫 id>；被叫 CONNECTED <主叫 id> */
        snprintf(sport, sizeof(sport), "%d", caller_port);
        __server_reply(server, caller_host, sport, "CONNECTED %s\n", caller_id);
        if (have_callee) {
            snprintf(csport, sizeof(csport), "%d", callee_port);
            __server_reply(server, callee_host, csport, "CONNECTED %s\n", callee_id);
        }
    } CATCH (ret) { }

    return ret;    /* 单一出口：落底 1(成功) / 抛错 <0 */
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
    } CATCH (ret) { }

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
