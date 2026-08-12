#ifndef __VFIO_PCIE_EDU_H__
#define __VFIO_PCIE_EDU_H__

#include <stdio.h>
#include <stdint.h>
#include <libobject/board/hal/vfio/Vfio_Pcie.h>

typedef struct Vfio_Pcie_Edu_s Vfio_Pcie_Edu;

/* edu DMA 完成中断方式（可切换） */
enum {
    EDU_IRQ_INTX = 0,   /* INTx：电平触发，AUTOMASKED 需 handler 里 unmask */
    EDU_IRQ_MSI  = 1,   /* MSI：边沿触发，无需 unmask（edu 单向量 MSI） */
};

/*
 * Vfio_Pcie_Edu 类：QEMU edu 教学设备（vendor 0x1234, device 0x11e8）的具体 VFIO 驱动。
 *
 * 继承 Vfio_Pcie 的通用机制（发现/BAR/寄存器访问、dma_config 通用配置、dma_copy
 * 便捷搬运），只实现 edu 特有的 DMA 触发 dma_run：
 *   - dma_run：把 dma_config 记录的 (dma_src, dma_dst, dma_len) 通过 edu 的
 *     SRC/DST/CNT/CMD 寄存器做两段中转搬运（guest→dma_buf→guest）并等待完成。
 *   - dma_config / dma_copy：继承 Vfio_Pcie 的通用实现（DEFINE_CLASS 里 value 为 NULL）。
 *
 * 注意：edu DMA 寄存器是 32 位，搬运前需 set_width(32)。
 *
 * 继承关系：Obj -> Vfio -> Vfio_Pcie -> Vfio_Pcie_Edu
 */

struct Vfio_Pcie_Edu_s {
    Vfio_Pcie parent;

    /*virtual methods reimplement*/
    int (*construct)(Vfio_Pcie_Edu *, char *);
    int (*deconstruct)(Vfio_Pcie_Edu *);
    int (*set)(Vfio_Pcie_Edu *module, char *attrib, void *value);
    void *(*get)(Vfio_Pcie_Edu *, char *attrib);
    char *(*to_json)(Vfio_Pcie_Edu *);

    /* 以下接口继承 Vfio_Pcie（DEFINE_CLASS 里 value 为 NULL，dma_run 除外） */
    int (*dma_config)(Vfio_Pcie_Edu *, void *buf_src, void *buf_dst,
                      uint32_t len, int direction);
    int (*dma_run)(Vfio_Pcie_Edu *);
    int (*dma_copy)(Vfio_Pcie_Edu *, void *buf_src, void *buf_dst,
                    uint32_t len);

    /*attribs*/
    int dma_timeout_ms;  /* 完成超时（ms），默认 5000 */
    int dma_wait_us;     /* 完成标志轮询间隔（us），默认 1000 */
    int dma_irq_mode;    /* 中断方式：EDU_IRQ_INTX(0)/EDU_IRQ_MSI(1)，默认 INTx */
    /* DMA 完成中断同步（无锁：中断回调置 volatile 完成标志，dma_run 轮询该标志）。
     * 中断本身经 VFIO register_irq 的 eventfd + io_worker 到达本对象，无需额外 eventfd。 */
    volatile int dma_done;         /* 完成标志（中断回调置 1，dma_run 轮询清零） */
    int dma_irq_registered;        /* DMA 完成中断是否已注册（懒注册） */
};

#endif
