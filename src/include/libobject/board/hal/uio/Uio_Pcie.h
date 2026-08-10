#ifndef __UIO_PCIE_H__
#define __UIO_PCIE_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/uio/Uio.h>

/*
 * Uio_Pcie 类：基于 UIO（uio_pci_generic）+ /dev/mem 的 PCIe 设备用户态驱动。
 *
 * 复用 Uio 的打开（open("/dev/uioX")，仅用于中断）、位宽、中断接口；
 * BAR 映射用 /dev/mem（arm64 + Linux 4.9 上 uio_pci_generic 无 info.mem[]、
 * 无 HAVE_PCI_MMAP，Uio.mmap 与 resourceN 都不可用）。Uio_Pcie 新增：
 *   - open_device：按 vendor/device ID 在 /sys/bus/pci/devices 下查找 PCIe
 *     设备（只读，无需绑定 UIO）。
 *   - get_info：读取 vendor/device/class/revision/irq/BAR 大小等 PCI 信息。
 *   - read_config / write_config：读写 PCI 配置空间
 *     （/sys/bus/pci/devices/<BDF>/config）。
 *   - bind_uio(bar)：把设备绑定到 uio_pci_generic，找到 /dev/uioX，并从
 *     /sys/bus/pci/devices/<BDF>/resource 取 BAR 物理基址，mmap /dev/mem 映射
 *     该 BAR。可多次 bind_uio 映射多个 BAR 并存。
 *   - 寄存器访问（Uio_Pcie 覆盖）：地址高 bar_shift 位是 BAR 序号、低位是 BAR 内
 *     偏移（同 mono 的 RBS_FPGA_PCI_BAR_OFFSET），可同时访问多个 BAR。
 *
 * 典型流程：
 *   Uio_Pcie *pcie = object_new(allocator, "Uio_Pcie", NULL);
 *   pcie->open_device(pcie, 0x1af4, 0x1000);   // 按 vendor:device 查找
 *   pcie->get_info(pcie, &info);               // 查看设备信息/BAR
 *   pcie->bind_uio(pcie, 0);                   // 绑定 uio_pci_generic + mmap BAR0
 *   pcie->bind_uio(pcie, 2);                   // 再映射 BAR2（多 BAR 并存）
 *   pcie->read_register(pcie, pcie_bar_addr(pcie, 0, 0x00), &val); // 读 BAR0 寄存器
 *   pcie->read_register(pcie, pcie_bar_addr(pcie, 2, 0x04), &val); // 读 BAR2 寄存器
 *
 * 继承关系：Obj -> Uio -> Uio_Pcie
 */

typedef struct Uio_Pcie_s Uio_Pcie;

/* PCI 设备信息（来自 sysfs） */
typedef struct pcie_dev_info {
    uint32_t vendor;      /* 厂商 ID */
    uint32_t device;      /* 设备 ID */
    uint32_t class;       /* 类别/子类/编程接口（0xMMNNPP） */
    uint32_t revision;    /* 修订版本 */
    int      irq;         /* 中断号，-1 表示无 */
    int      num_bars;    /* 有效 BAR 数量 */
    uint64_t bar_size[6]; /* 各 BAR 大小（字节），0 表示该 BAR 无效 */
} pcie_dev_info_t;

struct Uio_Pcie_s {
    Uio parent;

    int (*construct)(Uio_Pcie *, char *);
    int (*deconstruct)(Uio_Pcie *);

    /*virtual methods reimplement*/
    int (*set)(Uio_Pcie *module, char *attrib, void *value);
    void *(*get)(Uio_Pcie *, char *attrib);
    char *(*to_json)(Uio_Pcie *);

    /* PCIe interface (built on top of Uio) */
    /* 按 vendor/device ID 自动查找 BDF 并打开（sysfs 发现，不绑定 UIO） */
    int (*open_device)(Uio_Pcie *pcie, uint32_t vendor, uint32_t device);
    /* 读取 PCI 设备信息 */
    int (*get_info)(Uio_Pcie *pcie, pcie_dev_info_t *info);
    /* 读写 PCI 配置空间（offset 字节偏移，数据为 32 位） */
    int (*read_config)(Uio_Pcie *pcie, int offset, uint32_t *data);
    int (*write_config)(Uio_Pcie *pcie, int offset, uint32_t data);
    /* 绑定 uio_pci_generic + /dev/mem 映射指定 BAR；可多次调用映射多个 BAR 并存 */
    int (*bind_uio)(Uio_Pcie *pcie, int bar);

    /* UIO device interface (inherited from Uio) */
    int (*get_size)(Uio_Pcie *pcie);
    int (*set_width)(Uio_Pcie *pcie, int width);
    int (*read_register)(Uio_Pcie *pcie, uint64_t offset, uint64_t *data);
    int (*write_register)(Uio_Pcie *pcie, uint64_t offset, uint64_t data);
    /* len 为期望读写的寄存器个数，返回值为实际读写的寄存器个数（失败返回负数） */
    int (*read_registers)(Uio_Pcie *pcie, uint64_t offset, uint64_t *data, uint32_t len);
    int (*write_registers)(Uio_Pcie *pcie, uint64_t offset, uint64_t *data, uint32_t len);
    /* interrupt interface (inherited from Uio) */
    int (*enable_irq)(Uio_Pcie *pcie);
    int (*disable_irq)(Uio_Pcie *pcie);
    int (*register_irq)(Uio_Pcie *pcie, uio_irq_handler_t handler, void *opaque);

    /*attribs*/
    char *bdf;            /* 总线:设备.功能，如 "0000:01:00.0" */
    int bar;              /* 最近映射的 BAR 索引（-1 表示未映射） */
    int bar_shift;        /* 地址编码位数：低 bar_shift 位是 BAR 内偏移，高位是 BAR 序号 */
    uint64_t bar_size[6]; /* 各 BAR 大小（字节），0 表示该 BAR 无效 */
    uint64_t bar_addr[6]; /* 各 BAR 物理基址（/sys/bus/pci/devices/<BDF>/resource 起始地址） */
    uint32_t bar_flags[6];/* 各 BAR flags（bit0=1 为 I/O BAR，bit0=0 为 Memory BAR） */
    uint8_t *bar_base[6]; /* 各 BAR 访问基址（虚拟地址，含页内偏移；寄存器访问用） */
    uint8_t *map_base[6]; /* 各 BAR /dev/mem 页对齐 mmap 基址（munmap 用），NULL 表示未映射 */
    uint64_t bar_mapped_size[6]; /* 各 BAR 实际映射大小（页对齐） */
    int uio_num;          /* 绑定的 /dev/uioX 编号，-1 表示未绑定 */
};

/* 把 (BAR 序号, BAR 内偏移) 编码成寄存器访问地址：高位 BAR、低位偏移。
 * 同 mono 的 RBS_FPGA_PCI_BAR_OFFSET(bar) = bar << 24，只是移位量 bar_shift
 * 由 bind_uio 按最大 BAR 大小动态确定。 */
static inline uint64_t pcie_bar_addr(Uio_Pcie *pcie, int bar, uint64_t offset)
{
    return ((uint64_t)bar << pcie->bar_shift) | offset;
}

#endif
