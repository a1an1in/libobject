#ifndef __UIO_H__
#define __UIO_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>

typedef struct Uio_s Uio;

/*
 * UIO 异步中断处理函数（即 io_worker 的 work_callback）。
 * opaque 为 io_worker 传入的 worker，可通过 ((Worker *)opaque)->opaque
 * 获取 register_irq 时传入的用户数据。
 */
typedef int (*uio_irq_handler_t)(void *opaque);

struct Uio_s {
    Obj parent;

    int (*construct)(Uio *,char *);
    int (*deconstruct)(Uio *);

    /*virtual methods reimplement*/
    int (*set)(Uio *module, char *attrib, void *value);
    void *(*get)(Uio *, char *attrib);
    char *(*to_json)(Uio *); 

    /* UIO device interface */
    /* 打开 UIO 设备：统一按设备路径打开（如 "/dev/uio0"）。
     * 若只有设备名（如 "fpga"），先用 uio_find_dev() 解析成路径 */
    int (*open)(Uio *uio, char *dev_path);
    /* 映射 uio->map_index 指定的 map（默认 0）。内部从 /sys/class/uio/uioX/maps/mapN/
     * 读取该 map 的大小；mmap offset = map 序号 * PAGE_SIZE（UIO 约定，
     * map0 → 0，mapN → N*PAGE_SIZE）。PCIe 多 BAR 设备设置 uio->map_index = BAR 序号后复用 */
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
    /* 注册异步中断处理函数（基于 io_worker），中断到来时异步调用 handler */
    int (*register_irq)(Uio *uio, uio_irq_handler_t handler, void *opaque);

    /*attribs*/
    int fd;                 /* /dev/uioX file descriptor */
    char *dev_path;         /* /dev/uioX path */
    char *name;             /* UIO device name (matched via /sys/class/uio) */
    uint8_t *base;          /* mmap'd virtual base address (byte addressing) */
    uint64_t size;          /* mapped region size in bytes */
    int map_index;          /* mmap 的 map 序号（默认 0，PCIe 多 BAR 设备用它选择 BAR） */
    int width;              /* register width in bits: 32 or 64, default 32 */
    int irq_enabled;        /* whether interrupt is enabled */

    /* async irq (io_worker) fields */
    Worker *irq_worker;     /* io_worker 监听 /dev/uioX 中断事件 */
    uio_irq_handler_t irq_handler;  /* 用户注册的中断处理函数 */
    void *irq_opaque;       /* 传给中断处理函数的用户数据 */
    uint32_t irq_count;     /* 最近一次中断的中断计数（供 handler 读取） */
};

/* 工具函数：按 UIO 设备名（/sys/class/uio/uioX/name）查找对应的 /dev/uioX 路径。
 * 返回 0 成功并把路径写入 dev_path；找不到返回 -1。 */
int uio_find_dev(const char *name, char *dev_path, int dev_path_len);

#endif
