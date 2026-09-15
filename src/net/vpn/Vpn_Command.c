/**
 * @file Vpn_Command.c
 * @Synopsis  VPN 命令行（xtools vpn ...）：解析选项 -> 组装 vpn_cfg_t -> 调 vpn_run()
 *
 * 复用项目 Command/Option 框架（与 httpd 命令同一范式）：
 *   - __construct 里 add_option 注册选项，并把"字段地址"作为 action 的 opaque；
 *   - 选项回调统一 __option_str_callback / __option_int_callback，直接写进本命令字段；
 *   - __run_command 做必填校验、host:port 拆分，然后 vpn_run（阻塞至 Ctrl+C）。
 *   - 末尾 REGISTER_APP_CMD(Vpn_Command, ...) 注册为 app 子命令；
 *     用户看到的命令名由 "/Command/name" 决定（这里 = "vpn"）。
 *
 * 用法：
 *   xtools vpn -i vpnA -p vpnB -s 119.4.206.14:12345 --tunnel-ip 10.0.0.1/24 --local-net 172.16.10.0/23
 *   xtools vpn --help
 *
 * @author Zoo
 * @date 2026-09-15
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/argument/Command.h>
#include <libobject/argument/Application.h>   /* REGISTER_APP_CMD / app_register_cmd */
#include <libobject/net/vpn/Vpn.h>
#include <libobject/net/vpn/Vpn_Command.h>

/* 采址 STUN / 保活周期的默认值统一来自 Vpn.h（test_vpn 共用同一份） */
#define VPN_DEFAULT_INTERVAL  VPN_DEFAULT_INTERVAL_MS

/* 通用字符串选项回调：opaque 传的是目标字段地址(char **)，直接写进去。 */
static int __option_str_callback(Option *option, void *opaque)
{
    char **slot = (char **)opaque;

    if (slot == NULL || option == NULL || option->value == NULL) {
        return 0;
    }
    *slot = STR2A(option->value);   /* 指向 Option 内 String 的缓冲，随命令对象存活 */
    return 1;
}

/* 通用整型选项回调：opaque 传的是目标字段地址(int *)。 */
static int __option_int_callback(Option *option, void *opaque)
{
    int *slot = (int *)opaque;

    if (slot == NULL || option == NULL || option->value == NULL) {
        return 0;
    }
    *slot = atoi(STR2A(option->value));
    return 1;
}

/*
 * "host:port" -> 拆成 host / port。
 *   - 无 ':' 时：host=整体，port=def_port（def_port 为空则 port 为空串，由调用方判错）；
 *   - 用 strrchr 取最后一个 ':'，天然兼容 "host:port"；不含 IPv6 字面量支持（首版）。
 */
static int __split_host_port(const char *in, char *host, int hlen,
                             char *port, int plen, const char *def_port)
{
    const char *colon;
    int n;

    if (in == NULL || host == NULL || hlen <= 0) {
        return -1;
    }
    colon = strrchr(in, ':');
    if (colon == NULL) {
        snprintf(host, hlen, "%s", in);
        if (port != NULL && plen > 0) {
            snprintf(port, plen, "%s", (def_port != NULL) ? def_port : "");
        }
        return 0;
    }
    n = (int)(colon - in);
    if (n <= 0 || n >= hlen) {
        return -1;
    }
    snprintf(host, hlen, "%.*s", n, in);
    if (port != NULL && plen > 0) {
        snprintf(port, plen, "%s", colon + 1);
    }
    return 0;
}

/* 空串一律当"未设置"，返回 NULL —— 便于直接塞进 vpn_cfg_t。 */
static char *__or_null(char *s)
{
    return (s != NULL && s[0] != '\0') ? s : NULL;
}

/* none/off/0 -> 表示"显式禁用"（如 --stun none = 回退到用信令服采址）。 */
static int __is_off(const char *s)
{
    return (s != NULL) && (strcmp(s, "none") == 0 || strcmp(s, "off") == 0 ||
                           strcmp(s, "0") == 0);
}

static int __construct(Vpn_Command *command, char *init_str)
{
    Command *c = (Command *)command;

    dbg_str(DBG_VIP, "construct vpn command");
    command->interval_ms = VPN_DEFAULT_INTERVAL;

    /* 身份 / 信令（说明面向用户，短句讲清"为什么填"） */
    c->add_option(c, "--id", "-i", "",
                  "本端名字(唯一标识，必填)；对端用 --peer 填它才能连到你",
                  __option_str_callback, &command->stun_id);
    c->add_option(c, "--peer", "-p", "",
                  "要连的对端名字；填了=主动连它(主叫)，不填=等对方连你(被叫)",
                  __option_str_callback, &command->peer_id);
    c->add_option(c, "--signal", "-s", "",
                  "信令服务器 host:port（必填）；双方靠它交换地址、撮合打洞",
                  __option_str_callback, &command->signal);
    /* 默认与已验证的测试命令一致：公网 STUN 采址。
     * 注意：若与信号服同内网而"用信令服采址"，采到的是内网地址，打洞会失败。
     * 想改回"用信令服采址"，把 --stun 指向信令服即可；写 none/off 可禁用（回退到信令服）。 */
    c->add_option(c, "--stun", "", VPN_DEFAULT_STUN_HOST ":" VPN_DEFAULT_STUN_PORT,
                  "采址用 STUN host[:port]（探测本机公网地址）；默认公网 STUN，可指向信令服",
                  __option_str_callback, &command->stun);
    c->add_option(c, "--stun2", "", VPN_DEFAULT_STUN2_HOST ":" VPN_DEFAULT_STUN2_PORT,
                  "第二个 STUN（判断 NAT 是否对称）；none/off 可禁用",
                  __option_str_callback, &command->stun2);
    c->add_option(c, "--local-service", "-l", "",
                  "本端数据口(UDP)：防火墙只放行特定端口时请固定它(如 -l 12346)；省略=随机",
                  __option_str_callback, &command->local_service);
    c->add_option(c, "--interval", "", "200",
                  "打洞/保活周期(毫秒)，默认 200",
                  __option_int_callback, &command->interval_ms);

    /* 虚拟网卡 / 路由 */
    c->add_option(c, "--tun", "-t", "",
                  "虚拟网卡名；省略=自动分配 tunN",
                  __option_str_callback, &command->tun_name);
    /* 前缀直接写在 --tunnel-ip 里（a.b.c.d/len），省略 /len 时按 /24，故不再单列 --netmask */
    c->add_option(c, "--tunnel-ip", "", "",
                  "本端隧道地址(必填)，如 10.0.0.1/24；两端必须同网段(对端如 10.0.0.2/24)才能互通",
                  __option_str_callback, &command->tunnel_ip);
    /* 只填"自己的"内网网段：链路建立后自动通告给对端，对端据此自动加路由。
     * 这样两端各自只需知道自己的网络，不必知道对方的内网。 */
    c->add_option(c, "--local-net", "", "",
                  "本端内网网段(如 172.16.10.0/23)：链路通了自动通告对端、对端自动加路由；省略=不通告",
                  __option_str_callback, &command->local_net);

    c->set(c, "/Command/name", "vpn");
    c->set(c, "/Command/description",
           "p2p VPN: L3 site-to-site tunnel over p2p (Linux TUN, root required)");

    return 0;
}

static int __deconstruct(Vpn_Command *command)
{
    dbg_str(DBG_VIP, "deconstruct vpn command");
    /* 字段都指向 Option 内 String 的缓冲，无需释放 */
    return 0;
}

static int __run_command(Vpn_Command *command)
{
    vpn_cfg_t cfg;
    char signal_host[128] = {0}, signal_port[32] = {0};
    char stun_host[128]   = {0}, stun_port[32]   = {0};
    char stun2_host[128]  = {0}, stun2_port[32]  = {0};

    if (__or_null(command->stun_id) == NULL) {
        dbg_str(DBG_ERROR, "vpn: --id 必填（-i <stun_id>）；--help 查看用法");
        return -1;
    }
    if (__or_null(command->tunnel_ip) == NULL) {
        dbg_str(DBG_ERROR, "vpn: --tunnel-ip 必填（本端隧道地址，如 10.0.0.1/24）");
        return -1;
    }
    if (__or_null(command->signal) == NULL) {
        dbg_str(DBG_ERROR, "vpn: --signal 必填（信令服务器 host:port）");
        return -1;
    }
    if (__split_host_port(command->signal, signal_host, sizeof(signal_host),
                          signal_port, sizeof(signal_port), NULL) < 0 ||
        signal_port[0] == '\0') {
        dbg_str(DBG_ERROR, "vpn: --signal 需为 host:port，如 119.4.206.14:12345");
        return -1;
    }
    /* --stun/--stun2：none/off/0 表示禁用（此时回退为"用信令服采址"） */
    if (__or_null(command->stun) != NULL && !__is_off(command->stun)) {
        if (__split_host_port(command->stun, stun_host, sizeof(stun_host),
                              stun_port, sizeof(stun_port), VPN_DEFAULT_STUN_PORT) < 0) {
            dbg_str(DBG_ERROR, "vpn: --stun 格式错（应为 host[:port]）");
            return -1;
        }
    }
    if (__or_null(command->stun2) != NULL && !__is_off(command->stun2)) {
        if (__split_host_port(command->stun2, stun2_host, sizeof(stun2_host),
                              stun2_port, sizeof(stun2_port), VPN_DEFAULT_STUN2_PORT) < 0) {
            dbg_str(DBG_ERROR, "vpn: --stun2 格式错（应为 host[:port]）");
            return -1;
        }
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.id             = command->stun_id;
    cfg.peer_id        = __or_null(command->peer_id);        /* NULL=被叫 */
    cfg.local_service  = __or_null(command->local_service);
    cfg.signal_host    = signal_host;
    cfg.signal_service = signal_port;
    cfg.stun_host      = __or_null(stun_host);               /* NULL=用信令服 */
    cfg.stun_service   = __or_null(stun_port);
    cfg.stun2_host     = __or_null(stun2_host);
    cfg.stun2_service  = __or_null(stun2_port);
    cfg.interval_ms    = command->interval_ms;
    cfg.tun_name       = __or_null(command->tun_name);       /* NULL=自动 */
    cfg.tunnel_ip      = command->tunnel_ip;                 /* 前缀写在 --tunnel-ip 的 /len 里 */
    cfg.local_net      = __or_null(command->local_net);
    cfg.remote_net     = NULL;                               /* 对端网段改为运行时自动交换，不再手工填 */

    dbg_str(DBG_VIP, "vpn: id=%s peer=%s signal=%s:%s service=%s stun=%s:%s stun2=%s:%s "
            "tunnel-ip=%s local-net=%s tun=%s interval=%d",
            cfg.id, cfg.peer_id ? cfg.peer_id : "(callee)",
            cfg.signal_host, cfg.signal_service,
            cfg.local_service ? cfg.local_service : "(random)",
            cfg.stun_host ? cfg.stun_host : "(signal)",
            cfg.stun_service ? cfg.stun_service : "-",
            cfg.stun2_host ? cfg.stun2_host : "-",
            cfg.stun2_service ? cfg.stun2_service : "-",
            cfg.tunnel_ip,
            cfg.local_net ? cfg.local_net : "-",
            cfg.tun_name ? cfg.tun_name : "(auto)", cfg.interval_ms);

    if (cfg.local_net == NULL) {
        dbg_str(DBG_WARN, "vpn: 未指定 --local-net：本端不会向对端通告内网网段，"
                "对端也就不会自动加路由（只做隧道连通性测试时可忽略）");
    }

    return vpn_run(&cfg);   /* 阻塞至 Ctrl+C */
}

static class_info_entry_t vpn_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Vpn_Command, construct, __construct),
    Init_Nfunc_Entry(2, Vpn_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Vpn_Command, run_command, __run_command),
    Init_End___Entry(4, Vpn_Command),
};
REGISTER_APP_CMD(Vpn_Command, vpn_command_class_info);
