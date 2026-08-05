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

    /* FPGA register interface (built on top of UIO mmap) */
    int (*read_register)(Uio *uio, uint32_t offset, uint32_t *data);
    int (*write_register)(Uio *uio, uint32_t offset, uint32_t data);
    int (*read_registers)(Uio *uio, uint32_t offset, uint32_t *data, uint32_t *len);
    int (*write_registers)(Uio *uio, uint32_t offset, uint32_t *data, uint32_t *len);

    /* interrupt interface */
    int (*enable_irq)(Uio *uio);
    int (*disable_irq)(Uio *uio);
    int (*wait_irq)(Uio *uio, int timeout_ms);

    /*attribs*/
    int fd;                 /* /dev/uioX file descriptor */
    char *dev_path;         /* /dev/uioX path */
    char *name;             /* UIO device name (matched via /sys/class/uio) */
    uint32_t *base;         /* mmap'd virtual base address (32-bit word addressing) */
    uint32_t size;          /* mapped region size in bytes */
    int irq_enabled;        /* whether interrupt is enabled */
};

#endif
