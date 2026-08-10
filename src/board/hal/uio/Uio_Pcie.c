/**
 * @file Uio_Pcie.c
 * @Synopsis  基于 UIO（uio_pci_generic）的 PCIe 设备用户态驱动。
 * 复用 Uio 的打开/映射/寄存器读写/中断接口，只新增 PCIe 特有功能：
 *   - sysfs 设备发现（按 vendor:device）
 *   - PCI 配置空间读写（/sys/bus/pci/devices/<BDF>/config）
 *   - 绑定 uio_pci_generic 并复用 Uio 映射指定 BAR
 * @author alan lin
 * @version
 * @date 2026-08-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mman.h>
#include <libobject/board/hal/uio/Uio_Pcie.h>
#include <libobject/core/utils/dbg/debug.h>

#define PCIE_SYSFS_PATH     "/sys/bus/pci/devices"
#define PCIE_UIO_DRV_PATH   "/sys/bus/pci/drivers/uio_pci_generic"
#define PCIE_CONFIG_NAME    "config"
#define PCIE_MAX_PATH_LEN   256
#define PCIE_NUM_BARS       6

/* 读取一个 sysfs 十六进制值文件 */
static int __read_sysfs_u64(const char *path, uint64_t *val)
{
    char buf[64] = {0};
    int fd, len;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (len <= 0) {
        return -1;
    }
    buf[len] = '\0';
    *val = strtoull(buf, NULL, 0);
    return 0;
}

static int __read_sysfs_u32(const char *path, uint32_t *val)
{
    uint64_t v;

    if (__read_sysfs_u64(path, &v) < 0) {
        return -1;
    }
    *val = (uint32_t)v;
    return 0;
}

/* 从 /sys/bus/pci/devices/<bdf>/resource 读取各 BAR 的物理基址、大小与 flags */
static int __read_bar_sizes(Uio_Pcie *pcie)
{
    char path[PCIE_MAX_PATH_LEN];
    FILE *fp;
    int i;

    snprintf(path, sizeof(path), "%s/%s/resource", PCIE_SYSFS_PATH, pcie->bdf);
    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    for (i = 0; i < PCIE_NUM_BARS; i++) {
        unsigned long long start = 0, end = 0, flags = 0;
        if (fscanf(fp, "%llx %llx %llx", &start, &end, &flags) != 3) {
            break;
        }
        pcie->bar_addr[i] = (uint64_t)start;
        pcie->bar_flags[i] = (uint32_t)flags;
        pcie->bar_size[i] = (end > start) ? (end - start + 1) : 0;
    }
    fclose(fp);
    return 0;
}

/*
 * 按 BDF 打开 PCIe 设备（如 "0000:01:00.0"）。
 * 只做 sysfs 发现与 BAR 信息读取，不绑定 UIO。
 */
static int __open_bdf(Uio_Pcie *pcie, char *bdf)
{
    char path[PCIE_MAX_PATH_LEN];
    struct stat st;
    int ret = -1;

    TRY {
        THROW_IF(pcie == NULL || bdf == NULL, -1);

        /* 1. 校验 /sys/bus/pci/devices/<bdf> 目录存在 */
        snprintf(path, sizeof(path), "%s/%s", PCIE_SYSFS_PATH, bdf);
        THROW_IF(stat(path, &st) < 0 || !S_ISDIR(st.st_mode), -1);

        /* 2. 记录 BDF */
        if (pcie->bdf == NULL) {
            pcie->bdf = strdup(bdf);
        } else {
            free(pcie->bdf);
            pcie->bdf = strdup(bdf);
        }
        THROW_IF(pcie->bdf == NULL, -1);

        /* 3. 读取 BAR 大小 */
        __read_bar_sizes(pcie);

        dbg_str(DBG_INFO, "pcie open_bdf success, bdf:%s", bdf);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie open_bdf failed, bdf:%s, errno:%d(%s)",
                bdf, errno, strerror(errno));
    }

    return ret;
}

/* 扫描 /sys/bus/pci/devices，按 vendor/device ID 匹配 BDF */
static int __open_device(Uio_Pcie *pcie, uint32_t vendor, uint32_t device)
{
    DIR *dir = NULL;
    struct dirent *ent;
    char path[PCIE_MAX_PATH_LEN];
    uint32_t v, d;
    int ret = -1;
    int opened = 0;

    TRY {
        THROW_IF(pcie == NULL, -1);

        dir = opendir(PCIE_SYSFS_PATH);
        THROW_IF(dir == NULL, -1);

        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.') {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%.200s/vendor", PCIE_SYSFS_PATH,
                     ent->d_name);
            if (__read_sysfs_u32(path, &v) < 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%.200s/device", PCIE_SYSFS_PATH,
                     ent->d_name);
            if (__read_sysfs_u32(path, &d) < 0) {
                continue;
            }
            if (v == vendor && d == device) {
                /* open_bdf 为内部函数（按 BDF 记录设备并读 BAR 信息）。
                 * 成功返回 >= 0（CATCH 会把正常完成置为 1） */
                ret = __open_bdf(pcie, ent->d_name);
                opened = (ret >= 0);
                break;
            }
        }
        closedir(dir);
        dir = NULL;

        /* 未找到或 open_bdf 失败：抛异常，让 CATCH 返回负数（不能用 ret，
         * 因为正常完成时 CATCH 会把 ret 置 1） */
        THROW_IF(!opened, -1);
    } CATCH (ret) {
        if (dir != NULL) {
            closedir(dir);
        }
        dbg_str(DBG_ERROR, "pcie open_device failed, vendor:0x%x, device:0x%x",
                vendor, device);
    }

    return ret;
}

/* 读取 PCI 设备信息 */
static int __get_info(Uio_Pcie *pcie, pcie_dev_info_t *info)
{
    char path[PCIE_MAX_PATH_LEN];
    uint32_t v;
    int i;

    if (pcie == NULL || info == NULL || pcie->bdf == NULL) {
        return -1;
    }

    memset(info, 0, sizeof(*info));

    snprintf(path, sizeof(path), "%s/%s/vendor", PCIE_SYSFS_PATH, pcie->bdf);
    __read_sysfs_u32(path, &info->vendor);
    snprintf(path, sizeof(path), "%s/%s/device", PCIE_SYSFS_PATH, pcie->bdf);
    __read_sysfs_u32(path, &info->device);
    snprintf(path, sizeof(path), "%s/%s/class", PCIE_SYSFS_PATH, pcie->bdf);
    __read_sysfs_u32(path, &info->class);
    snprintf(path, sizeof(path), "%s/%s/revision", PCIE_SYSFS_PATH, pcie->bdf);
    __read_sysfs_u32(path, &info->revision);
    snprintf(path, sizeof(path), "%s/%s/irq", PCIE_SYSFS_PATH, pcie->bdf);
    if (__read_sysfs_u32(path, &v) == 0) {
        info->irq = (int)v;
    } else {
        info->irq = -1;
    }

    info->num_bars = 0;
    for (i = 0; i < PCIE_NUM_BARS; i++) {
        info->bar_size[i] = pcie->bar_size[i];
        if (info->bar_size[i] != 0) {
            info->num_bars++;
        }
    }

    return 0;
}

/* 读 PCI 配置空间：从 offset 读 4 字节 */
static int __read_config(Uio_Pcie *pcie, int offset, uint32_t *data)
{
    char path[PCIE_MAX_PATH_LEN];
    int fd, ret = -1;
    ssize_t n;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL || data == NULL, -1);
        THROW_IF(offset < 0 || offset + 4 > 4096, -1);

        snprintf(path, sizeof(path), "%s/%s/%s", PCIE_SYSFS_PATH,
                 pcie->bdf, PCIE_CONFIG_NAME);
        fd = open(path, O_RDONLY);
        THROW_IF(fd < 0, -1);

        n = pread(fd, data, sizeof(*data), offset);
        close(fd);
        THROW_IF(n != (ssize_t)sizeof(*data), -1);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie read_config failed, bdf:%s, off:0x%x, "
                "errno:%d(%s)",
                pcie ? (pcie->bdf ? pcie->bdf : "?") : "?", offset,
                errno, strerror(errno));
    }

    return ret;
}

/* 写 PCI 配置空间：向 offset 写 4 字节 */
static int __write_config(Uio_Pcie *pcie, int offset, uint32_t data)
{
    char path[PCIE_MAX_PATH_LEN];
    int fd, ret = -1;
    ssize_t n;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL, -1);
        THROW_IF(offset < 0 || offset + 4 > 4096, -1);

        snprintf(path, sizeof(path), "%s/%s/%s", PCIE_SYSFS_PATH,
                 pcie->bdf, PCIE_CONFIG_NAME);
        fd = open(path, O_WRONLY);
        THROW_IF(fd < 0, -1);

        n = pwrite(fd, &data, sizeof(data), offset);
        close(fd);
        THROW_IF(n != (ssize_t)sizeof(data), -1);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie write_config failed, bdf:%s, off:0x%x, "
                "errno:%d(%s)",
                pcie ? (pcie->bdf ? pcie->bdf : "?") : "?", offset,
                errno, strerror(errno));
    }

    return ret;
}

/* 从 /sys/bus/pci/devices/<bdf>/uio/uioX 获取已绑定的 UIO 编号 */
static int __find_uio_num(Uio_Pcie *pcie)
{
    char path[PCIE_MAX_PATH_LEN];
    DIR *dir;
    struct dirent *ent;
    int num = -1;

    snprintf(path, sizeof(path), "%s/%s/uio", PCIE_SYSFS_PATH, pcie->bdf);
    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "uio", 3) == 0) {
            num = atoi(ent->d_name + 3);
            break;
        }
    }
    closedir(dir);
    return num;
}

/* 把设备绑定到 uio_pci_generic。
 * uio_pci_generic 的 id_table 为空，直接 bind 会因 driver_match 失败返回 ENODEV，
 * 必须先写 new_id 把 vendor/device 加入驱动 id 表（new_id 会触发匹配设备自动绑定）。
 */
static int __bind_to_uio(Uio_Pcie *pcie)
{
    char path[PCIE_MAX_PATH_LEN];
    char id[64];
    uint32_t vendor = 0, device = 0;
    int fd = -1, ret = -1;
    ssize_t n;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL, -1);

        /* 1. 读 vendor/device，写入 new_id 加入驱动 id 表 */
        snprintf(path, sizeof(path), "%s/%s/vendor", PCIE_SYSFS_PATH, pcie->bdf);
        __read_sysfs_u32(path, &vendor);
        snprintf(path, sizeof(path), "%s/%s/device", PCIE_SYSFS_PATH, pcie->bdf);
        __read_sysfs_u32(path, &device);

        snprintf(path, sizeof(path), "%s/new_id", PCIE_UIO_DRV_PATH);
        fd = open(path, O_WRONLY);
        THROW_IF(fd < 0, -1);
        snprintf(id, sizeof(id), "%x %x", vendor, device);
        n = write(fd, id, strlen(id));
        close(fd);
        fd = -1;
        THROW_IF(n != (ssize_t)strlen(id), -1);

        /* 2. new_id 通常已触发自动绑定；若仍未绑定，显式 bind。
         *    已绑定（自动绑定）时 bind 会返回 ENODEV，属正常，忽略 */
        snprintf(path, sizeof(path), "%s/bind", PCIE_UIO_DRV_PATH);
        fd = open(path, O_WRONLY);
        if (fd >= 0) {
            n = write(fd, pcie->bdf, strlen(pcie->bdf));
            close(fd);
            fd = -1;
        }

        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie bind_to_uio failed, bdf:%s, errno:%d(%s)",
                pcie ? (pcie->bdf ? pcie->bdf : "?") : "?", errno,
                strerror(errno));
    } FINALLY {
        if (fd >= 0) {
            close(fd);
        }
    }

    return ret;
}

/*
 * 若 /dev/uioX 节点不存在（如 busybox 无 devtmpfs），从 /sys/class/uio/uioX/dev
 * 读取 major:minor 并 mknod 创建。
 */
static int __ensure_uio_devnode(Uio_Pcie *pcie)
{
    char sys_path[PCIE_MAX_PATH_LEN];
    char dev_path[PCIE_MAX_PATH_LEN];
    char buf[32] = {0};
    int fd, len, major = 0, minor = 0;

    snprintf(dev_path, sizeof(dev_path), "/dev/uio%d", pcie->uio_num);
    if (access(dev_path, F_OK) == 0) {
        return 0;
    }

    snprintf(sys_path, sizeof(sys_path), "/sys/class/uio/uio%d/dev", pcie->uio_num);
    fd = open(sys_path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (len <= 0 || sscanf(buf, "%d:%d", &major, &minor) != 2) {
        return -1;
    }

    if (mknod(dev_path, S_IFCHR | 0600, makedev(major, minor)) < 0) {
        return -1;
    }
    dbg_str(DBG_INFO, "uio devnode created: %s (%d:%d)", dev_path, major, minor);
    return 0;
}

/*
 * 映射 BAR（多 BAR 并存）：在 arm64（Linux 4.9）上：
 *   - uio_pci_generic 不设置 info.mem[] → 无 /sys/class/uio/uioX/maps/mapN/，Uio.mmap 不可用；
 *   - arm64 未定义 HAVE_PCI_MMAP → 无 /sys/bus/pci/devices/<BDF>/resourceN，也不能直接 mmap。
 * 因此从 /sys/bus/pci/devices/<BDF>/resource（文本文件，始终存在）拿到 BAR 物理基址，
 * 然后 mmap /dev/mem（CONFIG_DEVMEM=y，CONFIG_STRICT_DEVMEM 未开，映射自由）。
 * 每个 BAR 独立映射到 pcie->bar_base[bar]（访问基址）/pcie->map_base[bar]（页对齐，
 * munmap 用），互不覆盖，支持多 BAR 同时访问。寄存器访问地址 = (bar<<bar_shift)|off。
 */
static int __map_bar(Uio_Pcie *pcie, int bar)
{
    Uio *uio = (Uio *)pcie;
    const char *mem_path = "/dev/mem";
    uint64_t phys, page, page_off, map_base, map_size;
    int mem_fd = -1, ret = -1;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL, -1);
        THROW_IF(bar < 0 || bar >= PCIE_NUM_BARS, -1);
        THROW_IF(pcie->bar_size[bar] == 0, -1);
        /* I/O BAR 不能通过 /dev/mem 映射 */
        THROW_IF(pcie->bar_flags[bar] & 0x1, -1);

        /* 1. 打开 /dev/mem（无 devtmpfs 时先 mknod c 1:1 创建） */
        mem_fd = open(mem_path, O_RDWR | O_SYNC);
        if (mem_fd < 0 && errno == ENOENT) {
            if (mknod(mem_path, S_IFCHR | 0600, makedev(1, 1)) == 0) {
                mem_fd = open(mem_path, O_RDWR | O_SYNC);
            }
        }
        THROW_IF(mem_fd < 0, -1);

        /* 2. 确保 bar_shift 能覆盖该 BAR 大小（多 BAR 取最大，地址编码一致） */
        while (pcie->bar_shift < 40 &&
               (1ULL << pcie->bar_shift) < pcie->bar_size[bar]) {
            pcie->bar_shift++;
        }

        /* 3. BAR 物理基址页对齐，计算映射偏移与大小 */
        page = getpagesize();
        phys = pcie->bar_addr[bar];
        page_off = phys & (page - 1);
        map_base = phys & ~(page - 1);
        map_size = (pcie->bar_size[bar] + page_off + page - 1) & ~(page - 1);

        /* 4. 该 BAR 已映射则先解除（其它 BAR 不受影响，支持多 BAR 并存） */
        if (pcie->map_base[bar] != NULL && pcie->map_base[bar] != MAP_FAILED) {
            munmap(pcie->map_base[bar], pcie->bar_mapped_size[bar]);
            pcie->map_base[bar] = NULL;
            pcie->bar_base[bar] = NULL;
        }

        /* 5. mmap /dev/mem：offset = 页对齐物理地址，基址加回页内偏移 */
        pcie->map_base[bar] = mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, mem_fd, (off_t)map_base);
        THROW_IF(pcie->map_base[bar] == MAP_FAILED, -1);
        pcie->bar_base[bar] = pcie->map_base[bar] + page_off;
        pcie->bar_mapped_size[bar] = map_size;
        pcie->bar = bar;
        uio->base = pcie->bar_base[bar];
        uio->size = map_size;
        dbg_str(DBG_INFO, "pcie map_bar success, bdf:%s, bar:%d, phys:0x%llx, "
                "size:0x%llx, base:%p, shift:%d",
                pcie->bdf, bar, (unsigned long long)phys,
                (unsigned long long)map_size, pcie->bar_base[bar],
                pcie->bar_shift);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie map_bar failed, bar:%d, errno:%d(%s)",
                bar, errno, strerror(errno));
        if (pcie->map_base[bar] != NULL && pcie->map_base[bar] != MAP_FAILED) {
            munmap(pcie->map_base[bar], pcie->bar_mapped_size[bar]);
            pcie->map_base[bar] = NULL;
            pcie->bar_base[bar] = NULL;
        }
        uio->base = NULL;
    } FINALLY {
        if (mem_fd >= 0) {
            close(mem_fd);
        }
    }

    return ret;
}

/*
 * 绑定 uio_pci_generic 并打开 /dev/uioX，然后通过 /dev/mem 按物理地址映射指定 BAR。
 * 需要 root 权限以及内核 CONFIG_UIO_PCI_GENERIC + CONFIG_DEVMEM。
 * 之后可通过继承的 read/write_register、enable/disable_irq、register_irq 访问。
 */
static int __bind_uio(Uio_Pcie *pcie, int bar)
{
    Uio *uio = (Uio *)pcie;
    char dev_path[PCIE_MAX_PATH_LEN];
    int ret = -1;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL, -1);
        THROW_IF(bar < 0 || bar >= PCIE_NUM_BARS, -1);

        /* 1. 查找已绑定的 uio 编号；未绑定则尝试绑定 uio_pci_generic */
        pcie->uio_num = __find_uio_num(pcie);
        if (pcie->uio_num < 0) {
            ret = __bind_to_uio(pcie);
            THROW_IF(ret < 0, -1);
            pcie->uio_num = __find_uio_num(pcie);
            THROW_IF(pcie->uio_num < 0, -1);
        }

        /* 2. 确保 /dev/uioX 节点存在（无 devtmpfs 时 mknod） */
        THROW_IF(__ensure_uio_devnode(pcie) < 0, -1);

        /* 3. 打开 /dev/uioX（仅首次；多 BAR 重复 bind 复用同一 fd 用于中断） */
        snprintf(dev_path, sizeof(dev_path), "/dev/uio%d", pcie->uio_num);
        if (uio->fd < 0) {
            ret = uio->open(uio, dev_path);
            THROW_IF(ret < 0, -1);
        }

        /* 4. 通过 /dev/mem 按物理地址映射指定 BAR（可多次 bind 映射多个 BAR 并存），
         *    之后寄存器访问用偏移编码：read_register((bar<<bar_shift)|off) */
        ret = __map_bar(pcie, bar);
        THROW_IF(ret < 0, -1);

        dbg_str(DBG_INFO, "pcie bind_uio success, bdf:%s, dev:%s, fd:%d, bar:%d",
                pcie->bdf, dev_path, uio->fd, pcie->bar);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "pcie bind_uio failed, bdf:%s, errno:%d(%s)",
                pcie ? (pcie->bdf ? pcie->bdf : "?") : "?", errno,
                strerror(errno));
        if (uio->fd >= 0) {
            close(uio->fd);
            uio->fd = -1;
        }
        if (uio->dev_path != NULL) {
            free(uio->dev_path);
            uio->dev_path = NULL;
        }
    }

    return ret;
}

/* 把编码地址拆成 BAR 序号 + BAR 内偏移（高位 BAR、低位偏移） */
static int __decode_addr(Uio_Pcie *pcie, uint64_t addr, int *bar, uint64_t *off)
{
    uint64_t mask;

    if (pcie == NULL) {
        return -1;
    }
    mask = (1ULL << pcie->bar_shift) - 1;
    *bar = (int)(addr >> pcie->bar_shift);
    *off = addr & mask;

    if (*bar < 0 || *bar >= PCIE_NUM_BARS || pcie->bar_base[*bar] == NULL) {
        dbg_str(DBG_ERROR, "pcie register addr decode failed, addr:0x%llx, "
                "bar:%d (shift:%d)",
                (unsigned long long)addr, *bar, pcie->bar_shift);
        return -1;
    }
    if (*off >= pcie->bar_size[*bar]) {
        dbg_str(DBG_ERROR, "pcie register offset out of range, bar:%d, off:0x%llx, "
                "size:0x%llx",
                *bar, (unsigned long long)*off,
                (unsigned long long)pcie->bar_size[*bar]);
        return -1;
    }
    return 0;
}

/* 读寄存器：offset 高位是 BAR 序号、低位是 BAR 内偏移（width 32/64） */
static int __read_register(Uio_Pcie *pcie, uint64_t offset, uint64_t *data)
{
    Uio *uio = (Uio *)pcie;
    uint8_t volatile *base;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    base = (uint8_t volatile *)pcie->bar_base[bar] + off;
    if (uio->width == 64) {
        *data = *(uint64_t volatile *)base;
    } else {
        *data = *(uint32_t volatile *)base;
    }
    return 0;
}

/* 写寄存器：offset 高位是 BAR 序号、低位是 BAR 内偏移（width 32/64） */
static int __write_register(Uio_Pcie *pcie, uint64_t offset, uint64_t data)
{
    Uio *uio = (Uio *)pcie;
    uint8_t volatile *base;
    int bar;
    uint64_t off;

    if (pcie == NULL || __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    base = (uint8_t volatile *)pcie->bar_base[bar] + off;
    if (uio->width == 64) {
        *(uint64_t volatile *)base = data;
    } else {
        *(uint32_t volatile *)base = (uint32_t)data;
    }
    return 0;
}

/* 批量读寄存器（len 个），offset 高位是 BAR 序号，返回实际读取个数 */
static int __read_registers(Uio_Pcie *pcie, uint64_t offset, uint64_t *data,
                            uint32_t len)
{
    Uio *uio = (Uio *)pcie;
    uint8_t volatile *base;
    uint32_t i, count, reg_size;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    reg_size = (uio->width == 64) ? 8 : 4;
    count = len;
    if (off + count * reg_size > pcie->bar_size[bar]) {
        count = (pcie->bar_size[bar] - off) / reg_size;
    }
    base = (uint8_t volatile *)pcie->bar_base[bar] + off;
    for (i = 0; i < count; i++) {
        if (uio->width == 64) {
            data[i] = *(uint64_t volatile *)(base + i * 8);
        } else {
            data[i] = *(uint32_t volatile *)(base + i * 4);
        }
    }
    return (int)count;
}

/* 批量写寄存器（len 个），offset 高位是 BAR 序号，返回实际写入个数 */
static int __write_registers(Uio_Pcie *pcie, uint64_t offset, uint64_t *data,
                             uint32_t len)
{
    Uio *uio = (Uio *)pcie;
    uint8_t volatile *base;
    uint32_t i, count, reg_size;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    reg_size = (uio->width == 64) ? 8 : 4;
    count = len;
    if (off + count * reg_size > pcie->bar_size[bar]) {
        count = (pcie->bar_size[bar] - off) / reg_size;
    }
    base = (uint8_t volatile *)pcie->bar_base[bar] + off;
    for (i = 0; i < count; i++) {
        if (uio->width == 64) {
            *(uint64_t volatile *)(base + i * 8) = data[i];
        } else {
            *(uint32_t volatile *)(base + i * 4) = (uint32_t)data[i];
        }
    }
    return (int)count;
}

/* 寄存器访问地址空间大小（低 bar_shift 位） */
static int __get_size(Uio_Pcie *pcie)
{
    if (pcie == NULL) {
        return -1;
    }
    return (int)(1ULL << pcie->bar_shift);
}

static int __construct(Uio_Pcie *module, char *init_str)
{
    int i;

    module->bdf = NULL;
    module->bar = -1;
    module->bar_shift = 12;
    module->uio_num = -1;
    for (i = 0; i < PCIE_NUM_BARS; i++) {
        module->bar_base[i] = NULL;
        module->map_base[i] = NULL;
        module->bar_mapped_size[i] = 0;
    }
    memset(module->bar_size, 0, sizeof(module->bar_size));
    memset(module->bar_addr, 0, sizeof(module->bar_addr));
    memset(module->bar_flags, 0, sizeof(module->bar_flags));
    return 0;
}

static int __deconstruct(Uio_Pcie *module)
{
    Uio *uio = (Uio *)module;
    int i;

    /* 解除所有 BAR 的 /dev/mem 映射（页对齐 map_base 用于 munmap），再交给 Uio.close */
    for (i = 0; i < PCIE_NUM_BARS; i++) {
        if (module->map_base[i] != NULL && module->map_base[i] != MAP_FAILED) {
            munmap(module->map_base[i], module->bar_mapped_size[i]);
            module->map_base[i] = NULL;
            module->bar_base[i] = NULL;
        }
    }
    uio->base = NULL;

    if (uio->fd >= 0 || uio->base != NULL) {
        uio->close(uio);
    }

    if (module->bdf != NULL) {
        free(module->bdf);
        module->bdf = NULL;
    }
    return 0;
}

/*
 * Uio_Pcie 注册接口。
 * read/write_register(s)、get_size 由 Uio_Pcie 覆盖（支持多 BAR：地址高位是 BAR 序号）；
 * set_width、enable/disable_irq、register_irq、open/close 继承 Uio（value 为 NULL）。
 */
DEFINE_CLASS(
    EXTENDS(Uio_Pcie, Uio),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open_device, __open_device),
    Class_VFunc_Entry(get_info, __get_info),
    Class_VFunc_Entry(read_config, __read_config),
    Class_VFunc_Entry(write_config, __write_config),
    Class_VFunc_Entry(bind_uio, __bind_uio),
    Class_VFunc_Entry(get_size, __get_size),
    Class_VFunc_Entry(set_width, NULL),
    Class_VFunc_Entry(read_register, __read_register),
    Class_VFunc_Entry(write_register, __write_register),
    Class_VFunc_Entry(read_registers, __read_registers),
    Class_VFunc_Entry(write_registers, __write_registers),
    Class_VFunc_Entry(enable_irq, NULL),
    Class_VFunc_Entry(disable_irq, NULL),
    Class_VFunc_Entry(register_irq, NULL)
);
