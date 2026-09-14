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
 * 继承 Vfio_Pcie 的通用机制（发现/BAR/寄存器访问、dma_* 入参块），只实现 edu 特有的
 * DMA 触发 dma_run：
 *   - dma_run：读取调用方填好的 Vfio_Pcie.dma_src/dma_dst/dma_len，通过 edu 的
 *     SRC/DST/CNT/CMD 寄存器做两段中转搬运（guest→dma_buf→guest）并等待完成。
 *   （主机内存端由调用方先 dma_map 得到 IOVA 并填入 dma_*，调用方负责 dma_unmap。）
 *
 * 注意：edu DMA 寄存器是 32 位，region_read/region_write 固定传 reg_width=32。
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

    /* override Vfio.dma_run：读取 Vfio_Pcie.dma_src/dma_dst/dma_len 触发 edu 两段搬运 */
    int (*dma_run)(Vfio_Pcie_Edu *);

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
