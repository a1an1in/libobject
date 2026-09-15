/**
 * @file Vpn.c
 * @Synopsis  VPN 对外入口实现：p2p 会话 <-> Tun 双向转发（Linux TUN，L3 路由）
 *
 * 流程（见 doc/net/vpn/p2p_vpn_design.md §5）：
 *   解析 vpn_cfg -> 打开 Tun（只 open + MTU）-> p2p 建链
 *     （主动方 p2p_session_create / 被动方等 INVITE 自动建会话）-> 等 CONNECTED
 *     -> **交换地址**（主动方发 NET_NOTIFY；被动方在应答里分配对端地址 + 带回本端地址）
 *     -> 一次性创建并配置 Tun（本端地址 + 各对端网段路由）-> on_ready -> 转发循环：
 *          出站：tun->read 出 IP 包 -> 按目的地址选 link -> p2p_session_send（透传，不加隧道头）
 *          入站：p2p recv 回调（p2p 事件线程）-> tun->write 注入本机协议栈
 *     -> Ctrl+C：回收全部链路（关会话 + 归还地址）与 Tun
 *
 * 多对端（一个 tun 复用给多条链路）：被动方为**每个对端**单独登记一条 link
 * （recv 回调带 session 参数即可区分，无需身份字段），并充当**地址分配者**
 * （池 = 自己的 --tunnel-ip 所在网段，占用表管理、断链即归还）；
 * 出站按目的地址（对端隧道地址精确 / 对端内网网段最长前缀）选 link 发送——
 * 于是路由层只需"把包引进 tun"，"发给哪个对端"由本层决定。
 *
 * 常量/协议/结构体等定义集中在 Vpn_Internal.h。
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
#include "Vpn_Internal.h"    /* 模块私有定义（常量/协议/结构体/结果码）都在这里 */

/* 常量、协议（vpn_ctrl_t）、结构体（vpn_net_t / vpn_link_t / vpn_ctx_t）、
 * 结果码与分派类型，全部集中在 Vpn_Internal.h。 */

/* 解析 IPv4 文本 -> 主机序地址；成功返回 0，失败返回 -1。
 * **容忍可选的 "/len" 后缀**（控制帧里的地址字段常写成 "10.0.0.2/24"，
 * 这里只取地址，前缀长度由调用方按各自语义决定）。
 * 例："10.0.0.2" / "10.0.0.2/24" 都 -> 10.0.0.2；"10.0.0.2x" 失败。 */
static int __ipv4_parse(const char *s, uint32_t *out)
{
    unsigned a, b, c, d;
    char tail;
    int got;

    if (s == NULL || out == NULL) {
        return -1;
    }
    got = sscanf(s, "%u.%u.%u.%u%c", &a, &b, &c, &d, &tail);
    if (got != 4 && !(got == 5 && tail == '/')) {
        return -1;
    }
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return -1;
    }
    *out = (a << 24) | (b << 16) | (c << 8) | d;
    return 0;
}

/* 取 IPv4 报文的目的地址（主机序）；非 IPv4/长度不够返回 0。 */
static uint32_t __ipv4_dst(const uint8_t *p, int len)
{
    if (p == NULL || len < 20 || (p[0] >> 4) != 4) {
        return 0;
    }
    return ((uint32_t)p[16] << 24) | ((uint32_t)p[17] << 16) |
           ((uint32_t)p[18] << 8) | (uint32_t)p[19];
}

/* 严格解析 "a.b.c.d/len"（len∈[1,32]；拒绝 0.0.0.0/x 与 /0 默认路由）。
 * 成功返回 0，并算好网络地址/掩码（主机序）。 */
static int __net_parse(const char *s, vpn_net_t *out)
{
    unsigned a, b, c, d, len;
    char tail;

    if (s == NULL || out == NULL || s[0] == '\0' ||
        strlen(s) >= VPN_CTRL_CIDR_MAX) {
        return -1;
    }
    if (sscanf(s, "%u.%u.%u.%u/%u%c", &a, &b, &c, &d, &len, &tail) != 5) {
        return -1;
    }
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return -1;
    }
    if (len < 1 || len > 32) {
        return -1;
    }
    out->prefix = (int)len;
    out->mask   = (len == 32) ? 0xFFFFFFFFu : (0xFFFFFFFFu << (32 - len));
    out->net    = ((a << 24) | (b << 16) | (c << 8) | d) & out->mask;
    if (out->net == 0) {
        return -1;                       /* 0.0.0.0/x */
    }
    return 0;
}

/* ip 是否落在 net 内。 */
static int __net_contains(const vpn_net_t *net, uint32_t ip)
{
    return (ip & net->mask) == net->net;
}

/* 网段 -> 文本（日志与"规范化后放控制帧"两用，长度 ≤ 18）。 */
static void __net_str(const vpn_net_t *n, char *buf, int len)
{
    snprintf(buf, len, "%u.%u.%u.%u/%d",
             (n->net >> 24) & 0xFF, (n->net >> 16) & 0xFF,
             (n->net >> 8) & 0xFF, n->net & 0xFF, n->prefix);
}

/* 地址 -> "a.b.c.d/len" 文本（长度可控，用于控制帧 payload）。 */
static void __ipv4_str(uint32_t ip, int prefix, char *buf, int bufsz)
{
    snprintf(buf, bufsz, "%u.%u.%u.%u/%d",
             (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
             prefix);
}

/* 求掩码的**长度**（即 /N 的 N，1~32）：两种写法都接受——
 *   点分("255.255.255.0" -> 24)：数左侧连续 1 的个数；
 *   数字("24" -> 24)：已经是长度，直接用。
 * 空/非法按 24 处理。
 * 注：Tun.c 里有一份等价实现（它的 configure() 也要这个长度），
 * 两者分属不同编译单元的静态工具；vpn -> tun 是单向依赖，故不强行合并。 */
static int __netmask_len(const char *netmask)
{
    unsigned a, b, c, d;
    uint32_t m;
    int n, prefix = 0;

    if (netmask == NULL || netmask[0] == '\0') {
        return 24;
    }
    if (strchr(netmask, '.') == NULL) {
        n = atoi(netmask);
        return (n > 0 && n <= 32) ? n : 24;
    }
    if (sscanf(netmask, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 ||
        a > 255 || b > 255 || c > 255 || d > 255) {
        return 24;
    }
    m = (a << 24) | (b << 16) | (c << 8) | d;
    for (prefix = 0; m & 0x80000000u; m <<= 1) {
        prefix++;
    }
    return prefix;
}

/* 取/建某会话对应的 link：
 *   主动建链方（dialer）在建会话时就登记；被动方在**收到该对端第一个包**时
 *   按 session 参数登记（p2p 回调带 session，天然能区分是谁），
 *   于是被动方（hub）可同时持有多个对端。
 * is_dialer：该链路是否由本端主动建（决定"谁发 NET_NOTIFY"）。
 * 空槽（session==NULL，如对端断链被回收后）会被**复用**，所以 n_links 是
 * "在线链路数"而非数组长度——遍历请用 VPN_MAX_LINKS 并跳过空槽。 */
static vpn_link_t *__vpn_link_get(vpn_ctx_t *ctx, p2p_session_t *session,
                                  int is_dialer)
{
    int i, slot = -1;

    for (i = 0; i < VPN_MAX_LINKS; i++) {
        if (ctx->links[i].session == session) {
            return &ctx->links[i];
        }
        if (slot < 0 && ctx->links[i].session == NULL) {
            slot = i;
        }
    }
    if (session == NULL || slot < 0) {
        dbg_str(DBG_ERROR, "vpn: 对端数已达上限 %d，忽略新会话", VPN_MAX_LINKS);
        return NULL;
    }
    memset(&ctx->links[slot], 0, sizeof(vpn_link_t));
    ctx->links[slot].session   = session;
    ctx->links[slot].is_dialer = is_dialer;
    ctx->n_links++;
    dbg_str(DBG_VIP, "vpn: 新增对端链路 slot=%d（在线 %d 条，%s）",
            slot, ctx->n_links, is_dialer ? "主动" : "被动");
    return &ctx->links[slot];
}

/* 出站选 link：按目的地址
 *   1) 对端隧道地址，精确命中 -> 该 link；
 *   2) 对端内网网段，最长前缀匹配 -> 该 link；
 *   3) 都没命中且只有一条 link -> 用它（点对点/主叫场景的兼容退化）；
 *   4) 多 link 且不命中 -> NULL（丢弃而不猜，避免把包串给别的对端）。 */
static vpn_link_t *__vpn_link_pick(vpn_ctx_t *ctx, uint32_t dst)
{
    vpn_link_t *best = NULL, *only = NULL;
    int i, n_online = 0, best_prefix = -1;

    if (dst == 0) {
        return NULL;
    }
    for (i = 0; i < VPN_MAX_LINKS; i++) {
        vpn_link_t *l = &ctx->links[i];

        if (l->session == NULL) {
            continue;                    /* 空槽（已回收）：跳过 */
        }
        n_online++;
        only = l;
        if (l->peer_tun_ip != 0 && l->peer_tun_ip == dst) {
            return l;
        }
        if (l->peer_net_ok && __net_contains(&l->peer_net, dst) &&
            l->peer_net.prefix > best_prefix) {
            best = l;
            best_prefix = l->peer_net.prefix;
        }
    }
    if (best != NULL) {
        return best;
    }
    if (n_online == 1) {
        return only;                     /* 只有一条链路：退化为直发（兼容） */
    }
    return NULL;
}

/* 发一个控制帧（可能运行在 p2p 事件线程里，所以不阻塞、不 sleep）：
 *   NET_NOTIFY     payload = 本端内网网段（可空）——只有主动方发一次，见 __vpn_exchange_addr
 *   NET_NOTIFY_ACK payload = 见 __vpn_reply_notify()
 * 只负责"发一帧"；发送时机见 __vpn_exchange_addr 与转发循环的兜底。 */
static int __vpn_send_ctrl(vpn_link_t *link, uint8_t type,
                           const char *payload, int payload_len)
{
    vpn_ctrl_t f;
    int n = VPN_CTRL_HDR_LEN;

    if (link == NULL || link->session == NULL) {
        return -1;
    }
    if (payload_len < 0 || payload_len >= VPN_CTRL_CIDR_MAX) {
        return -1;
    }
    memset(&f, 0, sizeof(f));
    memcpy(f.magic, VPN_CTRL_MAGIC, VPN_CTRL_MAGIC_LEN);
    f.type = type;
    if (payload != NULL && payload_len > 0) {
        f.len = (uint16_t)payload_len;
        memcpy(f.payload, payload, payload_len);
        n += payload_len;
    }
    return p2p_session_send(link->session, (const uint8_t *)&f, n);
}

/* 控制帧分派类型（vpn_ctrl_handler_t / vpn_ctrl_entry_t）见 Vpn_Internal.h；
 * 下面只放分派表实例 g_vpn_ctrl_table 与各处理函数。 */

/* 结果码 enum（VPN_NOTIFY_*）见 Vpn_Internal.h。 */

/* 记录对端的隧道地址 / 内网网段（都用于出站选路与装路由）；ip / net 均可为 NULL。
 * **只记录、不碰 tun**：配置类动作统一在 __vpn_configure_tun / __vpn_install_link_routes——
 * 本函数可能跑在 p2p 事件线程，而且地址交换发生在建 tun 之前。
 * 返回 VPN_NOTIFY_OK 或拒绝码（调用方据此决定应答里的码）。 */
static int __vpn_link_set_peer_info(vpn_ctx_t *ctx, vpn_link_t *link,
                                    const char *ip, const char *net)
{
    vpn_net_t pnet;

    (void)ctx;
    if (ip != NULL && ip[0] != '\0') {
        uint32_t tun_ip;

        if (__ipv4_parse(ip, &tun_ip) < 0) {
            return VPN_NOTIFY_EBADIP;
        }
        if (link->peer_tun_ip != tun_ip) {
            link->peer_tun_ip = tun_ip;
            dbg_str(DBG_VIP, "vpn: 对端隧道地址 = %s", ip);
        }
    }
    if (net == NULL || net[0] == '\0') {
        return VPN_NOTIFY_OK;            /* 对端没配内网网段：只记隧道地址 */
    }
    if (__net_parse(net, &pnet) < 0) {
        return VPN_NOTIFY_EBADNET;
    }
    if (link->peer_net_text[0] != '\0' && strcmp(link->peer_net_text, net) == 0) {
        return VPN_NOTIFY_OK;            /* 同一网段：去重 */
    }
    snprintf(link->peer_net_text, sizeof(link->peer_net_text), "%s", net);
    link->peer_net    = pnet;
    link->peer_net_ok = 1;
    link->route_done  = 0;               /* 交给"装路由"的时机去装 */
    return VPN_NOTIFY_OK;
}

/* 地址池初始化（**被动方 = 分配者**）：网段取自 --tunnel-ip（本端地址也就是它）。
 * 没配 --tunnel-ip 或非法 -> 无法当分配者（vpn_run 会据此报错）。 */
static void __vpn_pool_init(vpn_ctx_t *ctx)
{
    const vpn_cfg_t *cfg = ctx->cfg;
    char text[64];
    vpn_net_t seg;

    ctx->pool_ok = 0;
    if (cfg->tunnel_ip == NULL || cfg->tunnel_ip[0] == '\0') {
        return;
    }
    if (strchr(cfg->tunnel_ip, '/') != NULL) {
        snprintf(text, sizeof(text), "%s", cfg->tunnel_ip);
    } else {
        snprintf(text, sizeof(text), "%s/%d", cfg->tunnel_ip,
                 __netmask_len(cfg->netmask));
    }
    if (__net_parse(text, &seg) < 0) {
        dbg_str(DBG_ERROR, "vpn: --tunnel-ip 非法(%s)，无法作为地址池", text);
        return;
    }
    if (__ipv4_parse(cfg->tunnel_ip, &ctx->my_tun_ip) < 0) {
        ctx->my_tun_ip = 0;           /* 上面已用 __net_parse 校验过，这里只是兜底 */
    }
    ctx->pool_net    = seg.net;
    ctx->pool_mask   = seg.mask;
    ctx->pool_prefix = seg.prefix;
    ctx->pool_ok     = 1;
    dbg_str(DBG_INFO, "vpn: 地址池 %u.%u.%u.%u/%d（本端 %s，对端按下标分 .1/.2/...）",
            (seg.net >> 24) & 0xFF, (seg.net >> 16) & 0xFF,
            (seg.net >> 8) & 0xFF, seg.net & 0xFF, seg.prefix, cfg->tunnel_ip);
}

/* 地址分配：从**占用表**里取第一个空闲地址（= pool_net|(i+1)，跳过本端地址）。
 * 占用表与 link 槽位无关，所以对端断链归还的地址可以被后来者复用。 */
static uint32_t __vpn_pool_alloc(vpn_ctx_t *ctx)
{
    int i;

    if (!ctx->pool_ok) {
        dbg_str(DBG_ERROR, "vpn: 未配置地址池（--tunnel-ip 非法或未填）");
        return 0;
    }
    for (i = 0; i < VPN_MAX_LINKS; i++) {
        uint32_t ip = ctx->pool_net | (uint32_t)(i + 1);

        if (ip == ctx->my_tun_ip || ctx->addr_used[i] != 0) {
            continue;
        }
        ctx->addr_used[i] = ip;
        dbg_str(DBG_INFO, "vpn: 分配隧道地址 %u.%u.%u.%u 给新对端",
                (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        return ip;
    }
    dbg_str(DBG_ERROR, "vpn: 地址池已满（最多 %d 个对端）", VPN_MAX_LINKS);
    return 0;
}

/* 归还地址（对端断链/链路释放时调用）：对应槽置空闲，供后来的对端复用。 */
static void __vpn_pool_free(vpn_ctx_t *ctx, uint32_t ip)
{
    int i;

    if (!ctx->pool_ok || ip == 0) {
        return;
    }
    for (i = 0; i < VPN_MAX_LINKS; i++) {
        if (ctx->addr_used[i] == ip) {
            ctx->addr_used[i] = 0;
            return;
        }
    }
}

/* 取第一条"主动建链且在线"的链路（没有则 NULL）。 */
static vpn_link_t *__vpn_link_dialer(vpn_ctx_t *ctx)
{
    int i;

    for (i = 0; i < VPN_MAX_LINKS; i++) {
        if (ctx->links[i].session != NULL && ctx->links[i].is_dialer) {
            return &ctx->links[i];
        }
    }
    return NULL;
}

/* 释放一条链路：关会话 + **归还隧道地址** + 清空槽（可被后续对端复用）。 */
static void __vpn_link_release(vpn_ctx_t *ctx, vpn_link_t *link)
{
    if (link == NULL || link->session == NULL) {
        return;
    }
    p2p_session_close(link->session);
    __vpn_pool_free(ctx, link->peer_tun_ip);
    dbg_str(DBG_INFO, "vpn: 释放链路（归还隧道地址 %u.%u.%u.%u）",
            (link->peer_tun_ip >> 24) & 0xFF, (link->peer_tun_ip >> 16) & 0xFF,
            (link->peer_tun_ip >> 8) & 0xFF, link->peer_tun_ip & 0xFF);
    memset(link, 0, sizeof(*link));
    if (ctx->n_links > 0) {
        ctx->n_links--;
    }
}

/* 释放**被动侧**已判定断链的链路（转发循环的空闲 tick 里调用，约 1s 一次）：
 * 遍历链路，连续 VPN_LINK_BAD_ROUNDS 轮 `p2p_session_is_connected()` 都不可用
 * 才调 __vpn_link_release()（关会话 + 归还隧道地址 + 空出槽位），避免瞬时抖动误判。
 *
 * **只回收被动链路**（is_dialer==0）：
 *   - 被动方分配出去的隧道地址与 link 槽位都是有限资源（hub 会被陆续连接），必须归还；
 *   - 主动方只有自己建的那一条：不涉及地址池，`p2p_session_close()` 之后也不会再
 *     create（vpn_run 只在启动时建一次），回收等于**永久断线**，还丢掉 p2p 保活
 *     自愈的机会——所以主动链路即使暂时不可用也保留，出站包由 is_connected 检查丢弃。 */
static void __vpn_release_dead_links(vpn_ctx_t *ctx)
{
    int i;

    for (i = 0; i < VPN_MAX_LINKS; i++) {
        vpn_link_t *l = &ctx->links[i];

        if (l->session == NULL || l->is_dialer) {
            continue;
        }
        if (p2p_session_is_connected(l->session) == 0) {
            l->bad_rounds = 0;
            continue;
        }
        if (++l->bad_rounds >= VPN_LINK_BAD_ROUNDS) {
            dbg_str(DBG_INFO, "vpn: 链路连续 %d 轮不可用，判定断链并回收",
                    l->bad_rounds);
            __vpn_link_release(ctx, l);
        }
    }
}

/* 给"已记下对端网段但还没装路由"的链路装路由（幂等）：
 * 地址信息在交换/收包阶段只记录；这里统一装——配 tun 后调用一次，
 * 转发循环的 tick 里再兜底（覆盖配置之后才到来的对端）。 */
static void __vpn_install_link_routes(vpn_ctx_t *ctx)
{
    int i;

    if (ctx->tun == NULL) {
        return;
    }
    for (i = 0; i < VPN_MAX_LINKS; i++) {
        vpn_link_t *l = &ctx->links[i];

        if (l->peer_net_ok && !l->route_done) {
            l->route_done = 1;
            dbg_str(DBG_VIP, "vpn: 对端内网网段 %s，加路由", l->peer_net_text);
            ctx->tun->route_add(ctx->tun, l->peer_net_text);
        }
    }
}

/* 回 NET_NOTIFY_ACK（唯一的应答出口）：
 *   码 0：从地址池为对端**分配隧道地址**（该链路还没有时），并带回**本端**隧道地址
 *         [+ 本端内网网段]，于是主动方一个来回就拿到"自己的地址 + 对端的地址"；
 *   码非 0：只带码（拒绝 / 地址池不可用），不带地址。
 * payload："<码> <给对端的地址/len> <本端隧道地址>[ <本端内网网段>]" */
static int __vpn_reply_notify(vpn_ctx_t *ctx, vpn_link_t *link, uint8_t code)
{
    char payload[VPN_CTRL_CIDR_MAX];
    char given[20], mine[20];

    if (code == VPN_NOTIFY_OK && link->peer_tun_ip == 0) {
        link->peer_tun_ip = __vpn_pool_alloc(ctx);   /* 占用表里取一个空闲地址 */
    }
    if (code == VPN_NOTIFY_OK && link->peer_tun_ip == 0) {
        code = VPN_NOTIFY_ENOADDR;
    }
    if (code != VPN_NOTIFY_OK) {
        snprintf(payload, sizeof(payload), "%u", (unsigned)code);
        return __vpn_send_ctrl(link, VPN_CTRL_NET_NOTIFY_ACK, payload,
                               (int)strlen(payload));
    }

    /* 各段都用规范化短文本（≤ 18 字节）+ 结果码 1 位，整条 payload 必落在 64 字节内 */
    __ipv4_str(link->peer_tun_ip, ctx->pool_prefix, given, sizeof(given));
    __ipv4_str(ctx->my_tun_ip, ctx->pool_prefix, mine, sizeof(mine));
    {
        char codec[4];               /* 结果码枚举只到 4：1 位足够 */
        char net[20];
        vpn_net_t pnet;

        snprintf(codec, sizeof(codec), "%u", (unsigned)code);
        if (ctx->cfg->local_net != NULL &&
            __net_parse(ctx->cfg->local_net, &pnet) == 0) {
            __net_str(&pnet, net, sizeof(net));
            snprintf(payload, sizeof(payload), "%s %s %s %s", codec, given, mine,
                     net);
        } else {
            snprintf(payload, sizeof(payload), "%s %s %s", codec, given, mine);
        }
    }
    dbg_str(DBG_VIP, "vpn: 已接受对端通告，分配 %s 给对端（本端 %s）", given, mine);
    return __vpn_send_ctrl(link, VPN_CTRL_NET_NOTIFY_ACK, payload,
                           (int)strlen(payload));
}

/* NET_NOTIFY 处理（表项直接指向本函数）：收到**主动方**的通告。
 *   payload = 对端的内网网段（可空）；本端按下标给它分配隧道地址，并回应答（带本端地址）。
 *   只记录对端信息，不碰 tun（配 tun 在地址交换之后统一做）。
 * 返回 0=已接受；负值=已拒绝（两种情况应答都已发出，码在应答 payload 里）。 */
static int __vpn_link_handle_notify(vpn_ctx_t *ctx, vpn_link_t *link,
                                    const vpn_ctrl_t *f, int len)
{
    char payload[VPN_CTRL_CIDR_MAX];
    uint8_t code = VPN_NOTIFY_OK;
    int n = f->len;

    /* payload 允许为空 = 对端没配内网网段 */
    if (n < 0 || n >= (int)sizeof(payload) ||
        (int)VPN_CTRL_HDR_LEN + n > len) {
        dbg_str(DBG_ERROR, "vpn: 通告帧 payload 非法(type=%u len=%u)",
                (unsigned)f->type, (unsigned)f->len);
        code = VPN_NOTIFY_EBADPAYLOAD;
        goto reply;
    }
    payload[0] = '\0';
    if (n > 0) {
        memcpy(payload, f->payload, n);
        payload[n] = '\0';
        code = (uint8_t)__vpn_link_set_peer_info(ctx, link, NULL, payload);
        if (code != VPN_NOTIFY_OK) {
            dbg_str(DBG_ERROR, "vpn: 对端通告的网段不合法('%s')，拒绝", payload);
        }
    }

reply:
    __vpn_reply_notify(ctx, link, code);   /* 接受/拒绝都回，只是码不同 */
    return (code == VPN_NOTIFY_OK) ? 0 : -1;
}

/* NET_NOTIFY_ACK：对端（被动方）对本端通告的应答：
 *   payload = "<码> <分配给本端的隧道地址/len> <对端隧道地址>[ <对端内网网段>]"
 *   码 0：记下"本端隧道地址"（稍后据此配 tun）+ "对端地址/网段"（选路用）；
 *   码非 0：拒绝 / 地址池不可用，告警——避免本端以为通了、实际对端没装路由。 */
static int __vpn_ctrl_on_notify_ack(vpn_ctx_t *ctx, vpn_link_t *link,
                                    const vpn_ctrl_t *f, int len)
{
    char payload[VPN_CTRL_CIDR_MAX];
    char mine[32] = {0}, peer[32] = {0}, net[VPN_CTRL_CIDR_MAX] = {0};
    unsigned code = VPN_NOTIFY_OK;
    int n = f->len;
    int got;

    if (link->notify_acked) {
        return 0;                        /* 幂等：只处理第一次 */
    }
    if (n <= 0 || n >= (int)sizeof(payload) ||
        (int)VPN_CTRL_HDR_LEN + n > len) {
        return -1;
    }
    memcpy(payload, f->payload, n);
    payload[n] = '\0';

    got = sscanf(payload, "%u %31s %31s %63s", &code, mine, peer, net);
    if (got < 1) {
        code = VPN_NOTIFY_EBADPAYLOAD;
    }
    if (code != VPN_NOTIFY_OK) {
        link->notify_acked = 1;
        dbg_str(DBG_ERROR, "vpn: 对端拒绝本端通告（结果码 %u）：不会配置本端隧道地址"
                "（码含义见源码 VPN_NOTIFY_*）", code);
        return 0;
    }
    if (got < 2) {
        dbg_str(DBG_ERROR, "vpn: 应答里没有分配给本端的隧道地址('%s')", payload);
        return -1;
    }
    link->notify_acked = 1;

    /* 1) 本端隧道地址：稍后 configure tun 直接用 my_tun_ip_text */
    snprintf(ctx->my_tun_ip_text, sizeof(ctx->my_tun_ip_text), "%s", mine);
    if (__ipv4_parse(mine, &ctx->my_tun_ip) < 0) {
        ctx->my_tun_ip = 0;
    }
    ctx->addr_ready = 1;
    dbg_str(DBG_VIP, "vpn: 对端已接受本端通告，分配本端隧道地址 %s", mine);

    /* 2) 对端隧道地址 [+ 对端内网网段]：选路用 */
    if (got >= 3 &&
        __vpn_link_set_peer_info(ctx, link, peer,
                                 (got >= 4) ? net : NULL) != VPN_NOTIFY_OK) {
        dbg_str(DBG_ERROR, "vpn: 应答里的对端地址/网段不合法('%s'/'%s')，忽略",
                peer, net);
    }
    return 0;
}

static const vpn_ctrl_entry_t g_vpn_ctrl_table[] = {
    { VPN_CTRL_NET_NOTIFY,     "NET_NOTIFY",     __vpn_link_handle_notify },
    { VPN_CTRL_NET_NOTIFY_ACK, "NET_NOTIFY_ACK", __vpn_ctrl_on_notify_ack },
};
#define VPN_CTRL_TABLE_NUM \
    (sizeof(g_vpn_ctrl_table) / sizeof(g_vpn_ctrl_table[0]))

/* 处理某个对端（link）发来的控制帧：按 type 查表分派。
 * 未知类型只记 detail 日志并忽略——这样将来加新帧类型时，新旧版本可共存。 */
static int __vpn_handle_ctrl(vpn_ctx_t *ctx, vpn_link_t *link,
                             const uint8_t *data, int len)
{
    const vpn_ctrl_t *f = (const vpn_ctrl_t *)data;
    size_t i;

    if (len < VPN_CTRL_HDR_LEN) {
        return -1;
    }
    for (i = 0; i < VPN_CTRL_TABLE_NUM; i++) {
        if (g_vpn_ctrl_table[i].type != f->type) {
            continue;
        }
        dbg_str(DBG_DETAIL, "vpn: 收到控制帧 %s", g_vpn_ctrl_table[i].name);
        return g_vpn_ctrl_table[i].handle(ctx, link, f, len);
    }
    dbg_str(DBG_DETAIL, "vpn: 忽略未知控制帧 type=%u", (unsigned)f->type);
    return 0;
}

/* 入站：控制帧在 VPN 层消化；业务 IP 包注入本机 Tun。
 * 运行于 p2p 事件线程，回调内不得阻塞。
 * 多对端：按回调带来的 session 认领/登记 link，于是被叫能同时服务多个主叫。 */
static int __vpn_recv(void *opaque, p2p_session_t *session,
                      const uint8_t *data, int len)
{
    vpn_ctx_t *ctx = (vpn_ctx_t *)opaque;
    vpn_link_t *link;

    if (ctx == NULL || data == NULL || len <= 0) {
        return -1;
    }
    link = __vpn_link_get(ctx, session, 0);   /* 0=被动链路：本端只应答、不发通告 */
    if (link == NULL) {
        return -1;                       /* 已达对端上限：不再接受新对端 */
    }
    /* 控制帧判别：首字节 0xFF 不可能是合法 IP 包（IPv4/IPv6 版本号是 4/6）。
     * 控制帧（通告/应答）必须能收下——**地址交换发生在建 tun 之前**，那时 tun 还不存在。 */
    if (len >= VPN_CTRL_HDR_LEN && (uint8_t)data[0] == 0xFF &&
        memcmp(data, VPN_CTRL_MAGIC, VPN_CTRL_MAGIC_LEN) == 0) {
        return __vpn_handle_ctrl(ctx, link, data, len);
    }
    if (ctx->tun == NULL) {
        return -1;                       /* 还在地址交换阶段：业务包先丢 */
    }
    return ctx->tun->write(ctx->tun, data, len);
}

/* 打开 tun：**只 open + 设 MTU**，不配地址——地址要等交换完成后一次性配置
 * （见 __vpn_configure_tun），避免"先配错地址、拿到后再改"的过程。 */
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
    dbg_str(DBG_INFO, "vpn: tun %s opened (mtu=%d)，待地址交换后配置",
            ctx->tun->name, ctx->tun->mtu);
    return 0;
}

/* 地址交换完成后**一次性**配置 tun：本端地址（对端分配的 / 自己配的）
 * + 各对端的内网网段路由（那些网段在交换/收包阶段已经记下）。 */
static int __vpn_configure_tun(vpn_ctx_t *ctx)
{
    const vpn_cfg_t *cfg = ctx->cfg;
    const char *ip = (ctx->my_tun_ip_text[0] != '\0') ? ctx->my_tun_ip_text
                                                      : cfg->tunnel_ip;

    if (ip == NULL || ip[0] == '\0') {
        dbg_str(DBG_ERROR, "vpn: 无本端隧道地址（对端未分配，且未配 --tunnel-ip）");
        return -1;
    }
    if (ctx->tun->configure(ctx->tun, ip, cfg->netmask) < 0) {
        dbg_str(DBG_ERROR, "vpn: tun configure failed (need root/CAP_NET_ADMIN?)");
        return -1;
    }
    __vpn_install_link_routes(ctx);      /* 对端内网网段路由（全部来自 notify 交换） */
    dbg_str(DBG_VIP, "vpn: tun %s up, tunnel-ip=%s mtu=%d", ctx->tun->name, ip,
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
        p2p_session_t *session = NULL;

        if (p2p_session_create(ctx->node, cfg->peer_id, &session) < 0) {
            dbg_str(DBG_ERROR, "vpn: %s create session to %s failed",
                    cfg->id, cfg->peer_id);
            return -1;
        }
        if (__vpn_link_get(ctx, session, 1) == NULL) {   /* 1=主动链路：由本端发通告 */
            p2p_session_close(session);
            return -1;
        }
    }
    return 0;
}

/* 等第一条链路可用：主叫等自己的会话 CONNECTED（超时失败）；
 * 被叫等任一主叫把会话带进来（常驻不超时，之后仍会继续接受新对端）。 */
static int __vpn_wait_link(vpn_ctx_t *ctx)
{
    struct event_base *eb = event_base_get_default_instance();
    int i;

    for (i = 0; ; i++) {
        if (eb != NULL && eb->eb != NULL && eb->eb->break_flag) {
            return -1;   /* Ctrl+C */
        }
        if (ctx->dial) {
            vpn_link_t *l = __vpn_link_dialer(ctx);

            if (l != NULL && p2p_session_is_connected(l->session) == 0) {
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
            if (ctx->n_links > 0) {
                dbg_str(DBG_VIP, "vpn: %s got peer session (callee), links=%d",
                        ctx->cfg->id, ctx->n_links);
                return 0;
            }
        }
        usleep(200000);
    }
}

/* 地址交换（**发生在配 tun 之前**）：只有**主动建链方**需要等待——
 *   它主动发一次 NET_NOTIFY（只带本端内网网段）；对端回 NET_NOTIFY_ACK，里面是
 *   "分配给本端的隧道地址 + 对端自己的隧道地址 [+ 对端内网网段]"。
 * 没收到就约每 1s 重发，最多 VPN_NET_NOTIFY_MAX_RETRY 次；超时返回 -1。
 * 被动方（被叫/hub）不等：它用 --tunnel-ip 作为自身地址，收到通告时就地分配并在应答里带回。 */
static int __vpn_exchange_addr(vpn_ctx_t *ctx)
{
    struct event_base *eb = event_base_get_default_instance();
    int i, rounds = 0;

    if (!ctx->dial) {
        return 0;                        /* 被动方：地址就是 --tunnel-ip，无需等待 */
    }
    for (;;) {
        if (eb != NULL && eb->eb != NULL && eb->eb->break_flag) {
            return -1;                   /* Ctrl+C */
        }
        if (ctx->addr_ready) {
            return 0;                    /* 已拿到对端分配的本端隧道地址 */
        }
        for (i = 0; i < VPN_MAX_LINKS; i++) {
            vpn_link_t *l = &ctx->links[i];

            if (!l->is_dialer || l->notify_acked ||
                l->notify_retries >= VPN_NET_NOTIFY_MAX_RETRY) {
                continue;                /* 被动链路不发 / 已应答 / 次数用完 */
            }
            if (p2p_session_is_connected(l->session) != 0) {
                continue;                /* 还没打通 */
            }
            if (l->notify_retries > 0 && (rounds % 5) != 0) {
                continue;                /* 已发过：约 1s 才重发一次 */
            }
            {
                char payload[VPN_CTRL_CIDR_MAX];
                const char *net = ctx->cfg->local_net;
                vpn_net_t pnet;

                /* 通告只带**本端内网网段**（自己的隧道地址由对端在应答里分配，
                 * 所以不用带）；没配或非法就发空 payload，等于只说"我来了"。
                 * 这里用 __net_parse 校验并**规范化**（如 10.0.0.5/24 -> 10.0.0.0/24），
                 * 于是对端收到的永远是"网络地址/len"形式的干净文本。 */
                payload[0] = '\0';
                if (net != NULL && net[0] != '\0') {
                    if (__net_parse(net, &pnet) < 0) {
                        dbg_str(DBG_ERROR, "vpn: --local-net 非法(%s)，通告不带网段",
                                net);
                    } else {
                        __net_str(&pnet, payload, sizeof(payload));
                    }
                }
                l->notify_retries++;
                if (l->notify_retries == 1) {
                    dbg_str(DBG_VIP, "vpn: 已向对端通告本端内网%s%s，等对端分配地址",
                            (payload[0] != '\0') ? " " : "",
                            (payload[0] != '\0') ? payload : "(未配 --local-net)");
                }
                __vpn_send_ctrl(l, VPN_CTRL_NET_NOTIFY, payload,
                                (int)strlen(payload));
            }
        }
        if (rounds >= VPN_WAIT_ROUNDS) {
            dbg_str(DBG_ERROR, "vpn: 等待对端分配隧道地址超时(%ds)",
                    VPN_WAIT_ROUNDS / 5);
            return -1;
        }
        usleep(200000);
        rounds++;
    }
}

/* 转发循环（出站）：poll 带超时以便响应 Ctrl+C；入站走 __vpn_recv 回调，无额外线程。
 * 多对端：读到一个包后按目的地址选 link 发送；空闲 tick 里兜底安装后到对端的网段路由。 */
static void __vpn_forward(vpn_ctx_t *ctx)
{
    struct event_base *eb = event_base_get_default_instance();
    uint8_t buf[VPN_RW_BUF_SIZE];
    int idle_rounds = 0;

    for (;;) {
        struct pollfd pfd;
        int n, i;

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
            /* 空闲 tick（5 × 200ms ≈ 1s）：
             *   1) 回收断链的对端（关会话 + 归还隧道地址 + 腾出槽位）；
             *   2) 兜底安装"后到的对端"的内网网段路由（幂等）。 */
            if (++idle_rounds >= 5) {
                idle_rounds = 0;
                __vpn_release_dead_links(ctx);
                __vpn_install_link_routes(ctx);
            }
            continue;   /* 超时：回到循环顶检查退出标志 */
        }
        n = ctx->tun->read(ctx->tun, buf, sizeof(buf));
        if (n <= 0) {
            continue;
        }
        /* 按目的地址选对端：对端隧道地址（精确）/ 对端内网网段（最长前缀）。
         * 多对端且都不命中 -> 丢弃，绝不"猜"着发给某一条链路。 */
        {
            vpn_link_t *l = __vpn_link_pick(ctx, __ipv4_dst(buf, n));

            if (l == NULL) {
                static int drop_logs;

                if ((drop_logs++ % 64) == 0) {
                    dbg_str(DBG_INFO, "vpn: 出站包无匹配的对端（需对端通告内网网段，"
                            "或目的为对端隧道地址），已丢弃");
                }
                continue;
            }
            if (p2p_session_is_connected(l->session) != 0) {
                continue;   /* 还没打通：丢弃出站包（等建链完成） */
            }
            if (p2p_session_send(l->session, buf, n) < 0) {
                dbg_str(DBG_INFO, "vpn: send %d bytes failed", n);
            }
        }
    }
}

int vpn_run(const vpn_cfg_t *cfg)
{
    vpn_ctx_t ctx;
    int i, ret = -1;      /* i：下面关闭全部 link 时用 */

    if (cfg == NULL || cfg->id == NULL ||
        cfg->signal_host == NULL || cfg->signal_service == NULL) {
        dbg_str(DBG_ERROR, "vpn: bad cfg (id/signal_* required)");
        return -1;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.cfg = cfg;
    ctx.dial = (cfg->peer_id != NULL);

    /* 被动方（被连的一方）是**地址分配者**：必须有自己的隧道地址（也是地址池来源） */
    if (!ctx.dial && (cfg->tunnel_ip == NULL || cfg->tunnel_ip[0] == '\0')) {
        dbg_str(DBG_ERROR, "vpn: 被动方必须配 tunnel_ip（自身地址 + 地址池）");
        return -1;
    }
    __vpn_pool_init(&ctx);

    if (__vpn_open_tun(&ctx) < 0) {          /* 只 open + MTU，不配地址 */
        goto out;
    }
    if (__vpn_open_p2p(&ctx) < 0) {
        goto out;
    }
    if (__vpn_wait_link(&ctx) < 0) {
        goto out;
    }
    if (__vpn_exchange_addr(&ctx) < 0) {     /* 主叫：发通告 + 等对端分配地址 */
        goto out;
    }
    if (__vpn_configure_tun(&ctx) < 0) {     /* 地址齐了，再一次性配 tun */
        goto out;
    }

    if (cfg->on_ready != NULL) {
        cfg->on_ready(cfg->opaque);
    }
    dbg_str(DBG_VIP, "vpn: tunnel up (%s, ip=%s, %d link(s)), Ctrl+C to stop",
            ctx.tun->name,
            (ctx.my_tun_ip_text[0] != '\0') ? ctx.my_tun_ip_text : cfg->tunnel_ip,
            ctx.n_links);
    __vpn_forward(&ctx);
    dbg_str(DBG_VIP, "vpn: stopped");
    ret = 0;

out:
    if (ret < 0 && cfg->on_error != NULL) {
        cfg->on_error(cfg->opaque, ret);
    }
    for (i = 0; i < VPN_MAX_LINKS; i++) {
        __vpn_link_release(&ctx, &ctx.links[i]);   /* 关会话 + 归还隧道地址 */
    }
    if (ctx.node != NULL) {
        p2p_node_close(ctx.node);
    }
    if (ctx.tun != NULL) {
        tun_destroy(ctx.tun);
    }
    return ret;
}
