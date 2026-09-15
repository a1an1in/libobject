/**
 * @file Vpn.c
 * @Synopsis  VPN 对外入口实现：p2p 会话 <-> Tun 双向转发（Linux TUN，L3 点对点）
 *
 * 流程（见 doc/net/vpn/p2p_vpn_design.md §5）：
 *   解析 vpn_cfg -> 打开并配置 Tun -> p2p_node_create 建链
 *     -> 主叫 p2p_session_create（异步 CALL）/ 被叫等 INVITE 自动建会话
 *     -> on_ready -> 转发循环：
 *          出站：tun->read 出 IP 包 -> p2p_session_send（直接透传，不加隧道头）
 *          入站：p2p recv 回调（p2p 事件线程）-> tun->write 注入本机协议栈
 *     -> Ctrl+C：close 会话与 Tun
 *
 * 首版取舍（§6）：点对点单通道，故出站不区分对端、无隧道头；被叫从
 * recv 回调"学到"对端会话句柄，从而也能出站回发。
 *
 * @author Zoo
 * @date 2026-09-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/net/p2p/p2p.h>
#include <libobject/net/vpn/Vpn.h>
#include "tun/Tun.h"

/* 出站读缓冲：需 >= 单包上限(MTU 1400 + IP 头余量)，取 4K 足够。 */
#define VPN_RW_BUF_SIZE 4096
/* 等待打通的最长轮次（主叫）：300 * 200ms = 60s；被叫常驻不超时。 */
#define VPN_WAIT_ROUNDS 300

/* ---- VPN 层控制帧（走 p2p 数据通道，与业务 IP 包区分）----
 * 首字节 0xFF 在合法 IP 包里不可能出现（版本号必须是 4/6），所以零误判。 */
#define VPN_CTRL_MAGIC     "\xFFVPN"
#define VPN_CTRL_MAGIC_LEN 4
#define VPN_CTRL_HDR_LEN   8
#define VPN_CTRL_HELLO     1            /* 通告本端内网网段（payload 为 CIDR 文本） */
#define VPN_CTRL_CIDR_MAX  64

typedef struct vpn_ctrl_s {
    uint8_t  magic[VPN_CTRL_MAGIC_LEN];  /* 0xFF 'V' 'P' 'N' */
    uint8_t  type;                       /* VPN_CTRL_HELLO */
    uint8_t  rsvd;
    uint16_t len;                        /* payload 字节数（同构实现，主机序） */
    char     payload[VPN_CTRL_CIDR_MAX]; /* type=HELLO：如 "172.16.10.0/23"（无 '\0'） */
} vpn_ctrl_t;

typedef struct vpn_ctx_s {
    const vpn_cfg_t *cfg;
    Tun *tun;
    p2p_node_t *node;
    p2p_session_t *session;   /* 主叫：create 所得；被叫：recv 回调学到 */
    int dial;                 /* 1=主叫 */
    int hello_sent;           /* 已通告过本端 local_net */
    int peer_cidr_seen;       /* 已收到并安装对端通告的网段 */
} vpn_ctx_t;

/* 校验对端通告的网段：合法 a.b.c.d/len（len∈[1,32]），且不是 0.0.0.0/x。 */
static int __cidr_valid(const char *s)
{
    unsigned a, b, c, d, len;
    char tail;

    if (s == NULL || s[0] == '\0' || strlen(s) >= VPN_CTRL_CIDR_MAX) {
        return -1;
    }
    if (sscanf(s, "%u.%u.%u.%u/%u%c", &a, &b, &c, &d, &len, &tail) != 5) {
        return -1;
    }
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return -1;
    }
    if (len < 1 || len > 32 || (a == 0 && b == 0 && c == 0 && d == 0)) {
        return -1;
    }
    return 0;
}

/* 把本端内网网段通告给对端（要有 session；多发几次抗丢包）。
 * 可能运行在 p2p 事件线程里，所以不阻塞、不 sleep。 */
static int __vpn_send_hello(vpn_ctx_t *ctx, int times)
{
    vpn_ctrl_t f;
    int i, n;

    if (ctx == NULL || ctx->session == NULL || ctx->cfg->local_net == NULL ||
        ctx->cfg->local_net[0] == '\0') {
        return 0;                       /* 没配 --local-net 就不通告 */
    }
    if (__cidr_valid(ctx->cfg->local_net) < 0) {
        dbg_str(DBG_ERROR, "vpn: --local-net 非法(%s)，忽略不通告",
                ctx->cfg->local_net);
        return -1;
    }
    memset(&f, 0, sizeof(f));
    memcpy(f.magic, VPN_CTRL_MAGIC, VPN_CTRL_MAGIC_LEN);
    f.type = VPN_CTRL_HELLO;
    f.len  = (uint16_t)strlen(ctx->cfg->local_net);
    memcpy(f.payload, ctx->cfg->local_net, f.len);
    n = VPN_CTRL_HDR_LEN + f.len;
    for (i = 0; i < times; i++) {
        p2p_session_send(ctx->session, (const uint8_t *)&f, n);
    }
    if (!ctx->hello_sent) {
        dbg_str(DBG_VIP, "vpn: advertised local net %s to peer",
                ctx->cfg->local_net);
        ctx->hello_sent = 1;
    }
    return 0;
}

/* 处理对端控制帧：目前只有 HELLO（对端通告它的内网网段 -> 本机据此加路由）。 */
static int __vpn_handle_ctrl(vpn_ctx_t *ctx, const uint8_t *data, int len)
{
    const vpn_ctrl_t *f = (const vpn_ctrl_t *)data;
    char cidr[VPN_CTRL_CIDR_MAX];
    int n;

    if (len < VPN_CTRL_HDR_LEN) {
        return -1;
    }
    if (f->type != VPN_CTRL_HELLO) {
        return 0;
    }
    n = f->len;
    if (n <= 0 || n >= VPN_CTRL_CIDR_MAX || (int)VPN_CTRL_HDR_LEN + n > len) {
        return -1;
    }
    memcpy(cidr, f->payload, n);
    cidr[n] = '\0';

    if (__cidr_valid(cidr) < 0) {
        dbg_str(DBG_ERROR, "vpn: peer advertised invalid cidr '%s', ignored", cidr);
        return -1;
    }
    dbg_str(DBG_VIP, "vpn: peer advertised local net %s, adding route", cidr);
    ctx->tun->route_add(ctx->tun, cidr);
    ctx->peer_cidr_seen = 1;
    /* 我们自己的通告可能丢了（或本端是被叫、刚学到 session）：顺手回发一次 */
    if (!ctx->hello_sent) {
        __vpn_send_hello(ctx, 3);
    }
    return 0;
}

/* 入站：控制帧在 VPN 层消化；业务 IP 包注入本机 Tun。
 * 运行于 p2p 事件线程，回调内不得阻塞。 */
static int __vpn_recv(void *opaque, p2p_session_t *session,
                      const uint8_t *data, int len)
{
    vpn_ctx_t *ctx = (vpn_ctx_t *)opaque;

    if (ctx == NULL || ctx->tun == NULL || data == NULL || len <= 0) {
        return -1;
    }
    if (ctx->session == NULL) {
        /* 被叫：收到对端数据时才拿到自动建立的会话句柄，记下供出站回发。 */
        ctx->session = session;
        dbg_str(DBG_INFO, "vpn: learned peer session (callee path)");
        __vpn_send_hello(ctx, 3);      /* 有 session 了，通告自己的网段 */
    }
    /* 控制帧判别：首字节 0xFF 不可能是合法 IP 包（IPv4/IPv6 版本号是 4/6） */
    if (len >= VPN_CTRL_HDR_LEN && (uint8_t)data[0] == 0xFF &&
        memcmp(data, VPN_CTRL_MAGIC, VPN_CTRL_MAGIC_LEN) == 0) {
        return __vpn_handle_ctrl(ctx, data, len);
    }
    return ctx->tun->write(ctx->tun, data, len);
}

static int __vpn_open_tun(vpn_ctx_t *ctx)
{
    const vpn_cfg_t *cfg = ctx->cfg;

    ctx->tun = tun_create();
    if (ctx->tun == NULL) {
        dbg_str(DBG_ERROR, "vpn: tun_create failed");
        return -1;
    }
    if (ctx->tun->open(ctx->tun, cfg->tun_name) < 0) {
        dbg_str(DBG_ERROR, "vpn: tun open failed (need root/CAP_NET_ADMIN?)");
        return -1;
    }
    ctx->tun->set_mtu(ctx->tun, ctx->tun->mtu);
    if (ctx->tun->configure(ctx->tun, cfg->tunnel_ip, cfg->netmask,
                            cfg->remote_net) < 0) {
        dbg_str(DBG_ERROR, "vpn: tun configure failed (need root/CAP_NET_ADMIN?)");
        return -1;
    }
    dbg_str(DBG_VIP, "vpn: tun %s up, tunnel-ip=%s netmask=%s route=%s mtu=%d",
            ctx->tun->name, cfg->tunnel_ip,
            (cfg->netmask != NULL) ? cfg->netmask : "24",
            (cfg->remote_net != NULL) ? cfg->remote_net : "-",
            ctx->tun->mtu);
    return 0;
}

static int __vpn_open_p2p(vpn_ctx_t *ctx)
{
    const vpn_cfg_t *cfg = ctx->cfg;
    p2p_cfg_t p2p_cfg;

    memset(&p2p_cfg, 0, sizeof(p2p_cfg));
    p2p_cfg.stun_id        = cfg->id;
    p2p_cfg.local_service  = cfg->local_service;
    p2p_cfg.signal_host    = cfg->signal_host;
    p2p_cfg.signal_service = cfg->signal_service;
    p2p_cfg.stun_host      = cfg->stun_host;
    p2p_cfg.stun_service   = cfg->stun_service;
    p2p_cfg.stun2_host     = cfg->stun2_host;
    p2p_cfg.stun2_service  = cfg->stun2_service;
    p2p_cfg.interval_ms    = (cfg->interval_ms > 0) ? cfg->interval_ms : 200;

    if (p2p_node_create(&ctx->node, __vpn_recv, &p2p_cfg, ctx) != 0) {
        dbg_str(DBG_ERROR, "vpn: %s p2p node online failed", cfg->id);
        return -1;
    }
    dbg_str(DBG_INFO, "vpn: %s online%s", cfg->id,
            ctx->dial ? ", calling peer" : ", waiting to be called");

    if (ctx->dial) {
        if (p2p_session_create(ctx->node, cfg->peer_id, &ctx->session) < 0) {
            dbg_str(DBG_ERROR, "vpn: %s create session to %s failed",
                    cfg->id, cfg->peer_id);
            return -1;
        }
    }
    return 0;
}

/* 等隧道打通：主叫等 CONNECTED（超时失败）；被叫等首个入站数据（常驻不超时）。 */
static int __vpn_wait_link(vpn_ctx_t *ctx)
{
    struct event_base *eb = event_base_get_default_instance();
    int i;

    for (i = 0; ; i++) {
        if (eb != NULL && eb->eb != NULL && eb->eb->break_flag) {
            return -1;   /* Ctrl+C */
        }
        if (ctx->dial) {
            if (ctx->session != NULL &&
                p2p_session_is_connected(ctx->session) == 0) {
                dbg_str(DBG_VIP, "vpn: %s <-> %s connected (server-confirmed)",
                        ctx->cfg->id, ctx->cfg->peer_id);
                return 0;
            }
            if (i >= VPN_WAIT_ROUNDS) {
                dbg_str(DBG_ERROR, "vpn: connect timeout (%ds)",
                        VPN_WAIT_ROUNDS / 5);
                return -1;
            }
        } else {
            if (ctx->session != NULL) {
                dbg_str(DBG_VIP, "vpn: %s got peer session (callee)", ctx->cfg->id);
                return 0;
            }
        }
        usleep(200000);
    }
}

/* 转发循环（出站）：poll 带超时以便响应 Ctrl+C；入站走 __vpn_recv 回调，无额外线程。 */
static void __vpn_forward(vpn_ctx_t *ctx)
{
    struct event_base *eb = event_base_get_default_instance();
    uint8_t buf[VPN_RW_BUF_SIZE];

    for (;;) {
        struct pollfd pfd;
        int n;

        if (eb != NULL && eb->eb != NULL && eb->eb->break_flag) {
            break;
        }
        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = ctx->tun->fd;
        pfd.events = POLLIN;
        n = poll(&pfd, 1, 200);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            dbg_str(DBG_ERROR, "vpn: poll failed: %s", strerror(errno));
            break;
        }
        if (n == 0) {
            continue;   /* 超时：回到循环顶检查退出标志 */
        }
        n = ctx->tun->read(ctx->tun, buf, sizeof(buf));
        if (n <= 0) {
            continue;
        }
        if (ctx->session == NULL ||
            p2p_session_is_connected(ctx->session) != 0) {
            continue;   /* 还没打通：丢弃出站包（等建链完成） */
        }
        if (p2p_session_send(ctx->session, buf, n) < 0) {
            dbg_str(DBG_INFO, "vpn: send %d bytes failed", n);
        }
    }
}

int vpn_run(const vpn_cfg_t *cfg)
{
    vpn_ctx_t ctx;
    int ret = -1;

    if (cfg == NULL || cfg->id == NULL || cfg->tunnel_ip == NULL ||
        cfg->signal_host == NULL || cfg->signal_service == NULL) {
        dbg_str(DBG_ERROR, "vpn: bad cfg (id/tunnel_ip/signal_* required)");
        return -1;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.cfg = cfg;
    ctx.dial = (cfg->peer_id != NULL);

    if (__vpn_open_tun(&ctx) < 0) {
        goto out;
    }
    if (__vpn_open_p2p(&ctx) < 0) {
        goto out;
    }
    if (__vpn_wait_link(&ctx) < 0) {
        goto out;
    }

    __vpn_send_hello(&ctx, 3);      /* 链路通了：把本端内网网段通告给对端 */

    if (cfg->on_ready != NULL) {
        cfg->on_ready(cfg->opaque);
    }
    dbg_str(DBG_VIP, "vpn: tunnel up (%s, ip=%s), Ctrl+C to stop",
            ctx.tun->name, cfg->tunnel_ip);
    __vpn_forward(&ctx);
    dbg_str(DBG_VIP, "vpn: stopped");
    ret = 0;

out:
    if (ret < 0 && cfg->on_error != NULL) {
        cfg->on_error(cfg->opaque, ret);
    }
    if (ctx.session != NULL) {
        p2p_session_close(ctx.session);
    }
    if (ctx.node != NULL) {
        p2p_node_close(ctx.node);
    }
    if (ctx.tun != NULL) {
        tun_destroy(ctx.tun);
    }
    return ret;
}
