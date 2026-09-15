/**
 * @file Vpn_Server_Command.c
 * @Synopsis  VPN 服务端命令行（xtools vpnserver ...）
 *
 * vpn 复用 p2p 的中心服务器：同一 UDP 端口兼「信令地址簿/撮合 + STUN 回显」（预留 TURN），
 * 即 p2p_server_run()。本命令是它的正式入口（命令行形态见 doc/net/vpn/README.md）。
 *
 * 用法：
 *   ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpnserver -l 0.0.0.0:12345     # 阻塞运行，Ctrl+C 停止
 *
 * @author Zoo
 * @date 2026-09-15
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/argument/Command.h>
#include <libobject/argument/Application.h>   /* REGISTER_APP_CMD */
#include <libobject/net/p2p/p2p.h>            /* p2p_server_run */
#include <libobject/net/vpn/Vpn_Server_Command.h>

#define VPN_SERVER_DEFAULT_LISTEN "0.0.0.0:12345"

static int __option_str_callback(Option *option, void *opaque)
{
    char **slot = (char **)opaque;

    if (slot == NULL || option == NULL || option->value == NULL) {
        return 0;
    }
    *slot = STR2A(option->value);
    return 1;
}

/* "host:port" -> 拆分；host 为空（":12345"）按 0.0.0.0 处理。 */
static int __split_host_port(const char *in, char *host, int hlen,
                             char *port, int plen)
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
            port[0] = '\0';
        }
        return 0;
    }
    n = (int)(colon - in);
    if (n == 0) {
        snprintf(host, hlen, "%s", "0.0.0.0");
    } else {
        snprintf(host, hlen, "%.*s", n, in);
    }
    if (port != NULL && plen > 0) {
        snprintf(port, plen, "%s", colon + 1);
    }
    return 0;
}

static int __construct(Vpn_Server_Command *command, char *init_str)
{
    Command *c = (Command *)command;

    dbg_str(DBG_VIP, "construct vpnserver command");

    c->add_option(c, "--listen", "-l", VPN_SERVER_DEFAULT_LISTEN,
                  "监听地址 host:port（UDP，默认 0.0.0.0:12345）",
                  __option_str_callback, &command->listen);

    c->set(c, "/Command/name", "vpnserver");
    c->set(c, "/Command/description",
           "vpn/p2p signalling + STUN server (same process, UDP); Ctrl+C to stop");

    return 0;
}

static int __deconstruct(Vpn_Server_Command *command)
{
    dbg_str(DBG_VIP, "deconstruct vpnserver command");
    return 0;
}

static int __run_command(Vpn_Server_Command *command)
{
    char host[128] = {0}, port[32] = {0};
    const char *listen = (command->listen != NULL && command->listen[0] != '\0')
                       ? command->listen : VPN_SERVER_DEFAULT_LISTEN;

    if (__split_host_port(listen, host, sizeof(host), port, sizeof(port)) < 0 ||
        port[0] == '\0') {
        dbg_str(DBG_ERROR, "vpnserver: --listen 需为 host:port，如 0.0.0.0:12345");
        return -1;
    }

    dbg_str(DBG_VIP, "vpnserver: listening on %s:%s (udp), Ctrl+C to stop",
            host, port);

    return p2p_server_run(host, port);   /* 阻塞至 Ctrl+C */
}

static class_info_entry_t vpn_server_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Vpn_Server_Command, construct, __construct),
    Init_Nfunc_Entry(2, Vpn_Server_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Vpn_Server_Command, run_command, __run_command),
    Init_End___Entry(4, Vpn_Server_Command),
};
REGISTER_APP_CMD(Vpn_Server_Command, vpn_server_command_class_info);
