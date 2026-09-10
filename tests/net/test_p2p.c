/**
 * @file test_p2p.c
 * @Synopsis  P2P 模块自测：node+session 多会话 + 中心服务器 P2p_Server
 *
 * 模型：p2p_node = 一个 Stun 节点(stun_id 上线)；p2p_session = 一条到某 remote
 * stun id 的链路。发起方 p2p_session_create(异步 CALL)，打通由服务器撮合确认，
 * p2p_session_is_connected 变 0 即可 send；被叫收到 INVITE 自动建会话并配合打洞，
 * 收到对端业务数据经 recv 回调(带 session 句柄)。
 *
 * 运行方式（构建后，建议加 --log-type=0 --log-level=0x16）：
 *   1) 进程内自动回环：-f test_p2p_loopback（server + B 先上线 + A 主动连 B）
 *   2) 三进程本机回环（<local_service> 用 '-'=随机）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerB - 127.0.0.1 9000
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerA - 127.0.0.1 9000 peerB
 *   3) 真机跨 NAT：信令服务器运行在公网；每节点用公共 STUN 采址（默认
 *      cloudflare/google），特殊环境可用后四参覆盖
 *      <stun_host> <stun_port> <stun2_host> <stun2_port>：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerB 12346 10.10.10.115 12345
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerA 19001 119.4.206.14 12345 peerB
 *
 * @author Zoo
 * @date 2026-09-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/net/p2p/p2p.h>
#include "../../src/net/p2p/P2p_Server.h"

#define P2P_SERVER_DEFAULT_PORT 9000

#define P2P_STUN1_DEFAULT_HOST "stun.cloudflare.com"
#define P2P_STUN1_DEFAULT_PORT "3478"
/* 第二 STUN（nat 对称探测用；与主 STUN 不同 IP）；CLI 第 8/9 参可覆盖 */
#define P2P_STUN2_DEFAULT_HOST "stun.l.google.com"
#define P2P_STUN2_DEFAULT_PORT "19302"

/* ======================= demo 上下文 / 回调 ======================= */
typedef struct p2p_demo_ctx_s {
    char id[32];
    int  recv_count;
    int  is_callee;       /* 1=被叫(只有被叫回发 hello) */
} p2p_demo_ctx_t;

/* recv 回调：收到对端业务数据；被叫/主叫都可借 session 句柄回发 hello */
static int p2p_on_recv(void *opaque, p2p_session_t *session,
                       const uint8_t *buf, int len)
{
    p2p_demo_ctx_t *ctx = (p2p_demo_ctx_t *)opaque;
    char msg[64], reply[64];

    ctx->recv_count++;
    snprintf(msg, sizeof(msg), "%.*s", len < 63 ? len : 63, (const char *)buf);
    dbg_str(DBG_INFO, "%s received %d bytes: %s", ctx->id, len, msg);

    /* 只有被叫回一次 hello（让主叫也确认双向可达），避免双方互相 echo 无限循环 */
    if (ctx->is_callee) {
        snprintf(reply, sizeof(reply), "hello from %s", ctx->id);
        p2p_session_send(session, (const uint8_t *)reply, (int)strlen(reply));
    }
    return 0;
}

/* ======================= 测试命令 ======================= */

/* 1) 中心服务器（P2p_Server：信令 REG/BYE 登记 + CALL/INVITE/ACCEPT/PUNCHOK 撮合
 *    + STUN 回显；预留 TURN），阻塞至 Ctrl+C。可选参数：端口 */
static int test_p2p_server(TEST_ENTRY *entry, int argc, char **argv)
{
    char port_str[16];
    int port = P2P_SERVER_DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    snprintf(port_str, sizeof(port_str), "%d", port);
    return p2p_server_run("0.0.0.0", port_str);
}
REGISTER_TEST_CMD(test_p2p_server);

/*
 * 2) 单节点：
 *    test_p2p_peer <stun_id> <signal_host> <signal_port>
 *                  [<peer_id> [<stun_host> <stun_port>]]
 *  - 带 <peer_id>：本端为发起方——node 上线后 p2p_session_create(peer_id) 主动连；
 *  - 不带 <peer_id>：本端为被叫/常驻——只 node 上线，等对方 CALL（Stun 自动应答打洞）。
 * 返回 1=互通；0=未互通；负=失败。
 */
static int test_p2p_peer(TEST_ENTRY *entry, int argc, char **argv)
{
    p2p_cfg_t cfg;
    p2p_demo_ctx_t ctx;
    p2p_node_t *node = NULL;
    p2p_session_t *s = NULL;
    char msg[64];
    const char *id, *peer_id = NULL;
    const char *stun_host = P2P_STUN1_DEFAULT_HOST;
    const char *stun_port = P2P_STUN1_DEFAULT_PORT;
    const char *stun2_host = P2P_STUN2_DEFAULT_HOST;     /* 第二 STUN(对称探测) */
    const char *stun2_port = P2P_STUN2_DEFAULT_PORT;
    const char *local_service = NULL;   /* 会话(data)口固定端口，'-'=随机 */
    struct event_base *eb = event_base_get_default_instance();
    int i, dial, result = 0;

    /* 参数：<stun_id> <local_service> <signal_host> <signal_port>
     *       [<peer_id> [<stun_host> <stun_port> [<stun2_host> <stun2_port>]]]
     * local_service 紧跟 stun_id（旧格式）；'-' 表示随机口。
     * 带 <peer_id> = 主叫；不带 = 被叫(常驻等被叫)。
     * stun2 可配信令服(它兼 STUN 回显)做 nat 对称探测；不配则不探测。 */
    if (argc < 5) {
        dbg_str(DBG_ERROR, "usage: test_p2p_peer <stun_id> <local_service>"
                " <signal_host> <signal_port> [<peer_id> [<stun_host> <stun_port>"
                " [<stun2_host> <stun2_port>]]]\n"
                "  <local_service> 为本端会话口固定端口，'-'=随机(多会话/防冲突用)\n"
                "  STUN 缺省取公共服务器(%s:%s)\n"
                "  <stun2_*> 第二 STUN(nat 对称探测)，可配成信令服自身\n"
                "  带 <peer_id> = 主叫；不带 = 被叫(常驻等被叫)",
                P2P_STUN1_DEFAULT_HOST, P2P_STUN1_DEFAULT_PORT);
        return -1;
    }
    id = argv[1];
    local_service = (strcmp(argv[2], "-") != 0) ? argv[2] : NULL;
    if (argc >= 6) {
        peer_id = argv[5];
    }
    if (argc >= 8) {
        stun_host = argv[6];
        stun_port = argv[7];
    }
    if (argc >= 10) {
        stun2_host = argv[8];
        stun2_port = argv[9];
    }
    dial = (peer_id != NULL) ? 1 : 0;

    memset(&cfg, 0, sizeof(cfg));
    cfg.stun_id        = id;
    cfg.local_service  = local_service;
    cfg.signal_host    = argv[3];
    cfg.signal_service = argv[4];
    cfg.stun_host      = stun_host;
    cfg.stun_service   = stun_port;
    cfg.stun2_host     = stun2_host;
    cfg.stun2_service  = stun2_port;
    cfg.interval_ms    = 200;

    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx.id), "%s", id);
    ctx.is_callee = dial ? 0 : 1;   /* 无 peer_id=被叫 */

    if (p2p_node_create(&node, p2p_on_recv, &cfg, &ctx) != 0) {
        dbg_str(DBG_ERROR, "%s node create failed", id);
        return -1;
    }
    dbg_str(DBG_INFO, "%s online%s", id, dial ? ", calling peer" : ", waiting to be called");

    if (dial) {
        if (p2p_session_create(node, peer_id, &s) < 0) {   /* >=0 即成功(已发 CALL) */
            dbg_str(DBG_ERROR, "%s session create failed", id);
            p2p_node_close(node);
            return -1;
        }
    }

    /* 等打通（主叫 CONNECTED 置位；被叫由对端数据驱动）；Ctrl+C 立即退出 */
    for (i = 0; i < 300; i++) {
        if ((s != NULL && p2p_session_is_connected(s) == 0) || ctx.recv_count > 0 ||
            (eb != NULL && eb->eb != NULL && eb->eb->break_flag)) {
            break;
        }
        usleep(200000);
    }

    if (s != NULL && p2p_session_is_connected(s) == 0) {
        dbg_str(DBG_INFO, "%s connected (server-confirmed)", id);
        snprintf(msg, sizeof(msg), "hello from %s", id);
        for (i = 0; i < 80; i++) {
            p2p_session_send(s, (const uint8_t *)msg, (int)strlen(msg));
            if (ctx.recv_count > 0 || (eb != NULL && eb->eb != NULL &&
                                       eb->eb->break_flag)) {
                result = (ctx.recv_count > 0) ? 1 : result;
                break;
            }
            usleep(100000);
        }
    } else if (ctx.recv_count > 0) {
        dbg_str(DBG_INFO, "%s receiving peer data (auto-answered)", id);
        result = 1;
    } else {
        dbg_str(DBG_ERROR, "%s not connected within timeout", id);
    }

    dbg_str(DBG_INFO, "%s result=%d", id, result);
    if (s != NULL) {
        p2p_session_close(s);
    }
    p2p_node_close(node);
    return result;
}
REGISTER_TEST_CMD(test_p2p_peer);

/* ======================= 本机回环自动测试（进程内） ======================= */
/* 起 1 个 P2p_Server + 两端：B 先 node 上线(被叫)，A 后 node 上线并 session 连 B；
 * Stun 内部自动撮合打洞，服务器确认双方 PUNCHOK 后回 CONNECTED。双向互通即 PASS。 */
/* 常驻被叫线程参数（loopback 的 B 与 multi 的 B/C 复用） */
typedef struct p2p_thread_arg_s {
    const char *id;
    const char *service;
    const char *local_service;
    p2p_demo_ctx_t ctx;
    int  result;
} p2p_thread_arg_t;

static int p2p_loop_node(const char *id, const char *peer_id, const char *service,
                         const char *local_service)
{
    p2p_cfg_t cfg;
    p2p_demo_ctx_t ctx;
    p2p_node_t *node = NULL;
    p2p_session_t *s = NULL;
    char msg[64];
    int i, result = 0;

    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx).id, "%s", id);
    ctx.is_callee = (peer_id == NULL) ? 1 : 0;   /* 无 peer_id=被叫 */
    memset(&cfg, 0, sizeof(cfg));
    cfg.stun_id        = id;
    cfg.local_service  = local_service;   /* 固定会话口验证；NULL=随机 */
    cfg.signal_host    = (char *)"127.0.0.1";
    cfg.signal_service = service;
    /* 会话 data socket 采址：本机回环直接问信令服务器(它兼 STUN 回显) */
    cfg.stun_host      = (char *)"127.0.0.1";
    cfg.stun_service   = service;
    cfg.interval_ms    = 200;

    if (p2p_node_create(&node, p2p_on_recv, &cfg, &ctx) != 0) {
        dbg_str(DBG_ERROR, "%s node create failed", id);
        return -1;
    }
    if (peer_id != NULL) {
        if (p2p_session_create(node, peer_id, &s) < 0) {
            dbg_str(DBG_ERROR, "%s session create failed", id);
            p2p_node_close(node);
            return -1;
        }
    }

    /* 等打通（主叫 CONNECTED 置位；被叫由对端数据驱动） */
    for (i = 0; i < 150; i++) {
        if ((s != NULL && p2p_session_is_connected(s) == 0) || ctx.recv_count > 0) {
            break;
        }
        usleep(200000);
    }
    if (s != NULL && p2p_session_is_connected(s) == 0) {
        dbg_str(DBG_VIP, "%s connected (server-confirmed)", id);
        snprintf(msg, sizeof(msg), "hello from %s", id);
        for (i = 0; i < 20; i++) {
            p2p_session_send(s, (const uint8_t *)msg, (int)strlen(msg));
            if (ctx.recv_count > 0) {
                result = 1;
                break;
            }
            usleep(100000);
        }
    } else if (ctx.recv_count > 0) {
        result = 1;
    }
    dbg_str(DBG_INFO, "%s result=%d", id, result);
    if (s != NULL) {
        p2p_session_close(s);
    }
    p2p_node_close(node);
    return result;
}

/* 常驻被叫：node 上线等被 CALL，收到对端数据即 result=1。loopback/multi 复用。 */
static void *p2p_thread_callback(void *arg)
{
    p2p_thread_arg_t *a = (p2p_thread_arg_t *)arg;
    p2p_cfg_t cfg;
    p2p_node_t *node = NULL;
    int i;

    memset(&a->ctx, 0, sizeof(a->ctx));
    snprintf(a->ctx.id, sizeof(a->ctx.id), "%s", a->id);
    a->ctx.is_callee = 1;
    memset(&cfg, 0, sizeof(cfg));
    cfg.stun_id        = a->id;
    cfg.local_service  = a->local_service;
    cfg.signal_host    = (char *)"127.0.0.1";
    cfg.signal_service = a->service;
    cfg.stun_host      = (char *)"127.0.0.1";
    cfg.stun_service   = a->service;
    cfg.interval_ms    = 200;
    if (p2p_node_create(&node, p2p_on_recv, &cfg, &a->ctx) != 0) {
        dbg_str(DBG_ERROR, "%s resident create failed", a->id);
        return NULL;
    }
    /* 等被叫(收到对端数据)或超时 */
    for (i = 0; i < 200 && a->ctx.recv_count == 0; i++) {
        usleep(100000);
    }
    a->result = (a->ctx.recv_count > 0) ? 1 : 0;
    p2p_node_close(node);
    return NULL;
}

static int test_p2p_loopback(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    P2p_Server *server = NULL;
    p2p_thread_arg_t b;
    pthread_t tb;
    char service[16];
    int Aresult = 0, ret = 0;

    TRY {
        snprintf(service, sizeof(service), "%d", P2P_SERVER_DEFAULT_PORT);
        server = object_new(allocator, "P2p_Server", NULL);
        THROW_IF(server == NULL, -1);
        EXEC(server->start(server, (char *)"127.0.0.1", service));

        memset(&b, 0, sizeof(b));
        b.id = "peerB";
        b.service = service;
        b.local_service = NULL;   /* loopback 默认随机口 */
        THROW_IF(pthread_create(&tb, NULL, p2p_thread_callback, &b) != 0, -1);
        usleep(500000);               /* B 先上线(在线) */

        Aresult = p2p_loop_node("peerA", "peerB", service, NULL);  /* A 后连接(随机口) */
        pthread_join(tb, NULL);

        dbg_str(DBG_INFO, "[p2p_loopback] A=%d B=%d", Aresult, b.result);
        THROW_IF(!(Aresult == 1 && b.result == 1), -1);
        dbg_str(DBG_VIP, "P2P LOOPBACK TEST PASSED (B online, A connects later)");
        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "P2P LOOPBACK TEST FAILED, ret=%d", ret);
    } FINALLY {
        if (server != NULL) {
            object_destroy(server);
        }
    }

    return ret;
}
REGISTER_TEST_FUNC(test_p2p_loopback);

/* ======================= 三节点多路自测（A 同连 B、C） ======================= */
/* server + B/C 常驻(被叫,各自线程 node 上线) + A 主线程 node 上线并同时 call B/C；
 * 两路都 CONNECTED 后 A 分别发 hello，收到对端数据(被叫 recv 回发)判该路互通。 */

static int test_p2p_multi(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    P2p_Server *server = NULL;
    p2p_thread_arg_t rb, rc;
    pthread_t tb, tc;
    p2p_cfg_t cfg;
    p2p_demo_ctx_t actx;
    p2p_node_t *node = NULL;
    p2p_session_t *sb = NULL, *sc = NULL;
    char service[16], msg[64];
    int i, ret = 0, b_ok = 0, c_ok = 0;

    TRY {
        snprintf(service, sizeof(service), "%d", P2P_SERVER_DEFAULT_PORT);
        server = object_new(allocator, "P2p_Server", NULL);
        THROW_IF(server == NULL, -1);
        EXEC(server->start(server, (char *)"127.0.0.1", service));

        /* B、C 常驻上线 */
        memset(&rb, 0, sizeof(rb));
        rb.id = "peerB"; rb.service = service;
        memset(&rc, 0, sizeof(rc));
        rc.id = "peerC"; rc.service = service;
        THROW_IF(pthread_create(&tb, NULL, p2p_thread_callback, &rb) != 0, -1);
        THROW_IF(pthread_create(&tc, NULL, p2p_thread_callback, &rc) != 0, -1);
        usleep(600000);   /* 等 B/C 都上线 */

        /* A 上线并同时连 B、C */
        memset(&cfg, 0, sizeof(cfg));
        memset(&actx, 0, sizeof(actx));
        snprintf(actx.id, sizeof(actx.id), "%s", "peerA");
        cfg.stun_id        = "peerA";
        cfg.signal_host    = (char *)"127.0.0.1";
        cfg.signal_service = service;
        cfg.stun_host      = (char *)"127.0.0.1";
        cfg.stun_service   = service;
        cfg.interval_ms    = 200;
        THROW_IF(p2p_node_create(&node, p2p_on_recv, &cfg, &actx) != 0, -1);

        THROW_IF(p2p_session_create(node, "peerB", &sb) < 0, -1);
        THROW_IF(p2p_session_create(node, "peerC", &sc) < 0, -1);

        /* 等两路打通 */
        for (i = 0; i < 200; i++) {
            if ((p2p_session_is_connected(sb) == 0 || rb.ctx.recv_count > 0) &&
                (p2p_session_is_connected(sc) == 0 || rc.ctx.recv_count > 0)) {
                break;
            }
            usleep(100000);
        }
        dbg_str(DBG_INFO, "[p2p_multi] B=%s C=%s",
                (p2p_session_is_connected(sb) == 0) ? "conn" : "wait",
                (p2p_session_is_connected(sc) == 0) ? "conn" : "wait");

        /* A 向两路各发数据，直到各自对端回包 */
        snprintf(msg, sizeof(msg), "hello from peerA to B");
        for (i = 0; i < 80 && rb.ctx.recv_count == 0; i++) {
            p2p_session_send(sb, (const uint8_t *)msg, (int)strlen(msg));
            usleep(100000);
        }
        snprintf(msg, sizeof(msg), "hello from peerA to C");
        for (i = 0; i < 80 && rc.ctx.recv_count == 0; i++) {
            p2p_session_send(sc, (const uint8_t *)msg, (int)strlen(msg));
            usleep(100000);
        }
        b_ok = (rb.ctx.recv_count > 0) ? 1 : 0;
        c_ok = (rc.ctx.recv_count > 0) ? 1 : 0;

        dbg_str(DBG_INFO, "[p2p_multi] result B_ok=%d C_ok=%d A_recv=%d",
                b_ok, c_ok, actx.recv_count);
        THROW_IF(!(b_ok == 1 && c_ok == 1), -1);
        dbg_str(DBG_VIP, "P2P MULTI TEST PASSED (A <-> B and A <-> C)");
        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "P2P MULTI TEST FAILED, ret=%d", ret);
    } FINALLY {
        if (node != NULL) {
            p2p_node_close(node);
        }
        pthread_join(tb, NULL);
        pthread_join(tc, NULL);
        if (server != NULL) {
            object_destroy(server);
        }
    }

    return ret;
}
REGISTER_TEST_FUNC(test_p2p_multi);
