#ifndef __UIO_PCIE_FPGA_H__
#define __UIO_PCIE_FPGA_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/uio/Uio_Pcie.h>

/*
 * Uio_Pcie_Fpga 类：PCIe 板卡上的 FPGA 专用驱动，继承 Uio_Pcie。
 *
 * 不新增接口，直接复用 Uio_Pcie 的全部能力（open_device/bind_uio/
 * read-write_register(s)/配置空间/中断 等），作为 PCIe FPGA 的基类，
 * 后续可在此扩展具体 FPGA 型号的接口。
 *
 * 典型流程：
 *   Uio_Pcie_Fpga *fpga = object_new(allocator, "Uio_Pcie_Fpga", NULL);
 *   fpga->open_device(fpga, 0x1234, 0x11e8);   // 设备发现
 *   fpga->bind_uio(fpga, 0);                    // 绑定 + 映射 BAR0
 *   fpga->set_width(fpga, 32);
 *   fpga->read_register(fpga, pcie_bar_addr(fpga, 0, 0x00), &val);
 *
 * 继承关系：Obj -> Uio -> Uio_Pcie -> Uio_Pcie_Fpga
 */

typedef struct Uio_Pcie_Fpga_s Uio_Pcie_Fpga;

struct Uio_Pcie_Fpga_s {
    Uio_Pcie parent;

    int (*construct)(Uio_Pcie_Fpga *, char *);
    int (*deconstruct)(Uio_Pcie_Fpga *);

    /*virtual methods reimplement*/
    int (*set)(Uio_Pcie_Fpga *module, char *attrib, void *value);
    void *(*get)(Uio_Pcie_Fpga *, char *attrib);
    char *(*to_json)(Uio_Pcie_Fpga *);

    /* 以下接口继承 Uio_Pcie（DEFINE_CLASS 里 value 为 NULL） */
    int (*open_device)(Uio_Pcie_Fpga *fpga, uint32_t vendor, uint32_t device);
    int (*get_info)(Uio_Pcie_Fpga *fpga, pcie_dev_info_t *info);
    int (*read_config)(Uio_Pcie_Fpga *fpga, int offset, uint32_t *data);
    int (*write_config)(Uio_Pcie_Fpga *fpga, int offset, uint32_t data);
    int (*bind_uio)(Uio_Pcie_Fpga *fpga, int bar);
    int (*get_size)(Uio_Pcie_Fpga *fpga);
    int (*set_width)(Uio_Pcie_Fpga *fpga, int width);
    int (*read_register)(Uio_Pcie_Fpga *fpga, uint64_t offset, uint64_t *data);
    int (*write_register)(Uio_Pcie_Fpga *fpga, uint64_t offset, uint64_t data);
    /* len 为期望读写的寄存器个数，返回值为实际读写的寄存器个数（失败返回负数） */
    int (*read_registers)(Uio_Pcie_Fpga *fpga, uint64_t offset, uint64_t *data,
                          uint32_t len);
    int (*write_registers)(Uio_Pcie_Fpga *fpga, uint64_t offset, uint64_t *data,
                           uint32_t len);
    /* interrupt interface (inherited from Uio_Pcie) */
    int (*enable_irq)(Uio_Pcie_Fpga *fpga);
    int (*disable_irq)(Uio_Pcie_Fpga *fpga);
    int (*register_irq)(Uio_Pcie_Fpga *fpga, uio_irq_handler_t handler,
                        void *opaque);
};

#endif
