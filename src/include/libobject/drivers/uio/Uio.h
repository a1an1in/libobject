#ifndef __UIO_H__
#define __UIO_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <libobject/core/Obj.h>

typedef struct Uio_s Uio;

struct Uio_s {
    Obj parent;

    int (*construct)(Uio *,char *);
    int (*deconstruct)(Uio *);

    /*virtual methods reimplement*/
    int (*set)(Uio *module, char *attrib, void *value);
    void *(*get)(Uio *, char *attrib);
    char *(*to_json)(Uio *); 

    /* UIO device interface */
    int (*open)(Uio *uio, char *name);
    int (*mmap)(Uio *uio);
    int (*get_size)(Uio *uio);
    int (*close)(Uio *uio);

    /* FPGA register interface (built on top of UIO mmap)
     * offset 为字节偏移，使用 uint64_t 以支持 64 位地址空间（aarch64 物理地址为 64 位）
     * data/value 为 64 位寄存器值，实际位宽由 width 属性控制（32 或 64，默认 32）
     */
    int (*set_width)(Uio *uio, int width);   /* 设置寄存器位宽：32 或 64 */
    int (*read_register)(Uio *uio, uint64_t offset, uint64_t *data);
    int (*write_register)(Uio *uio, uint64_t offset, uint64_t data);
    /* len 为期望读写的寄存器个数，返回值为实际读写的寄存器个数（失败返回负数） */
    int (*read_registers)(Uio *uio, uint64_t offset, uint64_t *data, uint32_t len);
    int (*write_registers)(Uio *uio, uint64_t offset, uint64_t *data, uint32_t len);

    /* interrupt interface */
    int (*enable_irq)(Uio *uio);
    int (*disable_irq)(Uio *uio);
    int (*wait_irq)(Uio *uio, int timeout_ms);

    /*attribs*/
    int fd;                 /* /dev/uioX file descriptor */
    char *dev_path;         /* /dev/uioX path */
    char *name;             /* UIO device name (matched via /sys/class/uio) */
    uint8_t *base;          /* mmap'd virtual base address (byte addressing) */
    uint32_t size;          /* mapped region size in bytes */
    int width;              /* register width in bits: 32 or 64, default 32 */
    int irq_enabled;        /* whether interrupt is enabled */
};

#endif
