#ifndef __P2P_H__
#define __P2P_H__

#include <stdint.h>

/*
 * P2P 网络模块（对外接口）。
 *
 * 目录/职责：
 *   - P2p_Server  : 中心服务器（同进程提供 信令 REG/GET(含 nat_type) + STUN 回显，
 *                  预留 TURN 中继），见 net/p2p/P2p_Server。
 *   - p2p_peer_run: 对外的“建立到对端的一条 P2P 通道”入口（本头）。
 *                   内部先尝试 STUN 打洞（打洞客户端在 net/p2p/stun），两端都对称时
 *                   需要 TURN 中继（src/net/turn，尚未接入）返回 -2。
 *
 * 本公共头不暴露内部实现类型；打洞客户端句柄以不透明的 struct Stun_s 出现。
 */

struct Stun_s;   /* 建链后用于收发的会话句柄（不透明） */

/* 对外业务配置（扁平；内部映射到打洞客户端 stun_peer_cfg_t） */
typedef struct p2p_cfg_s {
    /* 身份 / 本地绑定 */
    const char *id;             /* 本 peer 注册 id */
    const char *peer_id;        /* 对端 id */
    const char *local_host;     /* 本地绑定 host，可空(默认 0.0.0.0) */
    const char *local_service;  /* 本地端口(打洞口)，如 "12346" */

    /* 服务器 */
    const char *signal_host;    /* 信令中心(P2p_Server)地址 */
    const char *signal_service; /* 信令中心端口 */
    const char *stun_host;      /* 第一个公共 STUN 地址(采址) */
    const char *stun_service;   /* 第一个公共 STUN 端口 */
    const char *stun2_host;     /* 可选：第二个公共 STUN 地址(NAT 对称探测) */
    const char *stun2_service;  /* 可选：第二个公共 STUN 端口 */

    /* 业务 */
    int (*recv_callback)(struct Stun_s *stun, uint8_t *buf, int len); /* 业务数据回调 */
    void *opaque;                /* 业务回调上下文 */

    /* 发送 / 超时 */
    const uint8_t *payload;     /* 打洞后周期发送的业务数据，可空(只打洞不主动发) */
    int payload_len;
    int interval_ms;            /* 发送周期(兼保活)，如 1000 */
    int timeout_ms;             /* 总时长/最长等待 */

    /* TURN 中继（预留，后续接入 src/net/turn） */
    const char *turn_host;      /* TURN 服务器地址（可空） */
    const char *turn_service;   /* TURN 服务器端口（可空） */
} p2p_cfg_t;

/*
 * 建立到对端的一条 P2P 通道（同步阻塞，直至互通 / 需 TURN / 超时）：
 *  1) 采址(公共 STUN) + 探测本端 NAT，经信令 REG/GET 交换对端地址与类型；
 *  2) 任一端非对称 → STUN 打洞并互发数据；
 *  3) 两端都对称 → 打洞不可行，走 TURN 决策口（TURN 尚未实现，返回 -2）。
 * 返回：
 *   1  = 已互通（打洞直连）；
 *   0  = 找到对端但未收到其数据（或打洞后未打通）；
 *   -1 = 失败；
 *   -2 = 双对称 NAT，需 TURN 中继（尚未实现 / 未配置）。
 */
int p2p_peer_run(const p2p_cfg_t *cfg);

/*
 * 运行中心服务器（P2p_Server：信令 REG/GET + STUN 回显；预留 TURN 中继），
 * 阻塞直到 Ctrl+C(SIGINT)。
 * @param host    监听地址，如 "0.0.0.0"
 * @param service 监听端口（UDP）
 * 返回：0=正常停止；-1=失败。
 */
int p2p_server_run(const char *host, const char *service);

#endif
