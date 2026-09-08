/**
 * @file test_p2p.c
 * @Synopsis  UDP P2P 穿透 demo：公共 STUN 采址 + 轻量信令交换 + peer 打洞/保活
 *
 * 新架构：STUN（取公网映射地址）与信令（REG/GET 地址交换）是两个不同地址。
 *
 * 运行方式（构建后，均建议加 --log-type=0，--log-level 可用 0x16 或 0x1ffff）：
 *   1) 本机回环验证（无需任何外部服务器；起 3 个子进程/终端，信令=STUN 共用）：
 *        # 终端1：信令服务器
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000
 *        # 终端2：peerA（本机打洞口 19001）
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *              test_p2p_peer peerA 19001 127.0.0.1 9000 peerB 127.0.0.1 9000
 *        # 终端3：peerB
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *              test_p2p_peer peerB 19002 127.0.0.1 9000 peerA 127.0.0.1 9000
 *   2) 真机跨 NAT（拆分架构；末尾两段为可选的第二个公共 STUN，用于 NAT 对称性探测，
 *      双方都对称时 stun_peer_run 返回 -2(需 TURN)，否则自动打洞）：
 *        # 双方可达的主机 S 起信令服务器（需放行 UDP <signal_port>）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345
 *        # 机器 A(NAT-A 后) peer1：信令走公网 S，采址+探测走两个公共 STUN
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peer1 19001 119.4.206.14 12345 peer2 stun.cloudflare.com 3478 stun1.l.google.com 3478
 *        # 机器 B(NAT-B 后，与 A 不同公网出口) peer2：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *              test_p2p_peer peer2 19002 <S> 12345 peer1 stun.cloudflare.com 3478 stun1.l.google.com 3478
 *   3) 与信令服务器同机的 peer：信令地址用本机内网 IP(勿用 127.0.0.1，否则
 *      回环先行会导致 discovery 收不到公共 STUN 回包)，采址仍走公共 STUN：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_peer peer2 12346 10.10.10.115 12345 peer1 stun.cloudflare.com 3478
 *
 * @author Zoo
 * @date 2026-08-13
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include "../../src/net/p2p/stun/Stun.h"
#include <libobject/net/p2p/p2p.h>

#define STUN_SERVER_DEFAULT_PORT 9000

/* ======================= P2P peer（基于 Stun 统一客户端） ======================= */
typedef struct p2p_demo_ctx_s {
    char id[32];
    int recv_count;
    char last_msg[256];
} p2p_demo_ctx_t;

/* Stun 统一客户端的业务数据接收回调 */
static int p2p_on_recv(Stun *stun, uint8_t *buf, int len)
{
    p2p_demo_ctx_t *ctx = (p2p_demo_ctx_t *)stun->opaque;
    int n = len < (int)sizeof(ctx->last_msg) - 1 ? len : (int)sizeof(ctx->last_msg) - 1;

    ctx->recv_count++;
    memcpy(ctx->last_msg, buf, n);
    ctx->last_msg[n] = 0;
    dbg_str(DBG_INFO, "[%s] recv %d bytes: %s", ctx->id, len, ctx->last_msg);
    return 0;
}

/* ======================= 测试命令 ======================= */

/* 1) 启动中心服务器（P2p_Server：信令 + STUN 回显，预留 TURN 中继），
 *    经对外 p2p_server_run 阻塞至 Ctrl+C。可选参数：端口（默认 9000） */
static int test_p2p_server(TEST_ENTRY *entry, int argc, char **argv)
{
    char port_str[16];
    int port = STUN_SERVER_DEFAULT_PORT;

    /* mockery 下发的 argv[0] 是命令名，真实参数从 argv[1] 开始 */
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    snprintf(port_str, sizeof(port_str), "%d", port);

    return p2p_server_run("0.0.0.0", port_str);
}
REGISTER_TEST_CMD(test_p2p_server);

/* 2) 单 peer：构造对外 p2p_cfg_t 调 p2p_peer_run（P2P 建链统一入口，
 *    内部先打洞；双对称需 TURN 返回 -2）
 * 参数: <id> <local_port> <signal_host> <signal_port> <peer_id>
 *       [<stun_host> <stun_port> [<stun2_host> <stun2_port>]]
 *   signal_* : 自己的信令服务器（P2p_Server，地址交换）
 *   stun_*   : 第一个免费公共 STUN（如 stun.cloudflare.com 3478）；省略则取信令地址(一体模式)
 *   stun2_*  : 可选第二个公共 STUN，用于 NAT 对称性探测；给了它 p2p_peer_run 会
 *              probe（两次采址比较外部端口），双方都对称时返回 -2(需 TURN)
 * 示例:
 *   真机跨 NAT（自动探测对称性）:
 *     peer1: test_p2p_peer peer1 19001 <S> 12345 peer2 stun.cloudflare.com 3478 stun1.l.google.com 3478
 *     peer2: test_p2p_peer peer2 19002 <S> 12345 peer1 stun.cloudflare.com 3478 stun1.l.google.com 3478
 *   本机回环（信令=STUN=本机，不探测）:
 *     test_p2p_peer peerA 19001 127.0.0.1 9000 peerB 127.0.0.1 9000
 */
static int test_p2p_peer(TEST_ENTRY *entry, int argc, char **argv)
{
    const char *id, *local_service, *signal_host, *signal_service;
    const char *peer_id, *stun_host, *stun_service;
    const char *stun2_host = NULL, *stun2_service = NULL;
    p2p_demo_ctx_t ctx;
    p2p_cfg_t cfg;
    char msg[128];
    int ret = 0;

    /* mockery 下发的 argv[0] 是命令名，真实参数从 argv[1] 开始 */
    if (argc < 6) {
        dbg_str(DBG_ERROR,
                "usage: test_p2p_peer <id> <local_port> <signal_host> <signal_port> <peer_id>"
                " [<stun_host> <stun_port> [<stun2_host> <stun2_port>]]");
        return -1;
    }
    id            = argv[1];
    local_service = argv[2];
    signal_host   = argv[3];
    signal_service = argv[4];
    peer_id       = argv[5];
    if (argc >= 8) {
        stun_host    = argv[6];
        stun_service = argv[7];
    } else {
        /* 省略 STUN：取信令地址，走 STUN+信令一体模式 */
        stun_host    = signal_host;
        stun_service = signal_service;
    }
    if (argc >= 10) {
        stun2_host    = argv[8];
        stun2_service = argv[9];
    }

    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx.id), "%s", id);
    snprintf(msg, sizeof(msg), "hello from %s", id);

    memset(&cfg, 0, sizeof(cfg));
    cfg.id             = id;
    cfg.peer_id        = peer_id;
    cfg.local_service  = local_service;
    cfg.signal_host    = signal_host;
    cfg.signal_service = signal_service;
    cfg.stun_host      = stun_host;
    cfg.stun_service   = stun_service;
    cfg.stun2_host     = stun2_host;
    cfg.stun2_service  = stun2_service;
    cfg.recv_callback  = p2p_on_recv;
    cfg.opaque         = &ctx;
    cfg.payload        = (uint8_t *)msg;
    cfg.payload_len    = (int)strlen(msg);
    cfg.interval_ms    = 1000;
    cfg.timeout_ms     = 60000;
    /* TURN 预留：demo 未配置 TURN，双对称时 p2p_peer_run 返回 -2 */

    ret = p2p_peer_run(&cfg);

    return ret;
}
REGISTER_TEST_CMD(test_p2p_peer);
