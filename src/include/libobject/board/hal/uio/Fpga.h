#ifndef __FPGA_H__
#define __FPGA_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/uio/Uio.h>

/*
 * Fpga 类：在通用 Uio 之上封装的 FPGA 专用驱动。
 *
 * 继承 Uio 的通用功能（open/mmap/close/set_width 等），
 * 并添加 FPGA 特有的接口（open_device：默认设备名 "fpga"），
 * 以及寄存器读写、中断接口（复用 Uio 的实现）。
 *
 * 设备树节点示例：
 *   fpga@b000000 {
 *       compatible = "generic-uio";
 *       reg = <0x0 0xb000000 0x0 0x1000>;
 *       interrupts = <0 70 4>;
 *       interrupt-parent = <&intc>;
 *   };
 */

typedef struct Fpga_s Fpga;

struct Fpga_s {
    Uio parent;

    int (*construct)(Fpga *,char *);
    int (*deconstruct)(Fpga *);

    /*virtual methods reimplement*/
    int (*set)(Fpga *module, char *attrib, void *value);
    void *(*get)(Fpga *, char *attrib);
    char *(*to_json)(Fpga *); 

    /* FPGA interface (built on top of Uio)
     * open_device 打开 FPGA UIO 设备并 mmap，name 为空时默认 "fpga"。
     * 其余接口复用 Uio 的实现。close 在 deconstruct 中自动调用。
     */
    int (*open_device)(Fpga *fpga, char *name);
    /* UIO device interface (inherited from Uio) */
    int (*get_size)(Fpga *fpga);
    /* FPGA register interface */
    int (*set_width)(Fpga *fpga, int width);
    int (*read_register)(Fpga *fpga, uint64_t offset, uint64_t *data);
    int (*write_register)(Fpga *fpga, uint64_t offset, uint64_t data);
    /* len 为期望读写的寄存器个数，返回值为实际读写的寄存器个数（失败返回负数） */
    int (*read_registers)(Fpga *fpga, uint64_t offset, uint64_t *data, uint32_t len);
    int (*write_registers)(Fpga *fpga, uint64_t offset, uint64_t *data, uint32_t len);

    /* FPGA interrupt interface */
    int (*enable_irq)(Fpga *fpga);
    int (*disable_irq)(Fpga *fpga);
    int (*register_irq)(Fpga *fpga, uio_irq_handler_t handler, void *opaque);

    /*attribs*/
    char *device_name;   /* FPGA UIO 设备名，默认 "fpga" */
};

#endif
