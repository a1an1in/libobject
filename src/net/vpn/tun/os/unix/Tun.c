/**
 * @file Tun.c
 * @Synopsis  Tun 抽象在 Linux 下的实现（/dev/net/tun，L3 模式）
 *
 * 首版（见 doc/net/vpn/p2p_vpn_design.md §6）：
 *   - /dev/net/tun + TUNSETIFF(IFF_TUN|IFF_NO_PI)，读到的就是裸 IP 包（L3）；
 *   - 地址/路由配置用外部 ip 命令（简单、可调试）；
 *   - MTU 用 ioctl(SIOCSIFMTU) 直接设置。
 *
 * 权限：open / configure / set_mtu 需要 root 或 CAP_NET_ADMIN。
 *
 * @author Zoo
 * @date 2026-09-14
 */

#ifdef LINUX_USER_MODE   /* 仅 Linux 平台编译；其它平台本文件为空 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <libobject/core/utils/dbg/debug.h>
#include "../../Tun.h"

#define TUN_DEV_PATH       "/dev/net/tun"
/* 默认 MTU 与 p2p 单包上限(1400)对齐，避免 IP 分片后被 p2p 丢弃。 */
#define TUN_MTU_DEFAULT    1400

/* 执行一条外部命令；返回 0 成功。 */
static int __run_cmd(const char *cmd)
{
    int ret = system(cmd);

    if (ret != 0) {
        dbg_str(DBG_ERROR, "tun: command failed (ret=%d): %s", ret, cmd);
        return -1;
    }
    return 0;
}

/* netmask -> 前缀长度：点分("255.255.255.0")或前缀("24")；空=24。 */
static int __netmask_to_prefix(const char *netmask)
{
    struct in_addr addr;
    uint32_t m;
    int prefix = 0;

    if (netmask == NULL || netmask[0] == '\0') {
        return 24;
    }
    if (strchr(netmask, '.') == NULL) {         /* 已是前缀数字 */
        int n = atoi(netmask);
        return (n > 0 && n <= 32) ? n : 24;
    }
    if (inet_aton(netmask, &addr) == 0) {
        return 24;
    }
    m = ntohl(addr.s_addr);
    while (m & 0x80000000u) {
        prefix++;
        m <<= 1;
    }
    return prefix;
}

static int __tun_open(Tun *tun, const char *name)
{
    struct ifreq ifr;
    int fd;

    if (tun == NULL || tun->fd >= 0) {
        return -1;
    }
    fd = open(TUN_DEV_PATH, O_RDWR);
    if (fd < 0) {
        dbg_str(DBG_ERROR, "tun: open %s failed: %s",
                TUN_DEV_PATH, strerror(errno));
        return -1;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;        /* L3 + 无 4 字节包头 */
    if (name != NULL && name[0] != '\0') {
        strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    }
    if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
        dbg_str(DBG_ERROR, "tun: TUNSETIFF on %s failed: %s",
                (name != NULL && name[0] != '\0') ? name : "(auto)",
                strerror(errno));
        close(fd);
        return -1;
    }
    tun->fd = fd;
    snprintf(tun->name, sizeof(tun->name), "%s", ifr.ifr_name);
    dbg_str(DBG_INFO, "tun: opened %s fd=%d", tun->name, tun->fd);
    return 0;
}

/* 加/改一条到 cidr 的路由（ip route replace <cidr> dev <tun>）；失败告警返回负值。 */
static int __tun_route_add(Tun *tun, const char *cidr)
{
    char cmd[256];

    if (tun == NULL || tun->fd < 0 || cidr == NULL || cidr[0] == '\0') {
        return -1;
    }
    snprintf(cmd, sizeof(cmd), "ip route replace %s dev %s", cidr, tun->name);
    if (__run_cmd(cmd) < 0) {
        dbg_str(DBG_INFO, "tun: route %s dev %s not added (may be covered by"
                " connected route)", cidr, tun->name);
        return -1;
    }
    dbg_str(DBG_INFO, "tun: route %s dev %s added", cidr, tun->name);
    return 0;
}

static int __tun_configure(Tun *tun, const char *ip, const char *netmask,
                           const char *route_net)
{
    char cmd[256], ipstr[80];
    int prefix;

    if (tun == NULL || tun->fd < 0 || ip == NULL || ip[0] == '\0') {
        return -1;
    }
    /* 两种写法都支持：
     *   ip 自带前缀("10.0.0.1/24") -> 直接用，忽略 netmask（否则会拼成 10.0.0.1/24/24）；
     *   ip 不带前缀("10.0.0.1")    -> 用 netmask 换算前缀（点分或 24 均可，空=24）。 */
    if (strchr(ip, '/') != NULL) {
        snprintf(ipstr, sizeof(ipstr), "%s", ip);
    } else {
        prefix = __netmask_to_prefix(netmask);
        snprintf(ipstr, sizeof(ipstr), "%s/%d", ip, prefix);
    }

    /* 用 replace 使配置幂等（重复启动/已存在地址不报错）。 */
    snprintf(cmd, sizeof(cmd), "ip addr replace %s dev %s", ipstr, tun->name);
    if (__run_cmd(cmd) < 0) {
        return -1;
    }
    snprintf(cmd, sizeof(cmd), "ip link set dev %s up", tun->name);
    if (__run_cmd(cmd) < 0) {
        return -1;
    }
    /* 静态对端网段路由：两端同网段(如都配 x.x.x.0/24)时已由本机地址的直连路由覆盖，
     * 故失败只告警、不中止（点对点同网段场景无需额外路由）。 */
    if (route_net != NULL && route_net[0] != '\0') {
        __tun_route_add(tun, route_net);
    }
    dbg_str(DBG_INFO, "tun: %s configured ip=%s route=%s",
            tun->name, ipstr,
            (route_net != NULL) ? route_net : "-");
    return 0;
}

static int __tun_read(Tun *tun, uint8_t *buf, int len)
{
    int n;

    if (tun == NULL || tun->fd < 0 || buf == NULL || len <= 0) {
        return -1;
    }
    do {
        n = (int)read(tun->fd, buf, len);
    } while (n < 0 && errno == EINTR);
    return n;
}

static int __tun_write(Tun *tun, const uint8_t *buf, int len)
{
    int n;

    if (tun == NULL || tun->fd < 0 || buf == NULL || len <= 0) {
        return -1;
    }
    do {
        n = (int)write(tun->fd, buf, len);
    } while (n < 0 && errno == EINTR);
    return n;
}

static int __tun_set_mtu(Tun *tun, int mtu)
{
    struct ifreq ifr;
    int s;

    if (tun == NULL || tun->fd < 0 || mtu <= 0) {
        return -1;
    }
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        dbg_str(DBG_ERROR, "tun: socket for set_mtu failed: %s", strerror(errno));
        return -1;
    }
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, tun->name, IFNAMSIZ - 1);
    ifr.ifr_mtu = mtu;
    if (ioctl(s, SIOCSIFMTU, &ifr) < 0) {
        dbg_str(DBG_ERROR, "tun: SIOCSIFMTU %d on %s failed: %s",
                mtu, tun->name, strerror(errno));
        close(s);
        return -1;
    }
    close(s);
    tun->mtu = mtu;
    dbg_str(DBG_INFO, "tun: %s mtu=%d", tun->name, mtu);
    return 0;
}

static int __tun_close(Tun *tun)
{
    if (tun == NULL) {
        return -1;
    }
    if (tun->fd >= 0) {
        close(tun->fd);
        tun->fd = -1;
    }
    return 0;
}

Tun *tun_create(void)
{
    Tun *tun = (Tun *)calloc(1, sizeof(*tun));

    if (tun == NULL) {
        return NULL;
    }
    tun->fd  = -1;
    tun->mtu = TUN_MTU_DEFAULT;

    tun->open      = __tun_open;
    tun->configure = __tun_configure;
    tun->route_add = __tun_route_add;
    tun->read      = __tun_read;
    tun->write     = __tun_write;
    tun->set_mtu   = __tun_set_mtu;
    tun->close     = __tun_close;
    return tun;
}

void tun_destroy(Tun *tun)
{
    if (tun == NULL) {
        return;
    }
    if (tun->fd >= 0) {
        tun->close(tun);
    }
    free(tun);
}

#endif /* LINUX_USER_MODE */
