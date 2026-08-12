/**
 * @file Vfio.c
 * @Synopsis  VFIO 通用用户态设备访问基类（与总线无关）。
 * 基于 /dev/vfio/vfio（container）+ /dev/vfio/<N>（group）+ 设备 fd 三层 fd：
 *   - open/close：三层 fd 生命周期（container → group → device）。
 *   - get_info/get_region_info：设备信息与 region（BAR）信息。
 *   - map_region/unmap_region：按 region 索引在设备 fd 上 mmap（VFIO 返回 offset，
 *     IOMMU 隔离，替代 UIO 的 /dev/mem）。多 region 并存。
 *   - register_irq：eventfd + VFIO_DEVICE_SET_IRQS + io_worker 异步（参考 Uio.register_irq）。
 *   - mask/unmask_irq：电平触发（AUTOMASKED）中断的手动 mask/unmask。
 *   - dma_map/dma_unmap：VFIO_IOMMU_MAP_DMA 把用户缓冲映射为 IOVA，设备经 IOMMU 访问。
 * 本类依赖内核 CONFIG_VFIO + CONFIG_VFIO_IOMMU_TYPE1 + CONFIG_VFIO_PCI，且设备须落在
 * 某个 iommu_group（有 SMMU/IOMMU）并绑定 vfio-pci 驱动。
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
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <linux/vfio.h>
#include <libobject/board/hal/vfio/Vfio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/concurrent/worker_api.h>
#include <libobject/concurrent/Producer.h>

#define VFIO_CONTAINER_PATH "/dev/vfio/vfio"
#define VFIO_MAX_REGIONS    16
#define VFIO_PAGE_SIZE      4096
#define VFIO_SYSFS_PATH     "/sys/class/vfio"
#define VFIO_DEV_PATH       "/dev/vfio"
#define VFIO_MAX_PATH_LEN   256

/*
 * 若 /dev/vfio/<N> 节点不存在（如 busybox 无 devtmpfs/udev），从
 * /sys/class/vfio/vfio<N>/dev 读取 major:minor 并 mknod 创建。
 * （同 Uio_Pcie.__ensure_uio_devnode 的处理方式。）
 */
static int __ensure_group_devnode(const char *group_path)
{
    char sys_path[VFIO_MAX_PATH_LEN];
    char buf[32] = {0};
    const char *p;
    int fd, len, major = 0, minor = 0, num;

    if (access(group_path, F_OK) == 0) {
        return 0;
    }

    /* 从 "/dev/vfio/<N>" 解析出组号 N */
    p = strrchr(group_path, '/');
    if (p == NULL) {
        return -1;
    }
    num = atoi(p + 1);

    /* 确保 /dev/vfio 目录存在 */
    if (access(VFIO_DEV_PATH, F_OK) < 0) {
        if (mkdir(VFIO_DEV_PATH, 0755) < 0) {
            return -1;
        }
    }

    /* 读 /sys/class/vfio/<N>/dev 的 major:minor（注意是组号目录，无 vfio 前缀） */
    snprintf(sys_path, sizeof(sys_path), "%s/%d/dev", VFIO_SYSFS_PATH, num);
    fd = open(sys_path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (len <= 0 || sscanf(buf, "%d:%d", &major, &minor) != 2) {
        return -1;
    }

    if (mknod(group_path, S_IFCHR | 0600, makedev(major, minor)) < 0) {
        return -1;
    }
    dbg_str(DBG_INFO, "vfio group devnode created: %s (%d:%d)",
            group_path, major, minor);
    return 0;
}

/*
 * 打开 VFIO 设备：按 group_path（如 "/dev/vfio/12"）+ device_name（如 "0000:00:02.0"）
 * 完成三层 fd 绑定。任一步失败按"设备→组→容器"逆序回滚已打开的 fd。
 */
static int __open(Vfio *vfio, char *group_path, char *device_name)
{
    struct vfio_device_info dev_info;
    int api_version, ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || group_path == NULL || device_name == NULL, -1);
        THROW_IF(vfio->device_fd >= 0, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        /* 1. 打开 container：/dev/vfio/vfio，校验 API 版本与 TYPE1 IOMMU 支持 */
        vfio->container_fd = open(VFIO_CONTAINER_PATH, O_RDWR);
        THROW_IF(vfio->container_fd < 0, -1);
        api_version = ioctl(vfio->container_fd, VFIO_GET_API_VERSION);
        THROW_IF(api_version != VFIO_API_VERSION, -1);
        THROW_IF(ioctl(vfio->container_fd, VFIO_CHECK_EXTENSION,
                       VFIO_TYPE1_IOMMU) != 1, -1);

        /* 2. 打开 group：/dev/vfio/<N>（无 devtmpfs 时先 mknod），并设置 container */
        __ensure_group_devnode(group_path);
        vfio->group_fd = open(group_path, O_RDWR);
        THROW_IF(vfio->group_fd < 0, -1);
        THROW_IF(ioctl(vfio->group_fd, VFIO_GROUP_SET_CONTAINER,
                       &vfio->container_fd) < 0, -1);

        /* 3. 为 container 设置 TYPE1 IOMMU */
        THROW_IF(ioctl(vfio->container_fd, VFIO_SET_IOMMU,
                       VFIO_TYPE1_IOMMU) < 0, -1);

        /* 4. 获取设备 fd，并读取设备信息 */
        vfio->device_fd = ioctl(vfio->group_fd, VFIO_GROUP_GET_DEVICE_FD,
                                device_name);
        THROW_IF(vfio->device_fd < 0, -1);

        memset(&dev_info, 0, sizeof(dev_info));
        dev_info.argsz = sizeof(dev_info);
        THROW_IF(ioctl(vfio->device_fd, VFIO_DEVICE_GET_INFO, &dev_info) < 0,
                 -1);
        memset(&vfio->info, 0, sizeof(vfio->info));
        vfio->info.flags = dev_info.flags;
        vfio->info.num_regions = dev_info.num_regions;
        vfio->info.num_irqs = dev_info.num_irqs;
        snprintf(vfio->info.group_path, sizeof(vfio->info.group_path), "%s",
                 group_path);
        snprintf(vfio->info.device_name, sizeof(vfio->info.device_name), "%s",
                 device_name);

        if (vfio->group_path == NULL) {
            vfio->group_path = strdup(group_path);
        } else {
            free(vfio->group_path);
            vfio->group_path = strdup(group_path);
        }
        THROW_IF(vfio->group_path == NULL, -1);
        snprintf(vfio->device_name, sizeof(vfio->device_name), "%s",
                 device_name);

        dbg_str(DBG_INFO, "vfio open success, group:%s, dev:%s, "
                "num_regions:%u, num_irqs:%u, container_fd:%d, group_fd:%d, "
                "device_fd:%d",
                group_path, device_name, vfio->info.num_regions,
                vfio->info.num_irqs, vfio->container_fd, vfio->group_fd,
                vfio->device_fd);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio open failed, group:%s, dev:%s, errno:%d(%s)",
                group_path, device_name, errno, strerror(errno));
        if (vfio->device_fd >= 0) {
            close(vfio->device_fd);
            vfio->device_fd = -1;
        }
        if (vfio->group_fd >= 0) {
            close(vfio->group_fd);
            vfio->group_fd = -1;
        }
        if (vfio->container_fd >= 0) {
            close(vfio->container_fd);
            vfio->container_fd = -1;
        }
        if (vfio->group_path != NULL) {
            free(vfio->group_path);
            vfio->group_path = NULL;
        }
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/* 获取设备信息 */
static int __get_info(Vfio *vfio, vfio_dev_info_t *info)
{
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || info == NULL, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        *info = vfio->info;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio get_info failed, errno:%d(%s)",
                errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/*
 * 获取指定 region 的信息（VFIO_DEVICE_GET_REGION_INFO）。
 * region 索引即 PCIe 的 BAR0-5（vfio-pci 固定映射）。
 */
static int __get_region_info(Vfio *vfio, int index, vfio_region_info_t *info)
{
    struct vfio_region_info reg;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || info == NULL || vfio->device_fd < 0, -1);
        THROW_IF(index < 0 || index >= VFIO_MAX_REGIONS, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        memset(&reg, 0, sizeof(reg));
        reg.argsz = sizeof(reg);
        reg.index = (__u32)index;
        THROW_IF(ioctl(vfio->device_fd, VFIO_DEVICE_GET_REGION_INFO, &reg) < 0,
                 -1);

        info->index = (int)reg.index;
        info->offset = reg.offset;
        info->size = reg.size;
        info->flags = reg.flags;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio get_region_info failed, index:%d, "
                "errno:%d(%s)", index, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/*
 * 映射 region（BAR）：在设备 fd 上 mmap，offset 用 VFIO 返回的 reg.offset
 * （设备 fd 地址空间内的 region 偏移，非物理地址；由 IOMMU 隔离）。
 * 多 region 并存：region_base[]/region_size[] 各存一份。
 */
static int __map_region(Vfio *vfio, int index)
{
    struct vfio_region_info reg;
    uint8_t *base;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || vfio->device_fd < 0, -1);
        THROW_IF(index < 0 || index >= VFIO_MAX_REGIONS, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        memset(&reg, 0, sizeof(reg));
        reg.argsz = sizeof(reg);
        reg.index = (__u32)index;
        THROW_IF(ioctl(vfio->device_fd, VFIO_DEVICE_GET_REGION_INFO, &reg) < 0,
                 -1);
        THROW_IF(!(reg.flags & VFIO_REGION_INFO_FLAG_MMAP), -1);
        THROW_IF(reg.size == 0, -1);

        /* 已映射则先解除（其它 region 不受影响，支持多 region 并存） */
        if (vfio->region_base[index] != NULL &&
            vfio->region_base[index] != MAP_FAILED) {
            munmap(vfio->region_base[index], vfio->region_size[index]);
            vfio->region_base[index] = NULL;
        }

        base = mmap(NULL, reg.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    vfio->device_fd, reg.offset);
        THROW_IF(base == MAP_FAILED, -1);

        vfio->region_base[index] = base;
        vfio->region_size[index] = reg.size;
        dbg_str(DBG_INFO, "vfio map_region success, index:%d, offset:0x%llx, "
                "size:0x%llx, base:%p",
                index, (unsigned long long)reg.offset,
                (unsigned long long)reg.size, base);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio map_region failed, index:%d, errno:%d(%s)",
                index, errno, strerror(errno));
        if (vfio->region_base[index] != NULL &&
            vfio->region_base[index] != MAP_FAILED) {
            munmap(vfio->region_base[index], vfio->region_size[index]);
            vfio->region_base[index] = NULL;
        }
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/* 解除指定 region 的映射 */
static int __unmap_region(Vfio *vfio, int index)
{
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL, -1);
        THROW_IF(index < 0 || index >= VFIO_MAX_REGIONS, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        if (vfio->region_base[index] != NULL &&
            vfio->region_base[index] != MAP_FAILED) {
            munmap(vfio->region_base[index], vfio->region_size[index]);
        }
        vfio->region_base[index] = NULL;
        vfio->region_size[index] = 0;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio unmap_region failed, index:%d, errno:%d(%s)",
                index, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/*
 * io_worker 的事件回调：eventfd 可读（有中断）时被异步调用。
 * 读取 eventfd 计数（8 字节）清除事件，保存到 vfio->irq_count，
 * 然后调用用户注册的中断处理函数（worker 的 work_callback）。
 */
static void __irq_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    vfio_irq_ctx_t *ctx = (vfio_irq_ctx_t *)worker->opaque;
    Vfio *vfio = ctx->vfio;
    uint64_t count = 0;
    ssize_t ret;

    ret = read(fd, &count, sizeof(count));
    if (ret != (ssize_t)sizeof(count)) {
        dbg_str(DBG_ERROR, "vfio read eventfd failed, errno:%d(%s)",
                errno, strerror(errno));
        return;
    }

    vfio->irq_count = (uint32_t)count;
    dbg_str(DBG_DETAIL, "vfio irq triggered, index:%d, sub:%d, count:%lu",
            ctx->irq_index, ctx->sub_index, (unsigned long)count);

    /* 分发到该 (index, 向量) 对应的 handler（支持多向量各自 handler） */
    if (ctx->handler != NULL) {
        ctx->handler(worker);
    }
}

/*
 * 注册异步中断（基于 eventfd + io_worker）。
 * 通过 VFIO_DEVICE_SET_IRQS（DATA_EVENTFD|ACTION_TRIGGER）把中断绑定到 eventfd，
 * 再用 io_worker 监听该 eventfd 的 EV_READ | EV_PERSIST 事件，
 * 中断到来时异步调用 handler。
 */
static int __register_irq(Vfio *vfio, int irq_index, int sub_index,
                          vfio_irq_handler_t handler, void *opaque)
{
    /* vfio_irq_set.data[] 为柔性数组：用内嵌 set + 尾随 eventfd 的局部结构体，
     * 避免堆分配，且 sizeof() 包含 data 空间（消除 memcpy 溢出警告） */
    struct {
        struct vfio_irq_set set;
        __s32 data;
    } irq_pack;
    struct vfio_irq_set *irq_set = &irq_pack.set;
    allocator_t *allocator;
    Worker *worker;
    vfio_irq_ctx_t *ctx = NULL;
    int32_t efd;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || vfio->device_fd < 0 || handler == NULL, -1);
        THROW_IF(irq_index < 0 || irq_index >= VFIO_MAX_IRQ_GROUPS, -1);
        THROW_IF(sub_index < 0 || sub_index >= VFIO_MAX_IRQ_VECTORS_PER_GROUP, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        ctx = &vfio->irq_ctx[irq_index][sub_index];

        /* 若该 (index, 向量) 已注册过，先注销旧 io_worker 并关闭旧 eventfd。
         * 不同向量的注册互不影响 → 支持 MSI-X 多向量各自 handler。 */
        if (ctx->worker != NULL) {
            worker_destroy(ctx->worker);
            ctx->worker = NULL;
        }
        if (ctx->efd >= 0) {
            close(ctx->efd);
            ctx->efd = -1;
        }

        /* 1. 创建 eventfd */
        efd = eventfd(0, EFD_NONBLOCK);
        THROW_IF(efd < 0, -1);

        /* 2. 把该向量绑定到 eventfd（count=1，绑定单个向量 sub_index）。
         *    MSI-X 支持逐向量增量绑定（每次 SET_IRQS 一个向量），
         *    因此可分别注册多个向量、各绑一个 eventfd/handler。 */
        memset(&irq_pack, 0, sizeof(irq_pack));
        irq_pack.set.argsz = sizeof(irq_pack);
        irq_pack.set.flags = VFIO_IRQ_SET_DATA_EVENTFD |
                             VFIO_IRQ_SET_ACTION_TRIGGER;
        irq_pack.set.index = (__u32)irq_index;
        irq_pack.set.start = (__u32)sub_index;
        irq_pack.set.count = 1;
        irq_pack.data = efd;
        THROW_IF(ioctl(vfio->device_fd, VFIO_DEVICE_SET_IRQS, irq_set) < 0,
                 -1);

        /* 3. 填充该向量 ctx，并挂到 worker->opaque（回调按 ctx 定位/分发） */
        ctx->vfio = vfio;
        ctx->irq_index = irq_index;
        ctx->sub_index = sub_index;
        ctx->efd = efd;
        ctx->handler = handler;
        ctx->opaque = opaque;

        /* 4. io_worker 异步监听 eventfd */
        allocator = vfio->parent.allocator;
        worker = io_worker(allocator, efd, EV_READ | EV_PERSIST,
                           NULL, NULL, __irq_ev_callback, handler, ctx);
        THROW_IF(worker == NULL, -1);
        ctx->worker = worker;

        dbg_str(DBG_INFO, "vfio register_irq ok, index:%d, sub:%d, "
                "eventfd:%d, worker:%p",
                irq_index, sub_index, efd, worker);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio register_irq failed, index:%d, sub:%d, "
                "errno:%d(%s)",
                irq_index, sub_index, errno, strerror(errno));
        if (ctx != NULL && ctx->worker != NULL) {
            worker_destroy(ctx->worker);
            ctx->worker = NULL;
        }
        if (ctx != NULL && ctx->efd >= 0) {
            close(ctx->efd);
            ctx->efd = -1;
        }
        if (ctx != NULL) {
            ctx->handler = NULL;
            ctx->opaque = NULL;
        }
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/* mask/unmask 指定 irq（level 触发 AUTOMASKED 中断需要手动 unmask） */
static int __mask_unmask_irq(Vfio *vfio, int irq_index, int sub_index,
                             uint32_t action)
{
    struct vfio_irq_set irq_set;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || vfio->device_fd < 0, -1);
        THROW_IF(irq_index < 0 || irq_index >= VFIO_MAX_IRQ_GROUPS, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        memset(&irq_set, 0, sizeof(irq_set));
        irq_set.argsz = sizeof(irq_set);
        irq_set.flags = VFIO_IRQ_SET_DATA_NONE | action;
        irq_set.index = (__u32)irq_index;
        irq_set.start = (__u32)sub_index;
        irq_set.count = 1;
        THROW_IF(ioctl(vfio->device_fd, VFIO_DEVICE_SET_IRQS, &irq_set) < 0,
                 -1);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio mask/unmask irq failed, index:%d, "
                "action:0x%x, errno:%d(%s)",
                irq_index, action, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

static int __mask_irq(Vfio *vfio, int irq_index, int sub_index)
{
    return __mask_unmask_irq(vfio, irq_index, sub_index,
                             VFIO_IRQ_SET_ACTION_MASK);
}

static int __unmask_irq(Vfio *vfio, int irq_index, int sub_index)
{
    return __mask_unmask_irq(vfio, irq_index, sub_index,
                             VFIO_IRQ_SET_ACTION_UNMASK);
}

/*
 * 把用户缓冲 buf 映射为 IOVA（VFIO_IOMMU_MAP_DMA）。
 * iova 由内部游标按页递增分配（自窗口起始处，避免与设备 DMA mask 冲突）；
 * 调用方把 *iova 写入设备 DMA 寄存器，设备经 IOMMU 访问该缓冲。
 * 注意：buf 需页对齐（或用 vfio 建议对齐），size 建议为页的整数倍。
 */
static int __dma_map(Vfio *vfio, void *buf, uint64_t size, uint64_t *iova)
{
    struct vfio_iommu_type1_dma_map dma_map;
    uint64_t page, base;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || buf == NULL || iova == NULL, -1);
        THROW_IF(vfio->container_fd < 0 || size == 0, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        /* 若未初始化 IOVA 游标，从高位窗口起（避开低地址保留区/guest RAM 区；
         * SMMUv3 stage-1 的 IOVA 空间独立，用接近真实 IOVA 的高位更可靠）。
         * 注意仍需满足设备 dma_mask（edu dma_mask 28 位，此处选 < 256MB 的高位页） */
        if (vfio->iova == 0) {
            vfio->iova = 0x0F000000ULL; /* 240MB 起，按页递增，避开低 64MB */
        }

        page = VFIO_PAGE_SIZE;
        base = (vfio->iova + page - 1) & ~(page - 1);

        memset(&dma_map, 0, sizeof(dma_map));
        dma_map.argsz = sizeof(dma_map);
        dma_map.flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE;
        dma_map.vaddr = (uint64_t)(uintptr_t)buf;
        dma_map.iova = base;
        dma_map.size = size;
        THROW_IF(ioctl(vfio->container_fd, VFIO_IOMMU_MAP_DMA, &dma_map) < 0,
                 -1);

        *iova = dma_map.iova;
        vfio->iova = base + size;
        dbg_str(DBG_INFO, "vfio dma_map success, buf:%p, size:0x%llx, "
                "iova:0x%llx",
                buf, (unsigned long long)size,
                (unsigned long long)dma_map.iova);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio dma_map failed, buf:%p, size:0x%llx, "
                "errno:%d(%s)",
                buf, (unsigned long long)size, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

/* 解除 IOVA 映射 */
static int __dma_unmap(Vfio *vfio, uint64_t iova, uint64_t size)
{
    struct vfio_iommu_type1_dma_unmap dma_unmap;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(vfio == NULL || vfio->container_fd < 0, -1);
        THROW_IF(pthread_mutex_lock(&vfio->lock) != 0, -1);
        locked = 1;

        memset(&dma_unmap, 0, sizeof(dma_unmap));
        dma_unmap.argsz = sizeof(dma_unmap);
        dma_unmap.iova = iova;
        dma_unmap.size = size;
        THROW_IF(ioctl(vfio->container_fd, VFIO_IOMMU_UNMAP_DMA,
                       &dma_unmap) < 0, -1);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vfio dma_unmap failed, iova:0x%llx, size:0x%llx, "
                "errno:%d(%s)",
                (unsigned long long)iova, (unsigned long long)size,
                errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&vfio->lock);
        }
    }

    return ret;
}

static int __close(Vfio *vfio)
{
    int i, j;

    if (vfio == NULL) {
        return -1;
    }

    /* 注销所有 (index, 向量) 的 io_worker 并关闭 eventfd */
    for (i = 0; i < VFIO_MAX_IRQ_GROUPS; i++) {
        for (j = 0; j < VFIO_MAX_IRQ_VECTORS_PER_GROUP; j++) {
            vfio_irq_ctx_t *ctx = &vfio->irq_ctx[i][j];
            if (ctx->worker != NULL) {
                worker_destroy(ctx->worker);
                ctx->worker = NULL;
            }
            if (ctx->efd >= 0) {
                close(ctx->efd);
                ctx->efd = -1;
            }
            ctx->handler = NULL;
            ctx->opaque = NULL;
        }
    }

    /* 解除所有 region 映射 */
    for (i = 0; i < VFIO_MAX_REGIONS; i++) {
        if (vfio->region_base[i] != NULL && vfio->region_base[i] != MAP_FAILED) {
            munmap(vfio->region_base[i], vfio->region_size[i]);
            vfio->region_base[i] = NULL;
        }
        vfio->region_size[i] = 0;
    }

    /* 按"设备→组→容器"逆序关闭 fd */
    if (vfio->device_fd >= 0) {
        close(vfio->device_fd);
        vfio->device_fd = -1;
    }
    if (vfio->group_fd >= 0) {
        close(vfio->group_fd);
        vfio->group_fd = -1;
    }
    if (vfio->container_fd >= 0) {
        close(vfio->container_fd);
        vfio->container_fd = -1;
    }
    if (vfio->group_path != NULL) {
        free(vfio->group_path);
        vfio->group_path = NULL;
    }

    return 0;
}

static int __construct(Vfio *module, char *init_str)
{
    int i, j;

    module->container_fd = -1;
    module->group_fd = -1;
    module->device_fd = -1;
    module->group_path = NULL;
    memset(module->device_name, 0, sizeof(module->device_name));
    memset(&module->info, 0, sizeof(module->info));
    for (i = 0; i < VFIO_MAX_REGIONS; i++) {
        module->region_base[i] = NULL;
        module->region_size[i] = 0;
    }
    for (i = 0; i < VFIO_MAX_IRQ_GROUPS; i++) {
        for (j = 0; j < VFIO_MAX_IRQ_VECTORS_PER_GROUP; j++) {
            memset(&module->irq_ctx[i][j], 0, sizeof(vfio_irq_ctx_t));
            module->irq_ctx[i][j].efd = -1;
        }
    }
    module->irq_count = 0;
    module->iova = 0;
    pthread_mutex_init(&module->lock, NULL);
    return 0;
}

static int __deconstruct(Vfio *module)
{
    if (module->device_fd >= 0 || module->group_fd >= 0 ||
        module->container_fd >= 0) {
        module->close(module);
    }
    pthread_mutex_destroy(&module->lock);
    return 0;
}

/*
 * 设备级 DMA 搬运的默认"不支持"实现。
 * 通用 Vfio 无法知道设备如何触发 DMA，返回 -1；具体设备类（如 Vfio_Edu）
 * 通过同名 Class_VFunc_Entry override 这两个虚函数。
 */
static int __dma_config(Vfio *vfio, uint64_t src_iova,
                        uint64_t dst_iova, uint32_t len,
                        int direction)
{
    dbg_str(DBG_ERROR, "vfio dma_config not supported by this device");
    return -1;
}

static int __dma_run(Vfio *vfio)
{
    dbg_str(DBG_ERROR, "vfio dma_run not supported by this device");
    return -1;
}

DEFINE_CLASS(
    EXTENDS(Vfio, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(get_info, __get_info),
    Class_VFunc_Entry(get_region_info, __get_region_info),
    Class_VFunc_Entry(map_region, __map_region),
    Class_VFunc_Entry(unmap_region, __unmap_region),
    Class_VFunc_Entry(register_irq, __register_irq),
    Class_VFunc_Entry(mask_irq, __mask_irq),
    Class_VFunc_Entry(unmask_irq, __unmask_irq),
    Class_VFunc_Entry(dma_map, __dma_map),
    Class_VFunc_Entry(dma_unmap, __dma_unmap),
    Class_VFunc_Entry(dma_config, __dma_config),
    Class_VFunc_Entry(dma_run, __dma_run)
);
