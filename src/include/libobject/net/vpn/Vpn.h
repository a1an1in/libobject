#ifndef __VPN_H__
#define __VPN_H__

#include <stdint.h>

/*
 * VPN 模块（对外接口，net/vpn）——基于 p2p 通道的站点间网段互访。
 *
 * 定位（见 doc/net/vpn/p2p_vpn_design.md）：
 *  - src/net/vpn 与 src/net/p2p 平级并列，vpn -> p2p 单向依赖（vpn 只用 p2p 公共头）；
 *  - 首版：Linux TUN、L3 路由点对点、配置地址/路由走外部 ip 命令、
 *    不加隧道头、不加加密，保活由 p2p 会话内部维持；
 *  - 数据面：Tun 读到的 IP 包直接 p2p_session_send（点对点单通道无需隧道头）；
 *    对端包经 p2p recv 回调写入 Tun 注入本机协议栈。
 *
 * 使用门槛：Linux 下打开/配置 TUN 需要 root 或 CAP_NET_ADMIN。
 */
/*
 * 默认值（xtools vpn 与 tests/net/test_vpn.c 共用同一份，避免"测试与 CLI 默认值不一致"）。
 * 尤其 STUN：默认必须指向**公网可达**的采址服务器——若改指向"与被叫同网段的信令服"，
 * 被叫会采到内网地址，对端拿它打洞必然失败（表现为两端迟迟不 CONNECTED）。
 */
#define VPN_DEFAULT_STUN_HOST  "stun.cloudflare.com"
#define VPN_DEFAULT_STUN_PORT  "3478"
#define VPN_DEFAULT_STUN2_HOST "stun.l.google.com"
#define VPN_DEFAULT_STUN2_PORT "19302"
#define VPN_DEFAULT_INTERVAL_MS 200

typedef struct vpn_cfg_s {
    /* ---- p2p 通道（语义同 p2p_cfg_t） ---- */
    const char *id;             /* 本端 stun id（SIGNIN 上报，被寻址用，必填） */
    const char *peer_id;        /* 对端 stun id：非空=主叫(主动建链)；空=被叫(常驻等被叫) */
    const char *local_service;  /* 会话 data 口本地端口，可空(随机)；多会话/防冲突用 */
    const char *signal_host;    /* 信令服务器 host（必填） */
    const char *signal_service; /* 信令服务器端口（必填） */
    const char *stun_host;      /* 主 STUN（采址）；空=用信令服务器自身 */
    const char *stun_service;
    const char *stun2_host;     /* 第二 STUN（nat 对称探测）；空=不探测 */
    const char *stun2_service;
    int interval_ms;            /* 打洞/保活周期，<=0 用 p2p 默认(200) */

    /* ---- 虚拟网卡 / 网段 ---- */
    const char *tun_name;       /* 设备名，可空(自动 tunN) */
    const char *tunnel_ip;      /* 本端隧道地址（tun 网卡地址，必填；可带 "/len" 前缀） */
    const char *netmask;        /* 掩码：点分("255.255.255.0")或前缀("24")；空=24 */

    /* 本端内网网段（写成"网络地址/前缀长度"，如 "172.16.10.0/23"）：**只填自己的**，
     * 链路建立后会自动通告给对端，对端据此自动 `ip route replace <它> dev tun`。可空=不通告。
     * 与下面 remote_net 的区别：这是"我是谁"，那是"我要去哪（对端的网段，需手动填）"。 */
    const char *local_net;

    /* 静态对端网段路由（如 "10.10.10.0/24"）：**程序化使用**的逃生口，
     * 启动时直接 `ip route replace <它> dev tun`。
     * 注意：`xtools vpn` 命令行已不暴露它——两端各填 local_net 自动交换即可。可空。 */
    const char *remote_net;

    /* ---- 事件 ---- */
    void (*on_ready)(void *opaque);          /* 隧道打通、可转发时回调(可空) */
    void (*on_error)(void *opaque, int ret); /* 建链/Tun 失败时回调(可空) */
    void *opaque;
} vpn_cfg_t;

/*
 * 运行 VPN：打开并配置 Tun -> p2p 节点上线 -> [主叫建会话] -> 等打通 ->
 * 双向转发（出站 tun read -> session send；入站 recv 回调 -> tun write），
 * 阻塞直到 Ctrl+C(SIGINT) 或出错。
 *
 * 返回：0=正常停止；负值=失败。
 */
int vpn_run(const vpn_cfg_t *cfg);

#endif
