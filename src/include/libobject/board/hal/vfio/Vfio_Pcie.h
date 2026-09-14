#ifndef __VFIO_PCIE_H__
#define __VFIO_PCIE_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/vfio/Vfio.h>

/*
 * Vfio_Pcie 类：基于 VFIO 的 PCIe 设备用户态驱动。
 *
 * 只新增 PCIe 特有功能（其余机制与 region 访问都在基类 Vfio）：
 *   - open_device：按 vendor/device ID 在 /sys/bus/pci/devices 下发现设备，
 *     解析其 iommu_group（/sys/bus/pci/devices/<BDF>/iommu_group → /dev/vfio/<N>），
 *     绑定 vfio-pci 驱动，再调用父类 open(group_path, device_name)。
 *   - BAR = region：VFIO-PCI 约定 BAR n = region n，因此映射/访问 BAR 直接使用
 *     父类 Vfio 的 region 接口（map_region / region_read / region_write），
 *     无独立的 map_bar / read_register。
 *
 * 典型流程：
 *   Vfio_Pcie *p = object_new(allocator, "Vfio_Pcie", NULL);
 *   p->open_device(p, 0x1234, 0x11e8);            // 发现 edu + 绑定 vfio-pci + 三层 fd
 *   vfio->map_region(vfio, 0);                    // 映射 region 0（= BAR0）
 *   vfio->region_read(vfio, 0, 0x00, &val, 32);   // 读 BAR0 寄存器
 *   vfio->dma_map(vfio, buf, size, &iova);        // 共享内存 DMA
 *
 * 继承关系：Obj -> Vfio -> Vfio_Pcie
 */

typedef struct Vfio_Pcie_s Vfio_Pcie;

struct Vfio_Pcie_s {
    Vfio parent;

    int (*construct)(Vfio_Pcie *, char *);
    int (*deconstruct)(Vfio_Pcie *);

    /*virtual methods reimplement*/
    int (*set)(Vfio_Pcie *module, char *attrib, void *value);
    void *(*get)(Vfio_Pcie *, char *attrib);
    char *(*to_json)(Vfio_Pcie *);

    /* PCIe interface (built on top of Vfio) */
    /* 按 vendor/device ID 发现设备 + 绑定 vfio-pci + 打开三层 fd；
     * 映射/访问 BAR 用父类 region 接口（BAR n = region n） */
    int (*open_device)(Vfio_Pcie *pcie, uint32_t vendor, uint32_t device);

    /* 设备级 DMA：dma_run（接口，默认继承 Vfio 不支持，由具体设备类如
     * Vfio_Pcie_Edu override）；主机内存端由调用方 dma_map 准备，并写入下方 dma_* 入参块 */
    int (*dma_run)(Vfio_Pcie *pcie);

    /*attribs*/
    char *bdf;            /* 总线:设备.功能，如 "0000:00:02.0" */
    /* DMA 搬运入参块（由调用方填充，dma_run 读取执行）：
     * 调用方先 dma_map 得到 IOVA，把 dma_src/dma_dst/dma_len 填好再调 dma_run；
     * dma_src_va/dma_dst_va 仅供调用方标记已映射的 VA（可选，NULL 表示不在析构时
     * 自动 dma_unmap，由调用方自己负责解除映射）。 */
    void *dma_src_va;     /* 已映射的源缓冲（用户虚拟地址），NULL=不自动清理 */
    void *dma_dst_va;     /* 已映射的目的缓冲（用户虚拟地址），NULL=不自动清理 */
    uint64_t dma_src;     /* 源 IOVA */
    uint64_t dma_dst;     /* 目的 IOVA */
    uint32_t dma_len;     /* 长度（字节） */
    int dma_dir;          /* 抽象方向：VFIO_DMA_TO_DEVICE/FROM_DEVICE */
};

#endif
