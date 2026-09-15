/**
 * @file test_vpn.c
 * @Synopsis  VPN 模块自测：Tun 自检 + 双端站点互访（vpn_run）
 *
 * 依赖 p2p 通道：需要一个信令服务器（也用 test_p2p_server）、两个 VPN 端点。
 *
 * 运行方式（构建后；Tun 相关命令需 root/CAP_NET_ADMIN；命令形态与 test_p2p_peer 一致）：
 *
 * !! 下面的示例仅供阅读：C 注释每行带 " * " 前缀，直接复制会被 shell 通配展开( * -> 文件名)
 * !! 导致 mockery 解析错乱并卡住。请从 doc/net/vpn/README.md 的 bash 代码块复制命令。
 * !! 且 test_vpn_peer 是常驻阻塞命令，请在独立终端运行、Ctrl+C 退出。
 *
 *   1) Tun 自检（**单机全自动**，CMD，**无参数**；需 root）：
 *        sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_vpn_tun
 *      全用默认值(10.99.0.1/24 + 10.99.9.0/24)：自造探针包(UDP -> 10.99.9.2)按
 *      remote_cidr 路由读回，读到即 PASS；无需第二个终端 ping，也无需对端。
 *
 *   2) 真机跨 NAT（服务器放行 UDP 12345；两端分别在各自 NAT 后）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_p2p_server 12345
 *        # 节点 B（被叫）：会话口固定 12346；本端 tun 10.0.0.2，要访问 A 侧 172.16.10.0/23
 *        #   （服务器内网口 10.10.10.115；B 侧内网 10.10.10.0/24 即 10.10.10.115/24）
 *        sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_vpn_peer vpnB 12346 10.10.10.115 12345 10.0.0.2 172.16.10.0/23
 *        # 等价命令行: xtools vpn -i vpnB -l 12346 -s 10.10.10.115:12345 \
 *        #              --ip 10.0.0.2/24 -r 172.16.10.0/23
 *        # 节点 A（主叫）：会话口固定 19001；本端 tun 10.0.0.1，要访问 B 侧 10.10.10.0/24
 *        #   （A 侧内网 172.16.10.33/23，netmask 255.255.254.0）
 *        sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_vpn_peer vpnA 19001 119.4.206.14 12345 10.0.0.1 10.10.10.0/24 vpnB
 *        # 等价命令行: xtools vpn -i vpnA -p vpnB -l 19001 -s 119.4.206.14:12345 \
 *        #              --ip 10.0.0.1/24 -r 10.10.10.0/24
 *
 *   3) 单机回环（信令服务器兼 STUN；末尾 <stun_host> <stun_port> 覆盖成 127.0.0.1 9000）：
 *        ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_p2p_server 9000
 *        sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_vpn_peer vpnB 12346 127.0.0.1 9000 10.0.0.2 10.0.0.0/24 auto 127.0.0.1 9000
 *        sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
 *             test_vpn_peer vpnA 19001 127.0.0.1 9000 10.0.0.1 10.0.0.0/24 vpnB auto 127.0.0.1 9000
 *
 *      注：mockery 会把以 '-' 开头的实参当自己的选项处理，**选项式传不进来**，所以本测试
 *          只能收**位置参数**（上面 '#' 行是对应的等价命令行，见 xtools vpn --help）。
 *
 * @author Zoo
 * @date 2026-09-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/argument/Command.h>   /* Command vfunc: set_args/parse_args/run_command */
#include <libobject/net/vpn/Vpn.h>
#include "../../src/net/vpn/tun/Tun.h"

/* 采址 STUN / 数据口等默认值统一在 Vpn_Command.c 里定义；
 * 本文件不再自带一份，避免"测试与正式 CLI 默认值不一致"再次踩坑。 */

/* ======================= 1) Tun 自检（单机全自动） ======================= */
/*
 * test_vpn_tun（CMD，**不接受参数**，全部使用下面 VPN_TUN_TEST_* 默认值）：
 *   单机即可自测 Tun 的 open/configure/read 通路（需 root/CAP_NET_ADMIN）：
 *     1) 建 tun + 配 <IP>/<MASK> + 按 <REMOTE_CIDR> 加路由；
 *     2) 自动向 REMOTE_CIDR 内一个地址发 UDP 探针（内核按路由把它交给 tun）；
 *     3) 从 tun 读回并校验(IPv4 + UDP + 目的地址)，读到即 PASS。
 *   无需第二个终端 ping，也无需对端；两个网段刻意不同，才能真正验证那道路由。
 *   用 CMD 而非 FUNC 用例：它需要 root，避免被全量 mockery 用例自动跑到而误报失败。
 * 返回 1=PASS；0=未读到(FAIL)；负=环境/参数错误。
 *
 * 运行：sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_vpn_tun
 */
#define VPN_TUN_TEST_IP          "10.99.0.1"      /* 本端 tun 地址（host） */
#define VPN_TUN_TEST_MASK        "24"             /* 上面地址的掩码（也可写 255.255.255.0） */
#define VPN_TUN_TEST_REMOTE_CIDR "10.99.9.0/24"   /* 路由目的网段（与上面刻意不同网段） */
#define VPN_TUN_TEST_NAME        NULL             /* NULL=内核自动分配 tunN */
#define VPN_TUN_TEST_PROBE_PORT  33445            /* 探针 UDP 目的端口 */

/*
 * 取 remote_cidr 内的探针地址：网段基址 + 2（避开 .0/.1 常用网关）；/31、/32 用其自身。
 * 必须避开 local_ip：否则内核判定"目的地址是本机"→ 本地投递，包不会进 tun，导致误判 FAIL。
 */
static int __probe_addr_from_cidr(const char *cidr, const char *local_ip,
                                  char *out, int outlen)
{
    char tmp[64], lip[64], *slash;
    struct in_addr addr, laddr;
    uint32_t ip, mask, probe, local = 0;
    int len = 32, n;

    if (cidr == NULL || out == NULL || outlen <= 0) {
        return -1;
    }
    snprintf(tmp, sizeof(tmp), "%s", cidr);
    slash = strchr(tmp, '/');
    if (slash != NULL) {
        *slash = '\0';
        len = atoi(slash + 1);
    }
    if (inet_pton(AF_INET, tmp, &addr) != 1) {
        return -1;
    }
    ip = ntohl(addr.s_addr);

    if (local_ip != NULL) {                       /* 解析本端地址（允许带 /prefix） */
        snprintf(lip, sizeof(lip), "%s", local_ip);
        slash = strchr(lip, '/');
        if (slash != NULL) {
            *slash = '\0';
        }
        if (inet_pton(AF_INET, lip, &laddr) == 1) {
            local = ntohl(laddr.s_addr);
        }
    }

    if (len >= 31) {
        probe = ip;
    } else {
        mask  = (len <= 0) ? 0 : (0xffffffffu << (32 - len));
        probe = (ip & mask) + 2;
        for (n = 0; n < 4 && (probe == local || probe == ip); n++) {
            probe += 1;                           /* 避开本端地址 / 原样基址 */
        }
    }
    addr.s_addr = htonl(probe);
    return (inet_ntop(AF_INET, &addr, out, outlen) != NULL) ? 0 : -1;
}

static int test_vpn_tun(TEST_ENTRY *entry, int argc, char **argv)
{
    Tun *tun;
    const char *ip   = VPN_TUN_TEST_IP;        /* 全部写死，不接受命令行参数 */
    const char *mask = VPN_TUN_TEST_MASK;
    const char *cidr = VPN_TUN_TEST_REMOTE_CIDR;
    const char *name = VPN_TUN_TEST_NAME;
    char probe[64];
    int i, fd, n, got = 0, sent = 0;
    uint8_t buf[4096], payload[16];
    struct sockaddr_in dst;

    (void)entry;
    (void)argc;      /* CMD 形态保留签名，但本用例不使用任何参数 */
    (void)argv;

    if (__probe_addr_from_cidr(cidr, ip, probe, sizeof(probe)) < 0) {
        dbg_str(DBG_ERROR, "test_vpn_tun: bad remote_cidr '%s'", cidr);
        return -1;
    }

    tun = tun_create();
    if (tun == NULL || tun->open(tun, name) < 0) {
        dbg_str(DBG_ERROR, "test_vpn_tun: open failed (need root/CAP_NET_ADMIN?)");
        if (tun != NULL) {
            tun_destroy(tun);
        }
        return -1;
    }
    tun->set_mtu(tun, tun->mtu);
    if (tun->configure(tun, ip, mask, cidr) < 0) {
        dbg_str(DBG_ERROR, "test_vpn_tun: configure failed");
        tun_destroy(tun);
        return -1;
    }
    dbg_str(DBG_VIP, "test_vpn_tun: %s up ip=%s/%s remote=%s, probing %s ...",
            tun->name, ip, mask, cidr, probe);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        dbg_str(DBG_ERROR, "test_vpn_tun: socket failed: %s", strerror(errno));
        tun_destroy(tun);
        return -1;
    }
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_port   = htons(VPN_TUN_TEST_PROBE_PORT);
    inet_pton(AF_INET, probe, &dst.sin_addr);
    memset(payload, 0x5a, sizeof(payload));

    /* 发包 3 次 + 读 tun（200ms * 10），校验目的地址与协议 */
    for (i = 0; i < 10 && got == 0; i++) {
        struct pollfd pfd;

        if (i < 3) {
            n = sendto(fd, payload, sizeof(payload), 0,
                       (struct sockaddr *)&dst, sizeof(dst));
            if (n > 0) {
                sent++;
            } else if (sent == 0) {
                dbg_str(DBG_ERROR, "test_vpn_tun: sendto %s failed: %s",
                        probe, strerror(errno));
            }
        }
        memset(&pfd, 0, sizeof(pfd));
        pfd.fd = tun->fd;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, 200) <= 0) {
            continue;
        }
        n = tun->read(tun, buf, sizeof(buf));
        if (n < 20 || (buf[0] >> 4) != 4) {
            continue;   /* 非 IPv4 包，忽略 */
        }
        if (buf[9] == IPPROTO_UDP &&
            memcmp(buf + 16, &dst.sin_addr.s_addr, 4) == 0) {
            got++;
            dbg_str(DBG_INFO, "test_vpn_tun: got probe packet %d bytes"
                    " (udp -> %s)", n, probe);
        }
    }
    close(fd);

    dbg_str(DBG_VIP, "test_vpn_tun: %s %s (sent=%d, got=%d)", tun->name,
            (got > 0) ? "PASS" : "FAIL", sent, got);
    tun_destroy(tun);
    return (got > 0) ? 1 : 0;
}
REGISTER_TEST_CMD(test_vpn_tun);

/* ======================= 2) VPN 端点（走 vpn 命令行实现） ======================= */
/*
 * test_vpn_peer <stun_id> <local_service> <signal_host> <signal_port>
 *               <local_ip> <remote_cidr> [<peer_id> [<tun_name>
 *               [<stun_host> <stun_port>]]]
 *
 * 实现方式：**复用正式命令行** —— 构造 Vpn_Command，把位置参数逐项写进它的 Option，
 * 再跑它的 run_command（与 `xtools vpn` 完全同一条路径，不再自己拼 vpn_cfg_t，
 * 默认值/校验/转发逻辑都只有一份）。
 *
 * 为什么用位置参数而不是 `-i/-s/...` 选项：mockery 会把以 '-' 开头的实参当自己的选项处理，
 * 选项根本传不到命令里（实测连测试函数都不会被调用）。等价命令行见上面 '#' 注释。
 *
 * 常驻运行至 Ctrl+C（与 vpn 命令一致）；返回 0=正常停止，负=失败。
 */

/* 把一个值写进命令的某个选项（等价于命令行 `--name value`）。 */
static void __set_cmd_option(Command *cmd, const char *name, const char *val)
{
    Option *o;

    if (cmd == NULL || name == NULL || val == NULL || val[0] == '\0') {
        return;
    }
    o = cmd->get_option(cmd, (char *)name);
    if (o != NULL) {
        o->set(o, "value", (char *)val);
    }
}

static int test_vpn_peer(TEST_ENTRY *entry, int argc, char **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *cmd;
    char signal[160], stun[160];
    int ret;

    if (argc < 7) {
        dbg_str(DBG_ERROR, "usage: test_vpn_peer <stun_id> <local_service>"
                " <signal_host> <signal_port> <local_ip> <remote_cidr>"
                " [<peer_id> [<tun_name> [<stun_host> <stun_port>]]]\n"
                "  <local_service> auto/0=随机（mockery 会丢 '-' 开头的参数，别写 '-'）\n"
                "  <tun_name> auto/省略=自动分配 tunN\n"
                "  带 <peer_id> = 主叫；不带 = 被叫\n"
                "  等价命令行: xtools vpn -i <id> [-p <peer>] -l <svc> -s <host:port>"
                " --ip <ip/len> -r <cidr>");
        return -1;
    }

    cmd = object_new(allocator, "Vpn_Command", NULL);
    if (cmd == NULL) {
        dbg_str(DBG_ERROR, "test_vpn_peer: create Vpn_Command failed");
        return -1;
    }

    /* 位置参数 -> 命令选项（与 `xtools vpn` 同名同义，默认值由命令自身提供） */
    __set_cmd_option(cmd, "--id", argv[1]);
    __set_cmd_option(cmd, "--local-service",
                     ((strcmp(argv[2], "auto") != 0) &&
                      (strcmp(argv[2], "0") != 0)) ? argv[2] : NULL);
    snprintf(signal, sizeof(signal), "%s:%s", argv[3], argv[4]);
    __set_cmd_option(cmd, "--signal", signal);
    __set_cmd_option(cmd, "--ip", argv[5]);
    __set_cmd_option(cmd, "--route", argv[6]);
    if (argc >= 8) {
        __set_cmd_option(cmd, "--peer", argv[7]);
    }
    if (argc >= 9 && strcmp(argv[8], "auto") != 0) {
        __set_cmd_option(cmd, "--tun", argv[8]);
    }
    if (argc >= 11) {   /* 可选覆盖主 STUN（同机回环可指向信令服自身） */
        snprintf(stun, sizeof(stun), "%s:%s", argv[9], argv[10]);
        __set_cmd_option(cmd, "--stun", stun);
    }

    cmd->run_option_actions(cmd);   /* 触发选项回调，把值写进 Vpn_Command 字段 */
    ret = cmd->run_command(cmd);    /* 与 `xtools vpn` 同一实现 */

    object_destroy(cmd);
    return ret;
}
REGISTER_TEST_CMD(test_vpn_peer);
