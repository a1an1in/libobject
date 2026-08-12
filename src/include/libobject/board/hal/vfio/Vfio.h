#ifndef __VFIO_H__
#define __VFIO_H__

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/mman.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>

typedef struct Vfio_s Vfio;

/* 中断组数（INTx/MSI/MSI-X/ERR/REQ）与每组最多向量数（MSI-X 常见 ≤64，可按需调大） */
#define VFIO_MAX_IRQ_GROUPS        5
#define VFIO_MAX_IRQ_VECTORS_PER_GROUP 64

/*
 * VFIO 异步中断处理函数（即 io_worker 的 work_callback）。
 * opaque 为 io_worker 传入的 Worker*；用 vfio_irq_get_vfio(opaque) 取回 Vfio 对象。
 */
typedef int (*vfio_irq_handler_t)(void *opaque);

/*
 * 每个 (irq_index, sub_index) 向量的中断注册状态（efd/worker/handler 合一）。
 * 以二维数组形式内嵌在 Vfio 对象里（无需动态分配）；
 * worker->opaque 指向本结构，供 __irq_ev_callback 定位向量并分发到对应 handler。
 */
typedef struct vfio_irq_ctx {
    Vfio *vfio;                    /* 所属 Vfio 对象 */
    int irq_index;                 /* 中断组（INTx/MSI/MSI-X/ERR/REQ） */
    int sub_index;                 /* 组内向量号 */
    int efd;                       /* 该向量 eventfd，-1=未注册 */
    Worker *worker;                /* 该向量 io_worker（异步中断） */
    vfio_irq_handler_t handler;    /* 该向量 handler */
    void *opaque;                  /* 该向量 opaque */
} vfio_irq_ctx_t;

/* 中断处理函数里获取所属 Vfio 对象（opaque 为 io_worker 传入的 Worker*） */
static inline Vfio *vfio_irq_get_vfio(void *opaque)
{
    Worker *worker = (Worker *)opaque;
    return ((vfio_irq_ctx_t *)worker->opaque)->vfio;
}

/* VFIO 设备信息（对应内核 vfio_device_info） */
typedef struct vfio_dev_info {
    uint32_t flags;        /* VFIO_DEVICE_FLAGS_*（PCI=1<<1） */
    uint32_t num_regions;  /* region 数量 */
    uint32_t num_irqs;     /* irq 组数量 */
    char group_path[64];   /* /dev/vfio/N */
    char device_name[32];  /* 如 "0000:00:02.0" */
} vfio_dev_info_t;

/* VFIO region 信息（对应内核 vfio_region_info） */
typedef struct vfio_region_info_ex {
    int      index;        /* region 索引（PCIe: BAR0-5） */
    uint64_t offset;       /* 设备 fd mmap 偏移（内核返回） */
    uint64_t size;         /* 大小 */
    uint32_t flags;        /* VFIO_REGION_INFO_FLAG_* */
} vfio_region_info_t;

/*
 * 设备级 DMA 搬运的抽象方向（与设备无关，具体设备实现时翻译成自己的寄存器位）：
 *   VFIO_DMA_TO_DEVICE  ：设备从 guest 内存读数据（CPU→设备）
 *   VFIO_DMA_FROM_DEVICE：设备向 guest 内存写数据（设备→CPU）
 */
enum {
    VFIO_DMA_TO_DEVICE   = 0,
    VFIO_DMA_FROM_DEVICE = 1,
};

/*
 * Vfio 类：通用 VFIO 用户态驱动基类（与总线无关）。
 *
 * 基于 /dev/vfio/vfio（container）+ /dev/vfio/<N>（group）+ 设备 fd 三层 fd：
 *   - open：按 group_path + device_name 打开，完成 container/group/device 三层绑定，
 *     VFIO_DEVICE_GET_INFO 获取设备信息。
 *   - map_region / unmap_region：按 region 索引在设备 fd 上 mmap（VFIO 返回 offset，
 *     IOMMU 隔离，替代 UIO 的 /dev/mem）。多 region 并存。
 *   - register_irq：eventfd + VFIO_DEVICE_SET_IRQS + io_worker 异步（参考 Uio.register_irq）。
 *   - dma_map / dma_unmap：VFIO_IOMMU_MAP_DMA 把用户缓冲映射为 IOVA，设备经 IOMMU 访问。
 *
 * 继承关系：Obj -> Vfio
 */

struct Vfio_s {
    Obj parent;

    int (*construct)(Vfio *, char *);
    int (*deconstruct)(Vfio *);

    /*virtual methods reimplement*/
    int (*set)(Vfio *module, char *attrib, void *value);
    void *(*get)(Vfio *, char *attrib);
    char *(*to_json)(Vfio *);

    /* VFIO device interface */
    /* 打开：按 group_path（如 "/dev/vfio/12"）+ device_name（如 "0000:00:02.0"） */
    int (*open)(Vfio *vfio, char *group_path, char *device_name);
    int (*close)(Vfio *vfio);
    /* 获取设备信息 */
    int (*get_info)(Vfio *vfio, vfio_dev_info_t *info);
    /* 获取指定 region 的信息 */
    int (*get_region_info)(Vfio *vfio, int index, vfio_region_info_t *info);
    /* 映射/解除映射 region（多 region 并存，region_base[]/region_size[]） */
    int (*map_region)(Vfio *vfio, int index);
    int (*unmap_region)(Vfio *vfio, int index);
    /* 中断：eventfd + io_worker 异步，sub_index 为 MSI/MSI-X 向量子序号 */
    int (*register_irq)(Vfio *vfio, int irq_index, int sub_index,
                        vfio_irq_handler_t handler, void *opaque);
    int (*mask_irq)(Vfio *vfio, int irq_index, int sub_index);
    int (*unmask_irq)(Vfio *vfio, int irq_index, int sub_index);
    /* DMA：把 buf（用户虚拟地址）映射为 iova，设备用 iova 访问 */
    int (*dma_map)(Vfio *vfio, void *buf, uint64_t size, uint64_t *iova);
    int (*dma_unmap)(Vfio *vfio, uint64_t iova, uint64_t size);
    /* 设备级 DMA 搬运接口（多态：Vfio 只声明接口，默认返回 -1 表示不支持）。
     *   - dma_config：配置一次搬运，入参为【用户虚拟地址】buf_src/buf_dst，
     *     内部 dma_map ×2 得到 IOVA 并记录到 pcie->dma_src/dma_dst/dma_len/dma_dir，
     *     不触发；通用实现由 Vfio_Pcie 提供，可复用（同配置反复 dma_run 不重映射）。
     *   - dma_run    ：触发一次搬运并等待完成（同步阻塞），设备相关，
     *     由具体设备类（如 Vfio_Pcie_Edu）override，读取 dma_* 属性执行。 */
    int (*dma_config)(Vfio *vfio, void *buf_src, void *buf_dst,
                      uint32_t len, int direction);
    int (*dma_run)(Vfio *vfio);

    /*attribs*/
    int container_fd;       /* /dev/vfio/vfio */
    int group_fd;           /* /dev/vfio/<N> */
    int device_fd;          /* VFIO_GROUP_GET_DEVICE_FD 返回的设备 fd */
    char *group_path;       /* 如 "/dev/vfio/12" */
    char device_name[32];   /* 如 "0000:00:02.0" */
    vfio_dev_info_t info;   /* 设备信息 */
    uint8_t *region_base[16];   /* 各 region mmap 基址，NULL 表示未映射 */
    uint64_t region_size[16];   /* 各 region 大小 */
    vfio_irq_ctx_t irq_ctx[VFIO_MAX_IRQ_GROUPS][VFIO_MAX_IRQ_VECTORS_PER_GROUP]; /* 各向量的中断状态（efd/worker/handler/opaque 合一） */
    uint32_t irq_count;              /* 最近一次中断计数（供 handler 读取） */
    pthread_mutex_t lock;            /* 进程内互斥锁，保护所有操作 */
    uint64_t iova;                   /* 内部 IOVA 分配游标（dma_map 按页递增分配） */
};

#endif
