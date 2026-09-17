#ifndef __TUN_H__
#define __TUN_H__

#include <stdint.h>

/*
 * Tun —— 虚拟网卡抽象（平台无关接口，vpn 内部使用，暂不对外）。
 *
 * 定位（见 doc/net/vpn/p2p_vpn_design.md §8 已决取舍）：
 *   Tun 先内聚在 vpn 内部（src/net/vpn/tun/）；出现第二使用者再上提为公共抽象层。
 *
 * 首版实现见 os/unix/Tun.c（Linux /dev/net/tun，L3 模式 IFF_TUN|IFF_NO_PI）。
 * 后续 os/window/Tap.c（Windows TAP/TUN）按同一接口补齐。
 *
 * 典型用法：
 *   Tun *tun = tun_create();
 *   tun->open(tun, "tun0");                 // name 可空(NULL/"")，由内核分配 tunN
 *   tun->set_mtu(tun, tun->mtu);
 *   tun->configure(tun, "10.0.0.1", "255.255.255.0", "10.0.0.0/24");
 *   tun->read(tun, buf, sizeof(buf));       // 出站：本机发往该网卡的 IP 包
 *   tun->write(tun, pkt, n);                // 入站：把对端包注入本机协议栈
 *   tun_destroy(tun);
 *
 * 权限：Linux 下 open/configure/set_mtu 需要 root 或 CAP_NET_ADMIN。
 */
typedef struct Tun_s Tun;

struct Tun_s {
    int  fd;                 /* 设备 fd；-1=未打开 */
    char name[16];           /* 实际设备名（open 后由内核回填，应与 IFNAMSIZ 一致） */
    int  mtu;                /* 建议 MTU（默认与 p2p 单包上限对齐，见 TUN_MTU_DEFAULT） */

    /* 打开设备：name 为空则自动分配；返回 >=0 成功（TRY 落底为 1），负值失败。 */
    int (*open)(Tun *tun, const char *name);
    /* 配置地址（调外部 ip 命令：`ip addr replace <ip/netmask> dev <tun>` + 置 up）。
     * netmask 支持点分("255.255.255.0")或前缀("24")，为空按 24；ip 可自带 "/len"。
     * **只管地址**：路由由调用方按需用 route_add() 逐条加（本模块的路由都来自
     * 运行时交换到的对端网段，没有静态路由配置）。 */
    int (*configure)(Tun *tun, const char *ip, const char *netmask);
    /* 追加/更新一条到 net 的路由（`ip route replace <net> dev <tun>`）。
     * 用于"链路建立后按对端通告的网段自动加路由"；失败返回负值（调用方通常只告警）。 */
    int (*route_add)(Tun *tun, const char *net);
    /* 读一个 IP 包（阻塞，被信号打断自动重试）；返回字节数，负值失败/已关闭。 */
    int (*read)(Tun *tun, uint8_t *buf, int len);
    /* 写一个 IP 包；返回写入字节数，负值失败。入站注入用。 */
    int (*write)(Tun *tun, const uint8_t *buf, int len);
    /* 设置 MTU（内核 ioctl）。 */
    int (*set_mtu)(Tun *tun, int mtu);
    /* 关闭设备（幂等）。 */
    int (*close)(Tun *tun);
};

/* 创建平台默认 TUN 实例（仅分配与绑定方法，不打开设备）；失败返回 NULL。 */
Tun *tun_create(void);

/* 销毁实例：先 close 再释放。tun 可空。 */
void tun_destroy(Tun *tun);

#endif
