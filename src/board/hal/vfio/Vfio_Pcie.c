/**
 * @file Vfio_Pcie.c
 * @Synopsis 基于 VFIO 的 PCIe 设备用户态驱动。
 * 继承 Vfio 的通用机制（三层 fd、region mmap、eventfd 中断、DMA 映射），
 * 只新增 PCIe 特有功能：
 *   - open_device：按 vendor/device ID 在 /sys/bus/pci/devices 下发现设备，
 *     读取 iommu_group → /dev/vfio/<N>，绑定 vfio-pci，调用父类 open。
 *   - map_bar：映射指定 BAR（= region 索引）。
 *   - 寄存器访问：偏移编码（高位 region/BAR、低位偏移），多 BAR 并存。
 * @author alan lin
 * @version
 * @date 2026-08-11
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
#include <sys/mman.h>
#include <libobject/board/hal/vfio/Vfio_Pcie.h>
#include <libobject/core/utils/dbg/debug.h>

#define PCIE_SYSFS_PATH       "/sys/bus/pci/devices"
#define PCIE_VFIO_DRV_PATH    "/sys/bus/pci/drivers/vfio-pci"
#define PCIE_UIO_DRV_PATH     "/sys/bus/pci/drivers/uio_pci_generic"
#define PCIE_MAX_PATH_LEN     256
#define PCIE_NUM_BARS         6

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

/* 从 /sys/bus/pci/devices/<bdf>/resource 读取各 BAR 大小（文本文件始终存在） */
static int __read_bar_sizes(Vfio_Pcie *pcie)
{
    char path[PCIE_MAX_PATH_LEN];
    FILE *fp;
    int i;

    snprintf(path, sizeof(path), "%s/%s/resource", PCIE_SYSFS_PATH, pcie->bdf);
    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    memset(pcie->bar_size, 0, sizeof(pcie->bar_size));
    for (i = 0; i < PCIE_NUM_BARS; i++) {
        unsigned long long start = 0, end = 0, flags = 0;
        if (fscanf(fp, "%llx %llx %llx", &start, &end, &flags) != 3) {
            break;
        }
        pcie->bar_size[i] = (end > start) ? (end - start + 1) : 0;
    }
    fclose(fp);
    return 0;
}

/*
 * 绑定设备到 vfio-pci。
 * 先把设备从 uio_pci_generic 解绑（若已绑定），再写 vfio-pci 的 new_id 加入驱动
 * id 表（会触发自动绑定）；若仍未绑定则显式 bind。
 */
static int __bind_to_vfio(Vfio_Pcie *pcie)
{
    char path[PCIE_MAX_PATH_LEN];
    char id[64];
    uint32_t vendor = 0, device = 0;
    int fd = -1, ret = -1;
    ssize_t n;

    TRY {
        THROW_IF(pcie == NULL || pcie->bdf == NULL, -1);

        /* 1. 从 uio_pci_generic 解绑（若已绑定，unbind 不存在时忽略） */
        snprintf(path, sizeof(path), "%s/unbind", PCIE_UIO_DRV_PATH);
        fd = open(path, O_WRONLY);
        if (fd >= 0) {
            (void)write(fd, pcie->bdf, strlen(pcie->bdf));
            close(fd);
            fd = -1;
        }

        /* 2. 读 vendor/device，写入 vfio-pci new_id 加入驱动 id 表 */
        snprintf(path, sizeof(path), "%s/%s/vendor", PCIE_SYSFS_PATH,
                 pcie->bdf);
        __read_sysfs_u32(path, &vendor);
        snprintf(path, sizeof(path), "%s/%s/device", PCIE_SYSFS_PATH,
                 pcie->bdf);
        __read_sysfs_u32(path, &device);

        snprintf(path, sizeof(path), "%s/new_id", PCIE_VFIO_DRV_PATH);
        EXEC(fd = open(path, O_WRONLY));
        snprintf(id, sizeof(id), "%x %x", vendor, device);
        n = write(fd, id, strlen(id));
        close(fd);
        fd = -1;
        THROW_IF(n != (ssize_t)strlen(id), -1);

        /* 3. new_id 通常已触发自动绑定；若仍未绑定，显式 bind。
         *    已绑定（自动绑定）时 bind 会返回 ENODEV，属正常，忽略 */
        snprintf(path, sizeof(path), "%s/bind", PCIE_VFIO_DRV_PATH);
        fd = open(path, O_WRONLY);
        if (fd >= 0) {
            n = write(fd, pcie->bdf, strlen(pcie->bdf));
            close(fd);
            fd = -1;
        }

        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio_pcie bind_to_vfio failed, bdf:%s, "
                "errno:%d(%s)",
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
 * 解析设备的 iommu_group，得到 /dev/vfio/<N> 路径。
 * iommu_group 必须是绝对路径 symlink（如 /sys/kernel/iommu_groups/12），
 * 取最后一段数字 N → /dev/vfio/12。
 */
static int __get_group_path(Vfio_Pcie *pcie, char *group_path, int len)
{
    char link_path[PCIE_MAX_PATH_LEN];
    char buf[PCIE_MAX_PATH_LEN] = {0};
    ssize_t n;
    const char *p;
    int num;

    snprintf(link_path, sizeof(link_path), "%s/%s/iommu_group",
             PCIE_SYSFS_PATH, pcie->bdf);
    n = readlink(link_path, buf, sizeof(buf) - 1);
    if (n <= 0) {
        dbg_str(DBG_ERROR, "vfio_pcie readlink iommu_group failed, "
                "bdf:%s (设备未落在 iommu_group，需 -M virt,iommu=smmuv3)",
                pcie->bdf);
        return -1;
    }
    buf[n] = '\0';

    /* 取路径最后一段（数字） */
    p = strrchr(buf, '/');
    if (p == NULL) {
        p = buf;
    } else {
        p++;
    }
    num = atoi(p);
    snprintf(group_path, len, "/dev/vfio/%d", num);
    return 0;
}

/*
 * 按 vendor/device ID 扫描 /sys/bus/pci/devices 发现设备，然后：
 *   - 解析 iommu_group → /dev/vfio/<N>
 *   - 绑定 vfio-pci
 *   - 读取 BAR 大小
 *   - 调用父类 Vfio.open(group_path, device_name)（三层 fd）
 */
static int __open_device(Vfio_Pcie *pcie, uint32_t vendor, uint32_t device)
{
    Vfio *vfio = (Vfio *)pcie;
    DIR *dir = NULL;
    struct dirent *ent;
    char path[PCIE_MAX_PATH_LEN];
    char group_path[PCIE_MAX_PATH_LEN];
    uint32_t v, d;
    int ret = -1;
    int opened = 0;

    TRY {
        THROW_IF(pcie == NULL, -1);
        THROW_IF(vfio->device_fd >= 0, -1); /* 已打开 */

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
                /* 记录 BDF */
                if (pcie->bdf == NULL) {
                    pcie->bdf = strdup(ent->d_name);
                } else {
                    free(pcie->bdf);
                    pcie->bdf = strdup(ent->d_name);
                }
                THROW_IF(pcie->bdf == NULL, -1);

                /* 解析 iommu_group → /dev/vfio/<N> */
                THROW_IF(__get_group_path(pcie, group_path,
                                          sizeof(group_path)) < 0, -1);

                /* 绑定 vfio-pci */
                EXEC(__bind_to_vfio(pcie));

                /* 读取 BAR 大小（region size 用 VFIO_DEVICE_GET_REGION_INFO，
                 * 此处仅用于 bar_shift 估算） */
                __read_bar_sizes(pcie);

                /* 调用父类 open（三层 fd） */
                ret = vfio->open(vfio, group_path, ent->d_name);
                opened = (ret >= 0);
                break;
            }
        }
        if (dir != NULL) {
            closedir(dir);
            dir = NULL;
        }

        /* 未找到或 open 失败：抛异常，让 CATCH 返回负数（不能用 ret，
         * 因为正常完成时 CATCH 会把 ret 置 1） */
        THROW_IF(!opened, -1);
    } CATCH (ret) {
        if (dir != NULL) {
            closedir(dir);
        }
        dbg_str(DBG_ERROR, "vfio_pcie open_device failed, vendor:0x%x, "
                "device:0x%x", vendor, device);
    }

    return ret;
}

/*
 * 映射指定 BAR（= region 索引），多 BAR 并存。
 * 映射后可通过 read/write_register(vfio_pcie_bar_addr(bar, off)) 访问。
 */
static int __map_bar(Vfio_Pcie *pcie, int bar)
{
    Vfio *vfio = (Vfio *)pcie;
    vfio_region_info_t info;
    int i, ret = -1;

    TRY {
        THROW_IF(pcie == NULL || vfio->device_fd < 0, -1);
        THROW_IF(bar < 0 || bar >= PCIE_NUM_BARS, -1);

        /* 1. 查 region（BAR）信息，确认可 mmap */
        ret = vfio->get_region_info(vfio, bar, &info);
        THROW_IF(ret < 0 || info.size == 0, -1);

        /* 2. 确保 bar_shift 能覆盖该 BAR 大小（多 BAR 取最大，地址编码一致） */
        while (pcie->bar_shift < 40 &&
               (1ULL << pcie->bar_shift) < info.size) {
            pcie->bar_shift++;
        }

        /* 3. 映射 region（父类 Vfio.map_region，多 region 并存） */
        EXEC(vfio->map_region(vfio, bar));
        pcie->bar_size[bar] = info.size;
        pcie->bar_mapped[bar] = 1;

        /* 4. bar_shift 由所有已映射 BAR 取最大，保证编码一致 */
        for (i = 0; i < PCIE_NUM_BARS; i++) {
            if (pcie->bar_mapped[i]) {
                while (pcie->bar_shift < 40 &&
                       (1ULL << pcie->bar_shift) < pcie->bar_size[i]) {
                    pcie->bar_shift++;
                }
            }
        }

        dbg_str(DBG_INFO, "vfio_pcie map_bar success, bdf:%s, bar:%d, "
                "size:0x%llx, base:%p, shift:%d",
                pcie->bdf, bar, (unsigned long long)info.size,
                vfio->region_base[bar], pcie->bar_shift);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio_pcie map_bar failed, bar:%d, errno:%d(%s)",
                bar, errno, strerror(errno));
    }

    return ret;
}

/* 把编码地址拆成 BAR 序号 + BAR 内偏移（高位 BAR、低位偏移） */
static int __decode_addr(Vfio_Pcie *pcie, uint64_t addr, int *bar, uint64_t *off)
{
    Vfio *vfio = (Vfio *)pcie;
    uint64_t mask;

    if (pcie == NULL) {
        return -1;
    }
    mask = (1ULL << pcie->bar_shift) - 1;
    *bar = (int)(addr >> pcie->bar_shift);
    *off = addr & mask;

    if (*bar < 0 || *bar >= PCIE_NUM_BARS ||
        vfio->region_base[*bar] == NULL) {
        dbg_str(DBG_ERROR, "vfio_pcie register addr decode failed, addr:0x%llx, "
                "bar:%d (shift:%d)",
                (unsigned long long)addr, *bar, pcie->bar_shift);
        return -1;
    }
    if (*off >= pcie->bar_size[*bar]) {
        dbg_str(DBG_ERROR, "vfio_pcie register offset out of range, bar:%d, "
                "off:0x%llx, size:0x%llx",
                *bar, (unsigned long long)*off,
                (unsigned long long)pcie->bar_size[*bar]);
        return -1;
    }
    return 0;
}

static int __set_width(Vfio_Pcie *pcie, int width)
{
    if (pcie == NULL) {
        return -1;
    }
    if (width != 32 && width != 64) {
        dbg_str(DBG_ERROR, "unsupported register width:%d, only 32 or 64",
                width);
        return -1;
    }
    pcie->width = width;
    return 0;
}

/* 读寄存器：offset 高位是 BAR 序号、低位是 BAR 内偏移（width 32/64） */
static int __read_register(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data)
{
    Vfio *vfio = (Vfio *)pcie;
    uint8_t volatile *base;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    base = (uint8_t volatile *)vfio->region_base[bar] + off;
    if (pcie->width == 64) {
        *data = *(uint64_t volatile *)base;
    } else {
        *data = *(uint32_t volatile *)base;
    }
    return 0;
}

/* 写寄存器：offset 高位是 BAR 序号、低位是 BAR 内偏移（width 32/64） */
static int __write_register(Vfio_Pcie *pcie, uint64_t offset, uint64_t data)
{
    Vfio *vfio = (Vfio *)pcie;
    uint8_t volatile *base;
    int bar;
    uint64_t off;

    if (pcie == NULL || __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    base = (uint8_t volatile *)vfio->region_base[bar] + off;
    if (pcie->width == 64) {
        *(uint64_t volatile *)base = data;
    } else {
        *(uint32_t volatile *)base = (uint32_t)data;
    }
    return 0;
}

/* 批量读寄存器（len 个），offset 高位是 BAR 序号，返回实际读取个数 */
static int __read_registers(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data,
                            uint32_t len)
{
    Vfio *vfio = (Vfio *)pcie;
    uint8_t volatile *base;
    uint32_t i, count, reg_size;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    reg_size = (pcie->width == 64) ? 8 : 4;
    count = len;
    if (off + count * reg_size > pcie->bar_size[bar]) {
        count = (pcie->bar_size[bar] - off) / reg_size;
    }
    base = (uint8_t volatile *)vfio->region_base[bar] + off;
    for (i = 0; i < count; i++) {
        if (pcie->width == 64) {
            data[i] = *(uint64_t volatile *)(base + i * 8);
        } else {
            data[i] = *(uint32_t volatile *)(base + i * 4);
        }
    }
    return (int)count;
}

/* 批量写寄存器（len 个），offset 高位是 BAR 序号，返回实际写入个数 */
static int __write_registers(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data,
                             uint32_t len)
{
    Vfio *vfio = (Vfio *)pcie;
    uint8_t volatile *base;
    uint32_t i, count, reg_size;
    int bar;
    uint64_t off;

    if (pcie == NULL || data == NULL ||
        __decode_addr(pcie, offset, &bar, &off) < 0) {
        return -1;
    }
    reg_size = (pcie->width == 64) ? 8 : 4;
    count = len;
    if (off + count * reg_size > pcie->bar_size[bar]) {
        count = (pcie->bar_size[bar] - off) / reg_size;
    }
    base = (uint8_t volatile *)vfio->region_base[bar] + off;
    for (i = 0; i < count; i++) {
        if (pcie->width == 64) {
            *(uint64_t volatile *)(base + i * 8) = data[i];
        } else {
            *(uint32_t volatile *)(base + i * 4) = (uint32_t)data[i];
        }
    }
    return (int)count;
}

/*
 * dma_config 通用实现：入参为用户缓冲，确保 src/dst 已映射为 IOVA 并记录配置，
 * 不触发。可复用：同一配置（地址+长度不变）反复 dma_run 不重映射；
 * 若地址/长度变化，先解除旧的映射再重新映射（避免 IOVA 泄漏）。
 * 设备相关（如何写寄存器、如何触发）由具体设备类的 dma_run override 负责。
 */
static int __dma_config(Vfio_Pcie *pcie, void *buf_src, void *buf_dst,
                        uint32_t len, int direction)
{
    Vfio *vfio = (Vfio *)pcie;

    if (pcie == NULL || buf_src == NULL || buf_dst == NULL || len == 0) {
        return -1;
    }

    /* 配置相同（地址+长度不变）：复用现有 IOVA，仅更新方向，不重新映射 */
    if (pcie->dma_src_va == buf_src && pcie->dma_dst_va == buf_dst &&
        pcie->dma_len == len) {
        pcie->dma_dir = direction;
        return 0;
    }

    /* 配置变化：先解除旧的映射（若存在），再重新映射 */
    if (pcie->dma_src_va != NULL) {
        vfio->dma_unmap(vfio, pcie->dma_src, (uint64_t)pcie->dma_len);
        pcie->dma_src_va = NULL;
    }
    if (pcie->dma_dst_va != NULL) {
        vfio->dma_unmap(vfio, pcie->dma_dst, (uint64_t)pcie->dma_len);
        pcie->dma_dst_va = NULL;
    }
    pcie->dma_len = 0;

    if (vfio->dma_map(vfio, buf_src, (uint64_t)len, &pcie->dma_src) < 0) {
        return -1;
    }
    pcie->dma_src_va = buf_src;
    if (vfio->dma_map(vfio, buf_dst, (uint64_t)len, &pcie->dma_dst) < 0) {
        /* 回滚已映射的 src */
        vfio->dma_unmap(vfio, pcie->dma_src, (uint64_t)len);
        pcie->dma_src_va = NULL;
        return -1;
    }
    pcie->dma_dst_va = buf_dst;
    pcie->dma_len = len;
    pcie->dma_dir = direction;
    return 0;
}

/*
 * 通用便捷 DMA 搬运：dma_config（映射+配置）→ dma_run（触发搬运）→ 解除映射复位。
 * 与设备无关：映射/解映射用通用 dma_map/dma_unmap，实际搬运派发到 dma_run。
 *
 * 注意：这是"一次性"便捷封装，每次调用都会重新映射 + 搬运 + 解除映射（重配置开销
 * 来自 dma_map/dma_unmap 的 IOMMU 页表编程/TLB 无效化/页 pin-unpin，微秒~几十微秒级）。
 * 若同一缓冲要反复搬运，应改用 dma_config（映射一次）+ 反复 dma_run（复用 IOVA，
 * 无重配置开销），避免每次 dma_copy 都重复 map/unmap。
 */
static int __dma_copy(Vfio_Pcie *pcie, void *buf_src, void *buf_dst,
                      uint32_t len)
{
    Vfio *vfio = (Vfio *)pcie;
    int ret = -1;

    if (pcie == NULL || buf_src == NULL || buf_dst == NULL || len == 0) {
        return -1;
    }
    /* 映射 + 配置（内部 dma_map ×2 + 记录 iova/len/dir） */
    if (pcie->dma_config(pcie, buf_src, buf_dst, len,
                         VFIO_DMA_TO_DEVICE) < 0) {
        return -1;
    }
    /* 触发搬运并等待完成（成功返回 >= 0，失败返回负值） */
    if (vfio->dma_run(vfio) >= 0) {
        ret = 0;
    }
    /* 清理：解除映射并复位状态（下次 dma_config 会重新映射） */
    if (pcie->dma_dst_va != NULL) {
        vfio->dma_unmap(vfio, pcie->dma_dst, (uint64_t)pcie->dma_len);
        pcie->dma_dst_va = NULL;
    }
    if (pcie->dma_src_va != NULL) {
        vfio->dma_unmap(vfio, pcie->dma_src, (uint64_t)pcie->dma_len);
        pcie->dma_src_va = NULL;
    }
    pcie->dma_len = 0;
    return ret;
}

static int __construct(Vfio_Pcie *module, char *init_str)
{
    int i;

    module->bdf = NULL;
    module->width = 32;
    module->bar_shift = 12;
    memset(module->bar_size, 0, sizeof(module->bar_size));
    memset(module->bar_mapped, 0, sizeof(module->bar_mapped));
    module->dma_src_va = NULL;
    module->dma_dst_va = NULL;
    module->dma_src = 0;
    module->dma_dst = 0;
    module->dma_len = 0;
    module->dma_dir = VFIO_DMA_TO_DEVICE;
    return 0;
}

static int __deconstruct(Vfio_Pcie *module)
{
    Vfio *vfio = (Vfio *)module;

    /* 自动解除 dma_config 遗留的 DMA 映射（若已配置），无需调用方显式 dma_unmap。
     * 析构顺序为子类先、父类后：此处执行时 Vfio.__deconstruct 尚未关闭 fd，
     * container_fd 仍有效，VFIO_IOMMU_UNMAP_DMA 可用。 */
    if (module->dma_src_va != NULL) {
        vfio->dma_unmap(vfio, module->dma_src, (uint64_t)module->dma_len);
        module->dma_src_va = NULL;
    }
    if (module->dma_dst_va != NULL) {
        vfio->dma_unmap(vfio, module->dma_dst, (uint64_t)module->dma_len);
        module->dma_dst_va = NULL;
    }
    module->dma_src = 0;
    module->dma_dst = 0;
    module->dma_len = 0;

    if (module->bdf != NULL) {
        free(module->bdf);
        module->bdf = NULL;
    }
    return 0;
}

/*
 * Vfio_Pcie 注册接口。
 * 只注册 Vfio_Pcie 结构体中显式声明的字段（open_device/map_bar/set_width/
 * read/write_register(s)）；其余（open/close/get_info/map_region/register_irq/
 * dma_map 等）继承 Vfio，无需在子类注册。
 */
DEFINE_CLASS(
    EXTENDS(Vfio_Pcie, Vfio),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open_device, __open_device),
    Class_VFunc_Entry(map_bar, __map_bar),
    Class_VFunc_Entry(set_width, __set_width),
    Class_VFunc_Entry(read_register, __read_register),
    Class_VFunc_Entry(write_register, __write_register),
    Class_VFunc_Entry(read_registers, __read_registers),
    Class_VFunc_Entry(write_registers, __write_registers),
    Class_VFunc_Entry(dma_config, __dma_config),
    /* dma_run：NULL 继承 Vfio 的不支持默认，由具体设备类（Vfio_Pcie_Edu）override */
    Class_VFunc_Entry(dma_run, NULL),
    Class_VFunc_Entry(dma_copy, __dma_copy)
);
