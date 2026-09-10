#ifndef __P2P_H__
#define __P2P_H__

#include <stdint.h>

/*
 * P2P 网络模块（对外接口）——多会话模型。
 *
 * 概念（与内部 Stun 对齐，去 peer_id，用 stun id）：
 *  - p2p 节点(node) = 一个 Stun 节点（stun_id、与信令服务器的常驻会话、会话表）。
 *    先 p2p_node_create 上线(SIGNIN)，后 p2p_node_close 下线(SIGNOUT)。
 *  - p2p 会话(session) = 一条到某 remote stun id 的链路；每会话独立 UDP 数据口，
 *    打洞目标(会话地址)经信令交换。主叫 p2p_session_create 发起 CALL(异步)；被叫
 *    收到 INVITE 自动建会话并配合打洞。
 *  - 连接"真正成功"以服务器为准：双方各自打洞成功上报(PUNCHOK)，服务器收齐回
 *    CONNECTED；p2p_session_is_connected 变 0 即打通，可 send。
 *
 * 目录/职责：
 *   - P2p_Server    : 中心服务器（SIGNIN/SIGNOUT 登记 + CALL/INVITE/ACCEPT/PUNCHOK 撮合 +
 *                     STUN 回显；预留 TURN 中继）。
 *   - p2p_node_*    : 节点(Stun) 生命周期。
 *   - p2p_session_* : 一条链路。
 *   - p2p_server_run: 运行中心服务器。
 */
typedef struct p2p_node_s p2p_node_t;        /* 不透明节点句柄 */
typedef struct p2p_session_s p2p_session_t;  /* 不透明会话句柄 */

/* 业务收包回调：某会话收到对端业务数据时调用；session 为该会话句柄，
 * data/len 仅在回调内有效。 */
typedef int (*p2p_recv_fn)(void *opaque, p2p_session_t *session,
                           const uint8_t *data, int len);

/* ===== 业务配置 ===== */
typedef struct p2p_cfg_s {
    /* 身份 / 本地绑定 */
    const char *stun_id;        /* 本节点 stun id（SIGNIN 上报，被寻址用） */
    const char *local_host;     /* 本地绑定 host，可空(默认 0.0.0.0)；会话 data socket 绑它 */
    const char *local_service;  /* peer/data socket 本地端口，可空=NULL 随机；指定便于安全组放行/验证 */

    /* 信令中心(必填) */
    const char *signal_host;
    const char *signal_service;

    /* 公共 STUN 采址服务器（可空=用信令服务器自身做采址，同机/loopback 可用） */
    const char *stun_host;
    const char *stun_service;
    /* 第二 STUN 采址服务器（可空=不做 nat 对称探测）；可配成信令服(它兼 STUN 回显) */
    const char *stun2_host;
    const char *stun2_service;

    /* 周期 */
    int interval_ms;            /* 打洞/保活周期，如 200 */

    /* TURN 中继（预留） */
    const char *turn_host;
    const char *turn_service;
} p2p_cfg_t;

/* ===== 节点 ===== */

/*
 * 创建节点并上线(SIGNIN，同步等服务器 OK)：
 *  - 内部建 Stun、连信令服务器、定 stun_id 并 SIGNIN；收包回调桥接 node 级 recv。
 *  - 返回后节点在线，可 p2p_session_create 主动连别人，也可被叫（INVITE 自动应答打洞）。
 * @param out    [out] 节点句柄(用 p2p_node_close 下线释放)
 * @param recv   节点默认业务收包回调(被叫自动会话 / 未 config 的会话都走它；session 定位对端)
 * @param cfg    配置（stun_id 必填；stun_host 为空则用信令服务器采址）
 * @param opaque 传给 recv 的上下文
 * 返回：0=在线；-1=失败。
 */
int p2p_node_create(p2p_node_t **out, p2p_recv_fn recv,
                    const p2p_cfg_t *cfg, void *opaque);

/* 关闭节点：下线(SIGNOUT)、关闭全部会话、释放内部 Stun 与句柄。幂等。 */
int p2p_node_close(p2p_node_t *node);

/* 查询节点是否在线：0=在线；-1=已关闭/无效。 */
int p2p_node_is_alive(p2p_node_t *node);

/* ===== 会话 ===== */

/*
 * 创建一条到 remote_stun_id 的会话并异步发起 CALL。**不阻塞**：
 *  - Stun 为 remote 建会话(data socket+采址)，发 CALL(带本会话地址)，等撮合回执；
 *  - 打通与否查 p2p_session_is_connected()：变 0 即服务器已确认双方打洞成功。
 * @param node 已上线节点
 * @param remote_stun_id 目标节点 stun id
 * @param out  [out] 会话句柄(用 p2p_session_close 关闭；节点关闭时会一并关闭)
 * 返回：0=已发起；-1=失败。
 */
int p2p_session_create(p2p_node_t *node, const char *remote_stun_id,
                       p2p_session_t **out);

/* 配置会话：绑定该会话业务收包回调与上下文（覆盖节点默认）。可空 recv=沿用节点默认。 */
int p2p_session_config(p2p_session_t *s, p2p_recv_fn recv, void *opaque);

/* 向该会话对端发业务包。返回 0 成功；负值失败(未通/已关闭/参数错)。 */
int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len);

/* 查询该会话是否打通：0=可 send；-1=未建立/已关闭。 */
int p2p_session_is_connected(p2p_session_t *s);

/* 关闭该会话：停保活、关 data socket、从节点会话表移除。幂等。 */
int p2p_session_close(p2p_session_t *s);

/* ===== 中心服务器 ===== */

/*
 * 运行中心服务器（P2p_Server），阻塞直到 Ctrl+C(SIGINT)。
 * @param host    监听地址，如 "0.0.0.0"
 * @param service 监听端口（UDP）
 * 返回：0=正常停止；-1=失败。
 */
int p2p_server_run(const char *host, const char *service);

#endif
