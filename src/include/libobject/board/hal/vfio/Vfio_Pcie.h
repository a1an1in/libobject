#ifndef __VFIO_PCIE_H__
#define __VFIO_PCIE_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/vfio/Vfio.h>

/*
 * Vfio_Pcie 类：基于 VFIO 的 PCIe 设备用户态驱动。
 *
 * 继承 Vfio 的通用机制（三层 fd、region mmap、eventfd 中断、DMA 映射），
 * 只新增 PCIe 特有功能：
 *   - open_device：按 vendor/device ID 在 /sys/bus/pci/devices 下发现设备，
 *     解析其 iommu_group（/sys/bus/pci/devices/<BDF>/iommu_group → /dev/vfio/<N>），
 *     绑定 vfio-pci 驱动，再调用父类 open(group_path, device_name)。
 *   - map_bar：把指定 BAR（= region 索引）映射为访问基址，多 BAR 并存。
 *   - 寄存器访问：地址高位是 region/BAR 序号、低位是 BAR 内偏移（偏移编码，
 *     与 Uio_Pcie.pcie_bar_addr 一致），可同时访问多个 BAR。
 *
 * 典型流程：
 *   Vfio_Pcie *p = object_new(allocator, "Vfio_Pcie", NULL);
 *   p->open_device(p, 0x1234, 0x11e8);   // 发现 edu + 绑定 vfio-pci + 三层 fd
 *   p->map_bar(p, 0);                    // 映射 BAR0
 *   p->read_register(p, vfio_pcie_bar_addr(p, 0, 0x00), &val);
 *   p->dma_map(p, buf, size, &iova);     // 共享内存 DMA
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
    /* 按 vendor/device ID 发现设备 + 绑定 vfio-pci + 打开三层 fd */
    int (*open_device)(Vfio_Pcie *pcie, uint32_t vendor, uint32_t device);
    /* 映射指定 BAR（= region 索引），多 BAR 并存 */
    int (*map_bar)(Vfio_Pcie *pcie, int bar);

    /* register interface (built on top of region mmap) */
    int (*set_width)(Vfio_Pcie *pcie, int width);
    /* 寄存器访问：offset 高位是 BAR 序号、低位是 BAR 内偏移（同 Uio_Pcie 偏移编码） */
    int (*read_register)(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data);
    int (*write_register)(Vfio_Pcie *pcie, uint64_t offset, uint64_t data);
    int (*read_registers)(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data,
                          uint32_t len);
    int (*write_registers)(Vfio_Pcie *pcie, uint64_t offset, uint64_t *data,
                           uint32_t len);

    /* 设备级 DMA：dma_config 在此提供通用实现（入参为用户缓冲，内部 dma_map ×2
     * 并记录到 dma_* 属性，不触发）；dma_run 是接口声明（默认继承 Vfio 的不支持
     * 实现），由具体设备类（如 Vfio_Pcie_Edu）override（设备相关触发）。 */
    int (*dma_config)(Vfio_Pcie *pcie, void *buf_src, void *buf_dst,
                      uint32_t len, int direction);
    int (*dma_run)(Vfio_Pcie *pcie);
    /* 通用便捷 DMA 搬运：dma_config+dma_run → 解除映射复位，一行完成 */
    int (*dma_copy)(Vfio_Pcie *pcie, void *buf_src, void *buf_dst, uint32_t len);

    /*attribs*/
    char *bdf;            /* 总线:设备.功能，如 "0000:00:02.0" */
    int width;            /* 寄存器位宽：32 或 64，默认 32 */
    int bar_shift;        /* 地址编码位数：低 bar_shift 位是 BAR 内偏移，高位是 BAR 序号 */
    uint64_t bar_size[6]; /* 各 BAR 大小（字节），0 表示该 BAR 无效 */
    int bar_mapped[6];    /* 各 BAR 是否已映射（0/1） */
    /* DMA 搬运配置（dma_config 记录，dma_run 读取执行） */
    void *dma_src_va;     /* 已映射的源缓冲（用户虚拟地址），NULL=未映射 */
    void *dma_dst_va;     /* 已映射的目的缓冲（用户虚拟地址），NULL=未映射 */
    uint64_t dma_src;     /* 源 IOVA */
    uint64_t dma_dst;     /* 目的 IOVA */
    uint32_t dma_len;     /* 长度（字节） */
    int dma_dir;          /* 抽象方向：VFIO_DMA_TO_DEVICE/FROM_DEVICE */
};

/* 把 (BAR 序号, BAR 内偏移) 编码成寄存器访问地址：高位 BAR、低位偏移。
 * 与 Uio_Pcie.pcie_bar_addr 一致。 */
static inline uint64_t vfio_pcie_bar_addr(Vfio_Pcie *pcie, int bar, uint64_t offset)
{
    return ((uint64_t)bar << pcie->bar_shift) | offset;
}

#endif
