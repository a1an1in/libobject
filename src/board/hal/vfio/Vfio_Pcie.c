/**
 * @file Vfio_Pcie.c
 * @Synopsis 基于 VFIO 的 PCIe 设备用户态驱动。
 * 只新增 PCIe 特有功能（其余机制与 region 访问都在基类 Vfio）：
 *   - open_device：按 vendor/device ID 在 /sys/bus/pci/devices 下发现设备，
 *     读取 iommu_group → /dev/vfio/<N>，绑定 vfio-pci，调用父类 open。
 *   - BAR = region：VFIO-PCI 约定 BAR n = region n，映射/访问 BAR 直接用父类
 *     Vfio.map_region / region_read / region_write。
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

static int __construct(Vfio_Pcie *module, char *init_str)
{
    module->bdf = NULL;
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

    /* 若调用方在 dma_src_va/dma_dst_va 里标记了已映射的缓冲，析构时自动 dma_unmap，
     * 调用方无需显式解除；未标记（NULL）则由调用方自己负责（如测试显式 unmap）。
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
 * 只注册 Vfio_Pcie 结构体中显式声明的字段（open_device）；
 * 其余（open/close/get_info/map_region/region_read/region_write/register_irq/
 * dma_map 等）继承 Vfio，无需在子类注册。
 */
DEFINE_CLASS(
    EXTENDS(Vfio_Pcie, Vfio),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open_device, __open_device),
    /* dma_run：NULL 继承 Vfio 的不支持默认，由具体设备类（Vfio_Pcie_Edu）override */
    Class_VFunc_Entry(dma_run, NULL)
);
