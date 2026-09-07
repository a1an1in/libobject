/**
 * @file test_p2p.c
 * @Synopsis  UDP P2P 穿透 demo：STUN 服务器（兼任地址交换）+ peer 打洞/保活
 *
 * 运行方式（构建后）：
 *   1) 单进程全流程验证（本机，自动起 STUN 服务器 + 两个 peer）：
 *        ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x6 test_p2p_punch
 *   2) 真机部署：
 *        # 在公网机器上启动 STUN 服务器（STUN + 地址簿/信令一体）：
 *        ./sysroot/linux/x86_64/bin/xtools mockery test_p2p_server 3478
 *        # NAT 后的 peer 1 / peer 2：
 *        ./sysroot/linux/x86_64/bin/xtools mockery test_p2p_peer peer1 19001 <server_host> 3478 peer2
 *        ./sysroot/linux/x86_64/bin/xtools mockery test_p2p_peer peer2 19002 <server_host> 3478 peer1
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
#include "../../src/net/stun/Stun.h"
#include "../../src/net/stun/Stun_Server.h"

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
    dbg_str(NET_SUC, "[%s] recv %d bytes: %s", ctx->id, len, ctx->last_msg);
    return 0;
}

/*
 * 运行一个 P2P peer：
 *  - 连接中心服务器（STUN+信令一体）
 *  - discovery：查询自己的公网映射地址
 *  - register_addr：注册自己（服务器记录 UDP 源地址）
 *  - lookup_addr：查询对端地址
 *  - punch：打洞；send：互发业务数据；keepalive：保活
 */
static int p2p_peer_run(const char *id, int local_port,
                        const char *server_host, int server_port,
                        const char *peer_id)
{
    allocator_t *allocator = allocator_get_default_instance();
    p2p_demo_ctx_t ctx;
    Stun *stun = NULL;
    char local_port_str[16], server_port_str[16], peer_port_str[16];
    char peer_host[64];
    int peer_port, ret = 0, i;
    char msg[128];

    memset(&ctx, 0, sizeof(ctx));
    snprintf(ctx.id, sizeof(ctx.id), "%s", id);
    snprintf(local_port_str, sizeof(local_port_str), "%d", local_port);
    snprintf(server_port_str, sizeof(server_port_str), "%d", server_port);

    TRY {
        stun = object_new(allocator, "Stun", NULL);
        THROW_IF(stun == NULL, -1);
        stun->opaque = &ctx;
        stun->local_service = local_port_str;
        EXEC(stun->set_recv_callback(stun, p2p_on_recv));

        /* 连接中心服务器（STUN 查询 + 信令共用同一 UDP socket） */
        EXEC(stun->connect(stun, (char *)server_host, server_port_str));

        /* 查询自己的公网映射地址 */
        EXEC(stun->discovery(stun));
        for (i = 0; i < 50 && stun->mapped_port == 0; i++) {
            usleep(100000);
        }
        dbg_str(NET_SUC, "[%s] my mapped address: %s:%d",
                id, stun->mapped_host, stun->mapped_port);

        /* 注册自己（服务器记录 UDP 源地址为公网映射地址） */
        EXEC(stun->register_addr(stun, (char *)id));

        /* 查询对端地址（成功返回 1，EXEC 仅检查 <0） */
        EXEC(stun->lookup_addr(stun, (char *)peer_id,
                               peer_host, sizeof(peer_host), &peer_port));
        dbg_str(NET_SUC, "[%s] peer %s address: %s:%d", id, peer_id, peer_host, peer_port);

        /* 打洞：向对端公网地址发送打洞包，建立 NAT 映射 */
        snprintf(peer_port_str, sizeof(peer_port_str), "%d", peer_port);
        EXEC(stun->punch(stun, peer_host, peer_port_str));
        usleep(200000);

        /* 互发业务数据 */
        snprintf(msg, sizeof(msg), "hello from %s", id);
        EXEC(stun->send(stun, msg, (int)strlen(msg)));
        dbg_str(NET_SUC, "[%s] sent: %s", id, msg);

        /* 保活：周期性发送 keepalive 维持 NAT 映射 */
        EXEC(stun->keepalive_start(stun, 1000));

        /* 等待对端数据 */
        for (i = 0; i < 30 && ctx.recv_count == 0; i++) {
            usleep(100000);
        }
        if (ctx.recv_count > 0) {
            dbg_str(NET_SUC, "[%s] P2P OK, received: %s", id, ctx.last_msg);
            ret = 1;
        } else {
            dbg_str(DBG_ERROR, "[%s] P2P FAIL, no data received", id);
            ret = -1;
        }

        EXEC(stun->keepalive_stop(stun));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "[%s] p2p_peer_run failed, ret=%d", id, ret);
    } FINALLY {
        if (stun != NULL) {
            object_destroy(stun);
        }
    }

    return ret;
}

/* ======================= 测试命令 ======================= */

/* 1) 启动 STUN 服务器（中心服务器，兼任地址交换/信令），阻塞。
 *    可选参数：端口（默认 9000） */
static int test_p2p_server(TEST_ENTRY *entry, int argc, char **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Stun_Server *server = NULL;
    char port_str[16];
    int port = STUN_SERVER_DEFAULT_PORT;
    int ret = 0;

    if (argc > 0) {
        port = atoi(argv[0]);
    }
    snprintf(port_str, sizeof(port_str), "%d", port);

    TRY {
        server = object_new(allocator, "Stun_Server", NULL);
        THROW_IF(server == NULL, -1);
        EXEC(server->start(server, (char *)"0.0.0.0", port_str));
        dbg_str(NET_SUC, "stun server running on port %d, Ctrl+C to stop", port);
        while (1) {
            sleep(1);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_p2p_server failed, ret=%d", ret);
    } FINALLY {
        if (server != NULL) {
            object_destroy(server);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_p2p_server);

typedef struct peer_thread_arg_s {
    char id[16];
    int local_port;
    char peer_id[16];
    int result;
} peer_thread_arg_t;

static void *peer_thread(void *arg)
{
    peer_thread_arg_t *a = (peer_thread_arg_t *)arg;

    a->result = p2p_peer_run(a->id, a->local_port,
                             "127.0.0.1", STUN_SERVER_DEFAULT_PORT, a->peer_id);
    return NULL;
}

/* 2) 单进程全流程验证：STUN 服务器 + 两个 peer（本机） */
static int test_p2p_punch(TEST_ENTRY *entry, int argc, char **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Stun_Server *server = NULL;
    peer_thread_arg_t pa, pb;
    pthread_t ta, tb;
    char port_str[16];
    int ret = 0;

    snprintf(port_str, sizeof(port_str), "%d", STUN_SERVER_DEFAULT_PORT);

    TRY {
        server = object_new(allocator, "Stun_Server", NULL);
        THROW_IF(server == NULL, -1);
        EXEC(server->start(server, (char *)"127.0.0.1", port_str));

        memset(&pa, 0, sizeof(pa));
        memset(&pb, 0, sizeof(pb));
        snprintf(pa.id, sizeof(pa.id), "%s", "peerA");
        pa.local_port = 19001;
        snprintf(pa.peer_id, sizeof(pa.peer_id), "%s", "peerB");
        snprintf(pb.id, sizeof(pb.id), "%s", "peerB");
        pb.local_port = 19002;
        snprintf(pb.peer_id, sizeof(pb.peer_id), "%s", "peerA");

        pthread_create(&ta, NULL, peer_thread, &pa);
        pthread_create(&tb, NULL, peer_thread, &pb);
        pthread_join(ta, NULL);
        pthread_join(tb, NULL);

        dbg_str(DBG_VIP, "peerA result: %d, peerB result: %d", pa.result, pb.result);
        THROW_IF(!(pa.result > 0 && pb.result > 0), -1);
        dbg_str(NET_SUC, "P2P PUNCH TEST PASSED");
        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "P2P PUNCH TEST FAILED, ret=%d", ret);
    } FINALLY {
        if (server != NULL) {
            object_destroy(server);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_p2p_punch);

/* 3) 单 peer（真机部署）
 * 参数: <id> <local_port> <server_host> <server_port> <peer_id>
 */
static int test_p2p_peer(TEST_ENTRY *entry, int argc, char **argv)
{
    const char *id, *server_host, *peer_id;
    int local_port, server_port;

    if (argc < 5) {
        dbg_str(DBG_ERROR,
                "usage: test_p2p_peer <id> <local_port> <server_host> <server_port> <peer_id>");
        return -1;
    }
    id = argv[0];
    local_port = atoi(argv[1]);
    server_host = argv[2];
    server_port = atoi(argv[3]);
    peer_id = argv[4];

    return p2p_peer_run(id, local_port, server_host, server_port, peer_id);
}
REGISTER_TEST_CMD(test_p2p_peer);
