/**
 * @file test_p2p.c
 * @Synopsis  P2P 模块自测：在线节点 p2p_session_open/connect + 中心服务器 P2p_Server
 *
 * 模型：两个 peer 都各自 p2p_session_open（上线；被叫由 Stun 内部自动应答打洞）；
 * 要连别人时由发起方 p2p_session_connect（异步，发 CALL 即返回）；连接是否成功以
 * 服务器为准：双方各自打洞成功上报(PUNCHOK)，服务器收齐回 CONNECTED，is_connected
 * 变 0 即打通。open 不阻塞；业务数据仅在打通后经 recv 回调。
 *
 * 运行方式（构建后，均建议加 --log-type=0，--log-level 用 0x16）：
 *   1) 三进程本机回环：
 *        # 终端1 中心服务器（P2p_Server：信令+撮合+STUN 回显）
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000
 *        # 终端2 被叫 B：不带 peer 参数 = 只 open，常驻在线可被叫
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerB 19002 127.0.0.1 9000
 *        # 终端3 发起方 A：带 peer 参数 = open 后主动 connect peerB
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerA 19001 127.0.0.1 9000 peerB
 *   2) 真机跨 NAT（STUN 缺省用公共默认 cloudflare/google，通常不用传；仅特殊环境追加覆盖）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345
 *        # 机器 B 常驻在线可被叫（4 参数）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerB 12346 10.10.10.115 12345
 *        # 机器 A 主动连 B（5 参数，<S> 为信令服务器地址）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peerA 19001 119.4.206.14 12345 peerB
 *
 * @author Zoo
 * @date 2026-08-13
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

/* 两个公共 STUN 的默认值（跨 NAT 打洞采址），通常不必传参 */
#define P2P_STUN1_DEFAULT_HOST "stun.cloudflare.com"
#define P2P_STUN1_DEFAULT_PORT "3478"
#define P2P_STUN2_DEFAULT_HOST "stun1.l.google.com"
#define P2P_STUN2_DEFAULT_PORT "19302"

/* ======================= demo 上下文 / 回调 ======================= */
typedef struct p2p_demo_ctx_s {
    char id[32];
    int recv_count;
} p2p_demo_ctx_t;

static int p2p_on_recv(void *opaque, const uint8_t *buf, int len)
{
    p2p_demo_ctx_t *ctx = (p2p_demo_ctx_t *)opaque;
    char msg[64];

    ctx->recv_count++;
    snprintf(msg, sizeof(msg), "%.*s", len < 63 ? len : 63, (const char *)buf);
    dbg_str(DBG_INFO, "[%s] recv %d bytes: %s", ctx->id, len, msg);
    return 0;
}

/* ======================= 测试命令 ======================= */

/* 1) 中心服务器（P2p_Server：信令 REG/GET/BYE + 撮合 CALL/ACCEPT/REJECT/
 *    PUNCHOK/CONNECTED + STUN 回显；预留 TURN），阻塞至 Ctrl+C。可选参数：端口 */
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
 * 2) 单 peer（peer 节点）：
 *    test_p2p_peer <id> <local_port> <signal_host> <signal_port>
 *                  [<peer_id> [<stun_host> <stun_port>]]
 *  - 带 <peer_id>：本端为发起方——open 上线后 p2p_session_connect(peer_id)，打通后互发 hello；
 *  - 不带 <peer_id>：本端为被叫/常驻——只 p2p_session_open 上线，等对方 CALL（Stun 自动
 *    应答打洞），收到对端数据即互发返回。
 * 返回 1=互通；0=未互通；负=失败。
 */
static int test_p2p_peer(TEST_ENTRY *entry, int argc, char **argv)
{
    p2p_cfg_t cfg;
    p2p_demo_ctx_t ctx;
    p2p_session_t *s = NULL;
    char msg[64], lp[16];
    const char *id, *peer_id = NULL;
    struct event_base *eb = event_base_get_default_instance();
    int i, dial, result = 0;

    const char *stun1_host, *stun1_port, *stun2_host, *stun2_port;

    /* 参数个数区分主/被叫与 STUN 覆盖（argv[0]=命令名，argv[1..] 为实参）：
     *   4 个参数(<id> <lp> <sh> <sp>)                        -> 被叫(只 open)
     *   5 个参数(<id> <lp> <sh> <sp> <peer_id>)               -> 主叫(open+connect)
     *   (STUN 两个公共服务器用默认值，通常不必传；内网/特殊场景才追加覆盖)
     *   6/7 个参数(... <stun1_h> <stun1_p> [...])             -> 被叫/主叫 + 覆盖 STUN1
     *   8/9 个参数(... <stun1_h> <stun1_p> <stun2_h> <stun2_p> [..]) -> 再覆盖 STUN2
     */
    if (argc < 5) {
        dbg_str(DBG_ERROR, "usage: test_p2p_peer <id> <local_port> <signal_host> <signal_port>"
                " [<peer_id>]\n"
                "  STUN 缺省取公共服务器(%s:%s / %s:%s)，可追加 "
                "[<stun1_h> <stun1_p> [<stun2_h> <stun2_p>]] 覆盖\n"
                "  带 <peer_id> = 主叫(主动 connect)；不带 = 被叫(只 open 等被叫)",
                P2P_STUN1_DEFAULT_HOST, P2P_STUN1_DEFAULT_PORT,
                P2P_STUN2_DEFAULT_HOST, P2P_STUN2_DEFAULT_PORT);
        return -1;
    }
    id = argv[1];
    peer_id = NULL;
    if (argc == 6 || argc >= 8) {
        peer_id = argv[5];   /* 5 个参数/7+ 个参数时 argv[5] 为 <peer_id> */
    }

    /* STUN 默认两个公共服务器 */
    stun1_host = P2P_STUN1_DEFAULT_HOST;
    stun1_port = P2P_STUN1_DEFAULT_PORT;
    stun2_host = P2P_STUN2_DEFAULT_HOST;
    stun2_port = P2P_STUN2_DEFAULT_PORT;
    if (argc == 7) {                 /* 被叫 + STUN1: argv[5..6] */
        stun1_host = argv[5];
        stun1_port = argv[6];
    } else if (argc >= 8) {          /* 主叫 + STUN1: argv[6..7] */
        stun1_host = argv[6];
        stun1_port = argv[7];
    }
    if (argc == 9) {                 /* 被叫 + STUN1 + STUN2: argv[7..8] */
        stun2_host = argv[7];
        stun2_port = argv[8];
    } else if (argc >= 10) {         /* 主叫 + STUN1 + STUN2: argv[8..9] */
        stun2_host = argv[8];
        stun2_port = argv[9];
    }

    snprintf(lp, sizeof(lp), "%d", atoi(argv[2]));
    memset(&cfg, 0, sizeof(cfg));
    cfg.id             = id;
    cfg.peer_id        = peer_id;
    cfg.local_service  = lp;
    cfg.signal_host    = argv[3];
    cfg.signal_service = argv[4];
    cfg.stun_host      = stun1_host;
    cfg.stun_service   = stun1_port;
    cfg.stun2_host     = stun2_host;
    cfg.stun2_service  = stun2_port;
    cfg.interval_ms    = 200;
    cfg.timeout_ms     = 60000;
    dial               = (peer_id != NULL) ? 1 : 0;

    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx.id), "%s", id);

    if (p2p_session_open(&s, p2p_on_recv, &cfg, &ctx) != 0) {
        dbg_str(DBG_ERROR, "[%s] open failed", id);
        return -1;
    }
    dbg_str(DBG_INFO, "[%s] online%s", id, dial ? ", connect to peer" : ", wait to be called");

    if (dial) {
        if (p2p_session_connect(s) < 0) {   /* >=0 即成功（已发出 CALL） */
            dbg_str(DBG_ERROR, "[%s] connect failed", id);
            p2p_session_close(s);
            return -1;
        }
    }

    /* 等打通（发起方 CONNECTED 置位；被叫由对端数据驱动）；Ctrl+C 立即退出 */
    for (i = 0; i < 300; i++) {
        if (p2p_session_is_connected(s) == 0 || ctx.recv_count > 0 ||
            (eb != NULL && eb->eb != NULL && eb->eb->break_flag)) {
            break;
        }
        usleep(200000);
    }

    if (p2p_session_is_connected(s) == 0) {
        dbg_str(DBG_VIP, "[%s] connected (server-confirmed)", id);
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
        dbg_str(DBG_INFO, "[%s] receiving peer data (auto-answered)", id);
        snprintf(msg, sizeof(msg), "hello from %s", id);
        for (i = 0; i < 40; i++) {
            p2p_session_send(s, (const uint8_t *)msg, (int)strlen(msg));
            if (eb != NULL && eb->eb != NULL && eb->eb->break_flag) {
                break;
            }
            usleep(100000);
        }
        result = 1;
    } else {
        dbg_str(DBG_ERROR, "[%s] not connected within timeout", id);
    }

    dbg_str(DBG_INFO, "[%s] result=%d", id, result);
    p2p_session_close(s);
    return result;
}
REGISTER_TEST_CMD(test_p2p_peer);

/* ======================= 本机回环自动测试（进程内） ======================= */
/* 起 1 个 P2p_Server + 两端：B 先 open 在线(被叫)，A 后 open+connect 主动连 B；
 * Stun 内部自动撮合打洞，服务器确认双方 PUNCHOK 后回 CONNECTED。双向互通即 PASS。 */
typedef struct p2p_loop_arg_s {
    const char *id;
    const char *peer_id;
    int  local_port;
    const char *service;
    int  result;
} p2p_loop_arg_t;

static int p2p_loop_node(const char *id, const char *peer_id, int local_port,
                         const char *service)
{
    p2p_cfg_t cfg;
    p2p_demo_ctx_t ctx;
    p2p_session_t *s = NULL;
    char lp[16], msg[64];
    int i, result = 0;

    snprintf(lp, sizeof(lp), "%d", local_port);
    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx.id), "%s", id);
    memset(&cfg, 0, sizeof(cfg));
    cfg.id             = id;
    cfg.peer_id        = peer_id;
    cfg.local_service  = lp;
    cfg.signal_host    = (char *)"127.0.0.1";
    cfg.signal_service = service;
    cfg.stun_host      = (char *)"127.0.0.1";
    cfg.stun_service   = service;
    cfg.interval_ms    = 200;
    cfg.timeout_ms     = 20000;

    if (p2p_session_open(&s, p2p_on_recv, &cfg, &ctx) != 0) {
        dbg_str(DBG_ERROR, "[%s] open failed", id);
        return -1;
    }
    if (peer_id != NULL && p2p_session_connect(s) < 0) {   /* >=0 即成功（已发出 CALL） */
        dbg_str(DBG_ERROR, "[%s] connect failed", id);
        p2p_session_close(s);
        return -1;
    }

    /* 等打通（发起方 CONNECTED 置位；被叫由对端数据驱动） */
    for (i = 0; i < 150; i++) {
        if (p2p_session_is_connected(s) == 0 || ctx.recv_count > 0) {
            break;
        }
        usleep(200000);
    }
    if (p2p_session_is_connected(s) == 0) {
        dbg_str(DBG_VIP, "[%s] connected (server-confirmed)", id);
        snprintf(msg, sizeof(msg), "hello from %s", id);
        for (i = 0; i < 80; i++) {
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
    dbg_str(DBG_INFO, "[%s] result=%d", id, result);
    p2p_session_close(s);
    return result;
}

static void *p2p_loop_thread(void *arg)
{
    p2p_loop_arg_t *a = (p2p_loop_arg_t *)arg;

    a->result = p2p_loop_node(a->id, a->peer_id, a->local_port, a->service);
    return NULL;
}

static int test_p2p_loopback(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    P2p_Server *server = NULL;
    p2p_loop_arg_t b;
    pthread_t tb;
    char service[16];
    int Aresult = 0, ret = 0;

    snprintf(service, sizeof(service), "%d", P2P_SERVER_DEFAULT_PORT);
    TRY {
        server = object_new(allocator, "P2p_Server", NULL);
        THROW_IF(server == NULL, -1);
        EXEC(server->start(server, (char *)"127.0.0.1", service));

        memset(&b, 0, sizeof(b));
        b.id = "peerB";
        b.local_port = 19002;
        b.service = service;
        THROW_IF(pthread_create(&tb, NULL, p2p_loop_thread, &b) != 0, -1);
        usleep(500000);               /* B 先上线(在线) */

        Aresult = p2p_loop_node("peerA", "peerB", 19001, service);  /* A 后连接 */
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
