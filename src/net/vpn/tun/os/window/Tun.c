/**
 * @file Tun.c
 * @Synopsis  Tun 抽象在 Windows 下的实现（占位）。
 *
 * Windows 没有 /dev/net/tun，对应的虚拟网卡是 TAP-Windows 驱动
 * (\\.Global\{GUID}.tap)，需要额外安装驱动与管理员权限。
 *
 * 当前版本不实现 TAP 驱动，这里只提供与平台无关接口一致的存根：
 *   - tun_create()  正常返回实例，保证调用方拿到的指针始终有效；
 *   - 打开/配置/读写等操作一律返回负值，调用方( vpn )据此报错退出，
 *     而不是链接期缺符号或运行期崩溃。
 *
 * 后续实现 TAP 时，按 os/unix/Tun.c 的结构替换各方法即可。
 *
 * @author Zoo
 * @date 2026-09-14
 */

#if (defined(WINDOWS_USER_MODE))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include "../../Tun.h"

/* 默认 MTU 与 p2p 单包上限(1400)对齐，避免 IP 分片后被 p2p 丢弃。 */
#define TUN_MTU_DEFAULT    1400

static int __tun_open(Tun *tun, const char *name)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_configure(Tun *tun, const char *ip, const char *netmask)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_route_add(Tun *tun, const char *net)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_read(Tun *tun, uint8_t *buf, int len)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_write(Tun *tun, const uint8_t *buf, int len)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_set_mtu(Tun *tun, int mtu)
{
    dbg_str(DBG_ERROR, "tun: TUN/TAP device is not supported on Windows");
    return -1;
}

static int __tun_close(Tun *tun)
{
    if (tun == NULL) {
        return -1;
    }
    tun->fd = -1;
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

#endif /* WINDOWS_USER_MODE */
