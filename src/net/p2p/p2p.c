/**
 * @file p2p.c
 * @Synopsis  P2P 对外建链接口 p2p_peer_run
 *
 * P2P 网络模块对外统一入口：先打洞（内部经打洞客户端 stun_peer_run），
 * 两端都对称时需 TURN 中继（src/net/turn，尚未接入）返回 -2。
 * 本文件是 p2p 模块内部实现，打洞客户端头（stun/Stun.h）只在 .c 里引用，
 * 不放进公共头。
 *
 * @author Zoo
 * @date 2026-08-13
 */

#include <string.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/net/p2p/p2p.h>
#include "stun/Stun.h"
#include "P2p_Server.h"

int p2p_peer_run(const p2p_cfg_t *cfg)
{
    stun_peer_cfg_t sc;
    Stun *stun = NULL;
    int ret = -1;

    TRY {
        THROW_IF(cfg == NULL, -1);
        THROW_IF(cfg->id == NULL || cfg->peer_id == NULL, -1);
        THROW_IF(cfg->signal_host == NULL || cfg->signal_service == NULL, -1);
        THROW_IF(cfg->stun_host == NULL || cfg->stun_service == NULL, -1);

        memset(&sc, 0, sizeof(sc));
        sc.id             = cfg->id;
        sc.peer_id        = cfg->peer_id;
        sc.local_host     = cfg->local_host;
        sc.local_service  = cfg->local_service;
        sc.signal_host    = cfg->signal_host;
        sc.signal_service = cfg->signal_service;
        sc.stun_host      = cfg->stun_host;
        sc.stun_service   = cfg->stun_service;
        sc.stun2_host     = cfg->stun2_host;
        sc.stun2_service  = cfg->stun2_service;
        sc.recv_callback  = cfg->recv_callback;
        sc.opaque         = cfg->opaque;
        sc.payload        = cfg->payload;
        sc.payload_len    = cfg->payload_len;
        sc.interval_ms    = cfg->interval_ms;
        sc.timeout_ms     = cfg->timeout_ms;

        /* 打洞路径：stun_peer_run 内部创建 Stun(经 &stun 出参)并完成
         * 采址/探测/信令/打洞/互发；本层负责释放 stun。 */
        ret = stun_peer_run(&stun, &sc);

        if (ret == -2) {
            /* 双对称 NAT：打洞不可行。TURN 中继尚未实现——预留决策口：
             * 后续若 cfg->turn_host/service 已配置，应在此用 Turn 做 relay；
             * 当前统一返回 -2，交由上层处理。 */
            dbg_str(DBG_ERROR,
                    "[%s] both peers SYMMETRIC NAT, hole punching impossible, "
                    "need TURN relay (turn=%s, not implemented yet)",
                    cfg->id,
                    (cfg->turn_host != NULL) ? cfg->turn_host : "(not configured)");
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "p2p_peer_run failed, ret=%d", ret);
    } FINALLY {
        if (stun != NULL) {
            object_destroy(stun);
        }
    }

    return ret;
}

/*
 * 运行中心服务器（P2p_Server）：信令 REG/GET + STUN 回显（预留 TURN 中继），
 * 阻塞直到 Ctrl+C(SIGINT)——框架把默认 event base 的 break_flag 置 1。
 */
int p2p_server_run(const char *host, const char *service)
{
    allocator_t *allocator = allocator_get_default_instance();
    P2p_Server *server = NULL;
    struct event_base *event_base;
    int ret = -1;

    TRY {
        THROW_IF(host == NULL || service == NULL, -1);

        server = object_new(allocator, "P2p_Server", NULL);
        THROW_IF(server == NULL, -1);

        EXEC(server->start(server, (char *)host, (char *)service));
        dbg_str(DBG_INFO, "p2p server running on %s:%s, Ctrl+C to stop", host, service);

        event_base = event_base_get_default_instance();
        while (event_base != NULL && event_base->eb->break_flag == 0) {
            sleep(1);
        }
        dbg_str(DBG_INFO, "p2p server stopped by Ctrl+C");
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "p2p_server_run failed, ret=%d", ret);
    } FINALLY {
        if (server != NULL) {
            object_destroy(server);
        }
    }

    return ret;
}
