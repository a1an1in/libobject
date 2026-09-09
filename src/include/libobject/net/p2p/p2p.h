#ifndef __P2P_H__
#define __P2P_H__

#include <stdint.h>

/*
 * P2P 网络模块（对外接口）。
 *
 * 目录/职责：
 *   - P2p_Server    : 中心服务器（信令 REG/GET/BYE + 撮合 CALL/ACCEPT/REJECT/
 *                     PUNCHOK/CONNECTED + STUN 回显；预留 TURN 中继）。
 *   - p2p_session_* : 一个 peer 节点 = 一个“在线会话”：
 *                       · p2p_session_open：构造 + 上线(REG) + 内置处理服务器
 *                         信令/预打洞消息（不阻塞）；
 *                       · 作为被叫收到 CALL 时内置自动应答打洞；
 *                       · 要主动连别人时由发起方调 p2p_peer_connect；
 *                       · 连接“真正成功”以服务器为准：双方各自打洞成功后上报
 *                         (PUNCHOK)，服务器收齐两边上报判定两端 ok，回 CONNECTED；
 *                       · 打洞成功后才置 connected，业务数据(打洞后)经 recv 回调，
 *                         打洞前的握手/保活消息内置消化，不外发。
 *   - p2p_server_run: 运行中心服务器。
 *
 * 本公共头不暴露内部实现（打洞客户端 stun/、P2p_Server 只在内部 .c 引用）。
 */

/* ===== 业务配置（扁平） ===== */
typedef struct p2p_cfg_s {
    /* 身份 / 本地绑定 */
    const char *id;             /* 本 peer 注册 id */
    const char *peer_id;        /* 对端 id（p2p_peer_connect 的目标） */
    const char *local_host;     /* 本地绑定 host，可空(默认 0.0.0.0) */
    const char *local_service;  /* 本地端口(打洞口)，如 "12346" */

    /* 服务器 */
    const char *signal_host;    /* 信令中心(P2p_Server)地址 */
    const char *signal_service; /* 信令中心端口 */
    const char *stun_host;      /* 第一个公共 STUN 地址(采址) */
    const char *stun_service;   /* 第一个公共 STUN 端口 */
    const char *stun2_host;     /* 可选：第二个公共 STUN 地址(NAT 对称探测) */
    const char *stun2_service;  /* 可选：第二个公共 STUN 端口 */

    /* 周期 / 超时 */
    int interval_ms;            /* 打洞/保活周期，如 1000 */
    int timeout_ms;             /* 建链/连接最长等待，如 60000 */

    /* TURN 中继（预留，后续接入 src/net/turn） */
    const char *turn_host;      /* TURN 服务器地址（可空） */
    const char *turn_service;   /* TURN 服务器端口（可空） */
} p2p_cfg_t;

/* ===== 在线节点（peer session） ===== */

typedef struct p2p_session_s p2p_session_t;  /* 不透明节点句柄 */

/* 业务收包回调：仅在“打洞成功建立链路后”收到对端业务数据时调用。
 * data/len 仅在回调内有效。 */
typedef int (*p2p_recv_fn)(void *opaque, const uint8_t *data, int len);

/*
 * 构造本端 P2P 节点并上线（**不阻塞**）：
 *  - 建打洞客户端、连信令中心、采址/探测 NAT、向服务器 REG（上线）；
 *  - 返回后本端在线，并**内置**处理信令/预打洞消息：
 *      · 作为被叫收到服务器 INVITE -> 自动 ACCEPT(愿意配合打洞) -> 打洞对齐
 *        -> 上报服务器 -> 服务器确认双方 ok 回 CONNECTED -> 置 connected；
 *      · 作为发起方，p2p_peer_connect 所需的 PEER/NOPEER/CONNECTED 通知同样内置。
 *  - 打洞成功前的握手消息不外发；打洞成功后对端业务数据经 recv 上报。
 * @param out    [out] 节点句柄(用 p2p_session_close 释放=下线)
 * @param recv   业务收包回调(仅打洞成功后)
 * @param cfg    配置(cfg->peer_id 为将来 p2p_peer_connect 的目标)
 * @param opaque 传给 recv 的上下文
 * 返回：0=已上线(在线、可被叫、可 connect)；-1=失败。
 */
int p2p_session_open(p2p_session_t **out, p2p_recv_fn recv,
                     const p2p_cfg_t *cfg, void *opaque);

/*
 * 主动连接 cfg->peer_id 的目标（仅发起方，本端须已 p2p_session_open）——**异步**：
 *  - 发 CALL 即返回；后续(INVITE/ACCEPT/PEER/打洞/PUNCHOK/CONNECTED)由 Stun 内部
 *    自动完成，无需轮询线程；
 *  - 结果查询 p2p_session_is_connected()：变为 0 即打通(服务器已确认双方打洞成功，
 *    可 p2p_session_send)。
 * @param s 已 open(上线) 的节点
 * 返回：0=已发出 CALL(进行中)；-1=失败(未上线/参数错)。
 */
int p2p_session_connect(p2p_session_t *s);

/* 向对端发送业务包。返回 0 成功；负值失败(未打通/已关闭/参数错)。 */
int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len);

/* 查询节点是否已建立链路：0=已打通(可 send)；-1=未建立/已关闭。 */
int p2p_session_is_connected(p2p_session_t *s);

/* 关闭节点：停内置处理与保活、释放句柄并向服务器 BYE 下线。幂等。 */
int p2p_session_close(p2p_session_t *s);

/* 查询节点是否在线：0=在线；负值=已关闭/无效。 */
int p2p_session_is_alive(p2p_session_t *s);

/* ===== 中心服务器 ===== */

/*
 * 运行中心服务器（P2p_Server），阻塞直到 Ctrl+C(SIGINT)。
 * @param host    监听地址，如 "0.0.0.0"
 * @param service 监听端口（UDP）
 * 返回：0=正常停止；-1=失败。
 */
int p2p_server_run(const char *host, const char *service);

#endif
