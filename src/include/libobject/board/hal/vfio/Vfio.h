#ifndef __VFIO_H__
#define __VFIO_H__

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/mman.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>

typedef struct Vfio_s Vfio;

/* 中断组数与每组最多向量数（具体含义由设备/总线定义，如 PCIe 的 INTx/MSI/MSI-X/ERR/REQ） */
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
    int irq_index;                 /* 中断组索引（含义由设备定义，如 PCIe 的 INTx/MSI/MSI-X） */
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
    uint32_t flags;        /* VFIO_DEVICE_FLAGS_*（由总线类型决定） */
    uint32_t num_regions;  /* region 数量 */
    uint32_t num_irqs;     /* irq 组数量 */
    char group_path[64];   /* /dev/vfio/N */
    char device_name[32];  /* group 内设备名（PCI 设备形如 "0000:00:02.0"） */
} vfio_dev_info_t;

/* VFIO region 信息（对应内核 vfio_region_info） */
typedef struct vfio_region_info_ex {
    int      index;        /* region 索引（PCIe 设备里 BAR0-5 对应 0-5） */
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
    /* 打开：按 group_path（如 "/dev/vfio/12"）+ group 内设备名（PCI 设备如 BDF） */
    int (*open)(Vfio *vfio, char *group_path, char *device_name);
    int (*close)(Vfio *vfio);
    /* 获取设备信息 */
    int (*get_info)(Vfio *vfio, vfio_dev_info_t *info);
    /* 获取指定 region 的信息 */
    int (*get_region_info)(Vfio *vfio, int index, vfio_region_info_t *info);
    /* 映射/解除映射 region（多 region 并存，region_base[]/region_size[]） */
    int (*map_region)(Vfio *vfio, int index);
    int (*unmap_region)(Vfio *vfio, int index);
    /* 配置 region 访问位宽：32 或 64（设备相关，默认 32）。 */
    int (*set_width)(Vfio *vfio, int width);
    /* 通用 region 访问（与总线无关，需先 map_region）：
     *   index=region 序号，offset=region 内字节偏移；位宽用 set_width 配置的 reg_width。 */
    int (*region_read)(Vfio *vfio, int index, uint64_t offset, uint64_t *data);
    int (*region_write)(Vfio *vfio, int index, uint64_t offset, uint64_t data);
    /* 中断：eventfd + io_worker 异步，sub_index 为组内向量号（如 MSI/MSI-X 向量） */
    int (*register_irq)(Vfio *vfio, int irq_index, int sub_index,
                        vfio_irq_handler_t handler, void *opaque);
    int (*mask_irq)(Vfio *vfio, int irq_index, int sub_index);
    int (*unmask_irq)(Vfio *vfio, int irq_index, int sub_index);
    /* DMA 映射：把 buf（CPU 虚拟地址 VA）映射为设备侧虚拟地址（IOVA），设备用 IOVA 访问。
     *   - 入参 buf=VA、size=字节数；出参 *iova=设备侧地址（IOVA）。
     *   - 注意：IOVA 是【给 IOMMU 的输入地址】，由本类内部游标分配，并非把 VA 翻译得来；
     *     VA 与 IOVA 是平级的两个虚拟地址，各自经 MMU / IOMMU 翻译到同一块物理地址(PA)。
     *   - 设备发 DMA 时只认 IOVA，由 IOMMU 查表翻成 PA。详见
     *     doc/board/vfio设计文档.md 6.4.1「地址模型：VA / PA / IOVA 三者关系」。 */
    int (*dma_map)(Vfio *vfio, void *buf, uint64_t size, uint64_t *iova);
    /* 解除映射：只能用 IOVA（IOMMU 里登记的就是它），不能用 VA */
    int (*dma_unmap)(Vfio *vfio, uint64_t iova, uint64_t size);
    /* 设备级 DMA 搬运接口（多态：Vfio 只声明接口，默认返回 -1 表示不支持）。
     * 只声明"触发"这一步：主机内存端由调用方用父类 dma_map 准备为 IOVA（并自行
     * 负责 dma_unmap），再把地址写入具体设备类的入参块（如 Vfio_Pcie 的
     * dma_src/dma_dst/dma_len）；dma_run 读取该入参块投递并触发一次搬运
     * （同步阻塞等待完成）。设备相关，由具体设备类（如 Vfio_Pcie_Edu）override。 */
    int (*dma_run)(Vfio *vfio);

    /*attribs*/
    int container_fd;       /* /dev/vfio/vfio */
    int group_fd;           /* /dev/vfio/<N> */
    int device_fd;          /* VFIO_GROUP_GET_DEVICE_FD 返回的设备 fd */
    char *group_path;       /* 如 "/dev/vfio/12" */
    char device_name[32];   /* group 内设备名（PCI 设备形如 "0000:00:02.0"） */
    vfio_dev_info_t info;   /* 设备信息 */
    uint8_t *region_base[16];   /* 各 region mmap 基址，NULL 表示未映射 */
    uint64_t region_size[16];   /* 各 region 大小 */
    vfio_irq_ctx_t irq_ctx[VFIO_MAX_IRQ_GROUPS][VFIO_MAX_IRQ_VECTORS_PER_GROUP]; /* 各向量的中断状态（efd/worker/handler/opaque 合一） */
    uint32_t irq_count;              /* 最近一次中断计数（供 handler 读取） */
    pthread_mutex_t lock;            /* 进程内互斥锁，保护所有操作 */
    int reg_width;                   /* region 访问默认位宽：32/64，默认 32（set_width 配置） */
    uint64_t iova_base;              /* IOVA 分配起始地址；0=用默认安全值（dma_map 前可设） */
    uint64_t iova;                   /* 内部 IOVA 分配游标（dma_map 按页递增分配） */
};

#endif
