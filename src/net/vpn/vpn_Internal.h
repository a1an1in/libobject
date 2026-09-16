#ifndef __VPN_INTERNAL_H__
#define __VPN_INTERNAL_H__

/*
 * VPN 内部定义（模块私有，不对外暴露；对外只有 libobject/net/vpn/vpn.h 的
 * vpn_cfg_t + vpn_run）。
 *
 * 集中放在这里的东西：
 *   - 常量：缓冲区/等待轮次/控制帧/重发上限/断链阈值/链路表上限；
 *   - 协议：控制帧结构 vpn_ctrl_t 与两种帧的 payload 约定；
 *   - 类型：网段 vpn_net_t、链路 vpn_link_t、运行上下文 vpn_ctx_t；
 *   - 结果码 enum 与"控制帧处理函数"的分派类型。
 * 实现见 Vpn.c；Tun 抽象见 tun/Tun.h。
 */

#include <stdint.h>
#include <libobject/net/p2p/p2p.h>
#include <libobject/net/vpn/vpn.h>
#include "tun/Tun.h"

/* 出站读缓冲：需 >= 单包上限(MTU 1400 + IP 头余量)，取 4K 足够。 */
#define VPN_RW_BUF_SIZE 4096
/* 等待打通/等地址分配的最长轮次：300 * 200ms = 60s。 */
#define VPN_WAIT_ROUNDS 300
/* 转发空闲 tick 周期（毫秒）：回收断链对端 + 兜底装路由（幂等），原先由出站
 * poll 循环的 5 × 200ms 实现，现在由一个定时 worker 驱动。 */
#define VPN_FORWARD_TICK_MS 1000

/* ---- VPN 层控制帧（走 p2p 数据通道，与业务 IP 包区分）----
 * 首字节 0xFF 在合法 IP 包里不可能出现（版本号必须是 4/6），所以零误判。 */
#define VPN_CTRL_MAGIC     "\xFFVPN"
#define VPN_CTRL_MAGIC_LEN 4
#define VPN_CTRL_HDR_LEN   8
#define VPN_CTRL_NET_NOTIFY     1       /* 通告：主动方发，payload 只带本端内网网段 */
#define VPN_CTRL_NET_NOTIFY_ACK 2       /* 应答：分配对端地址 + 带回本端地址/网段 */
#define VPN_CTRL_CIDR_MAX      64       /* 控制帧 payload 上限（文本，无 '\0'） */
/* 未收到应答时，"通告"的兜底重发上限（约 1s 一次），防对端不回应时无限发。 */
#define VPN_NET_NOTIFY_MAX_RETRY 10
/* 断链回收阈值：连续这么多轮（tick，约 1s/轮）都不可用才判定断链并回收，防抖动误判。 */
#define VPN_LINK_BAD_ROUNDS 5
/* 对端链路（link）表上限：被动方最多同时接受这么多对端；每个 link 一条会话。
 * 上限是为了"固定大小的静态表"，避免为多对端引入内存分配；它同时也是地址池容量。 */
#define VPN_MAX_LINKS 8

/* 地址交换协议（**交换发生在配 tun 之前**，故两个 payload 都不依赖 tun）：
 *
 * NET_NOTIFY 由**主动建链方**发出，payload 只带它的**内网网段**（可空）：
 *   "172.16.10.0/23"   （空 = 该端没配 --local-net）
 *
 * NET_NOTIFY_ACK 由**接收方（被动方 = 地址分配者）**回，payload =
 *   "<码> <分配给对端的隧道地址/len> <本端隧道地址/len>[ <本端内网网段>]"
 *   例 "0 10.0.0.2/24 10.0.0.1/24 10.10.10.0/24"
 *   码 0 = 已接受；码非 0 = 拒绝原因码（VPN_NOTIFY_*），此时不带地址。
 * 于是：主动方拿到**自己的**隧道地址（据此配 tun）+ 被叫的地址；被动方按分配结果
 * 也就知道了每个对端的地址（"被叫应知道所有对端隧道地址"）——一个来回完成。 */
typedef struct vpn_ctrl_s {
    uint8_t  magic[VPN_CTRL_MAGIC_LEN];  /* 0xFF 'V' 'P' 'N' */
    uint8_t  type;                       /* NET_NOTIFY / NET_NOTIFY_ACK */
    uint8_t  rsvd;
    uint16_t len;                        /* payload 字节数（同构实现，主机序） */
    char     payload[VPN_CTRL_CIDR_MAX]; /* 见上（无 '\0'） */
} vpn_ctrl_t;

/* 通告处理结果 / 应答里的结果码：0=已接受；非 0=拒绝原因（放在 NET_NOTIFY_ACK 的 payload 里）。 */
enum {
    VPN_NOTIFY_OK = 0,
    VPN_NOTIFY_EBADPAYLOAD,  /* payload 空/超长/取不出字段 */
    VPN_NOTIFY_EBADIP,       /* 隧道地址不合法 */
    VPN_NOTIFY_EBADNET,      /* 内网网段不合法（含 /0、0.0.0.0/x） */
    VPN_NOTIFY_ENOADDR,      /* 地址池未配置/已满：无法给对端分配隧道地址 */
};

/* 网段（CIDR）表示 */
typedef struct vpn_net_s {
    uint32_t net;      /* 网络地址（主机序） */
    uint32_t mask;     /* 掩码（主机序） */
    int      prefix;   /* 前缀长度 1~32 */
} vpn_net_t;

/* 一条对端链路：一条 p2p 会话 + 该对端带来的隧道地址/内网网段（出站选路用）。 */
typedef struct vpn_link_s {
    p2p_session_t *session;                    /* 主动方：create 所得；被动方：recv 回调学到 */
    /* 1=本端主动建链的链路（由本端发 NET_NOTIFY）；0=被动链路（只应答，不发通告） */
    int      is_dialer;
    int      route_done;                       /* 该对端网段的路由是否已装（幂等用） */
    int      bad_rounds;                       /* 连续"N 轮不可用"计数：断链回收防抖动 */
    /* 是否已收到对端对本端通告的**应答**（NET_NOTIFY_ACK）：
     * 应答本身区分"接受/拒绝"（见 payload），这里只记"要不要再重发"。 */
    int      notify_acked;
    int      notify_retries;                   /* 已发次数（0=还没发过；也用于重发上限） */
    uint32_t peer_tun_ip;                      /* 对端隧道地址（0=还没分配/还没收到） */
    int      peer_net_ok;                      /* peer_net 是否有效（用于选路） */
    vpn_net_t peer_net;                        /* 对端内网网段（解析后） */
    char     peer_net_text[VPN_CTRL_CIDR_MAX]; /* 对端内网网段原文（""=无），用于去重 */
} vpn_link_t;

typedef struct vpn_ctx_s {
    const vpn_cfg_t *cfg;
    Tun *tun;                     /* 非空 = 已配好地址并 up（"能转发"的标志） */
    p2p_node_t *node;
    int dial;                     /* 1=主动方（主动连 cfg->peer_id）；0=被动方（等别人连） */
    int n_links;                  /* **在线**链路数（空槽被回收后会被复用，故不是数组长度） */
    vpn_link_t links[VPN_MAX_LINKS];

    /* ---- 本端隧道地址（主动方：应答里分配得到；被动方：来自 cfg） ---- */
    int      addr_ready;          /* 已拿到本端隧道地址，可以配 tun 了 */
    uint32_t my_tun_ip;           /* 本端隧道地址（主机序） */
    char     my_tun_ip_text[32];  /* 形如 "10.0.0.2/24"（configure 直接用） */
    int      notify_rounds;       /* 主动方发通告的次数（含超时重发） */

    /* ---- 地址池（被动方=分配者）：由 --tunnel-ip 的网段派生，本端地址即 --tunnel-ip ---- */
    int      pool_ok;             /* 池可用（--tunnel-ip 合法） */
    int      pool_prefix;
    uint32_t pool_net;            /* 网段地址（主机序） */
    uint32_t pool_mask;           /* 掩码（主机序） */
    /* **占用表**：槽 i 的地址 = pool_net|(i+1)，0=空闲。
     * 地址由占用表管理（而不是 link 下标），所以对端断链后归还的地址
     * 可以被后来的对端复用，且不同槽位之间永远不会撞。 */
    uint32_t addr_used[VPN_MAX_LINKS];

    /* ---- 转发用 worker（都挂在默认 producer 的事件线程上，vpn_close_tun 里销毁）----
     *   tun_worker ：tun fd 可读 -> 出站转发（io_worker，回调里只读一个包）
     *   tick_worker：约每 VPN_FORWARD_TICK_MS 一次兜底 tick（timer_worker）
     * 二者与 tun 同生共死（见 vpn_open_tun / vpn_close_tun：tun 起来就挂上，
     * 关 tun 先撤 worker）；用 void* 承载，避免这个模块内部头反过来依赖
     * concurrent 的 Worker 类型。 */
    void *tun_worker;
    void *tick_worker;
} vpn_ctx_t;

/* ---- 控制帧处理：表驱动分派（新增帧类型只需加一行表项，不必改分派逻辑）----
 * 处理函数签名统一为 (ctx, link, 帧, 报文总长)，返回 0=正常 / 负值=异常。
 * 注意：全部运行在 p2p 事件线程，禁止阻塞。 */
typedef int (*vpn_ctrl_handler_t)(vpn_ctx_t *ctx, vpn_link_t *link,
                                  const vpn_ctrl_t *f, int len);

typedef struct vpn_ctrl_entry_s {
    uint8_t            type;    /* 帧类型 */
    const char        *name;    /* 名称，只用于日志/排障 */
    vpn_ctrl_handler_t handle;  /* 处理函数 */
} vpn_ctrl_entry_t;

#endif /* __VPN_INTERNAL_H__ */
