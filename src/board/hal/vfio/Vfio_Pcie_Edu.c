/**
 * @file Vfio_Pcie_Edu.c
 * @Synopsis QEMU edu 教学设备（vendor 0x1234, device 0x11e8）的具体 VFIO 驱动。
 * 继承 Vfio_Pcie 的通用机制（发现/BAR/寄存器访问、dma_config 通用配置、dma_copy
 * 便捷搬运），只实现 edu 特有的 DMA 触发 dma_run：
 *   - dma_run：把 dma_config 记录的 (dma_src, dma_dst, dma_len) 通过 edu 的
 *     SRC/DST/CNT/CMD 寄存器做两段中转搬运（guest→dma_buf→guest）；
 *     完成通知用 **DMA 完成中断**（CMD bit2=EDU_DMA_IRQ 置位 → DMA_IRQ）：
 *       register_irq（eventfd + io_worker）→ 中断处理函数清设备中断并置 volatile
 *       完成标志；dma_run 轮询该标志（带超时），无锁、无需额外 eventfd。
 * 注意：edu DMA 寄存器是 32 位，搬运前需 set_width(32)。
 * @author alan lin
 * @version
 * @date 2026-08-12
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libobject/board/hal/vfio/Vfio_Pcie_Edu.h>
#include <libobject/core/utils/dbg/debug.h>

#define EDU_VENDOR    0x1234
#define EDU_DEVICE    0x11e8

/* edu DMA 寄存器（BAR0 内偏移） */
#define EDU_DMA_SRC       0x80
#define EDU_DMA_DST       0x88
#define EDU_DMA_CNT       0x90
#define EDU_DMA_CMD       0x98
#define EDU_DMA_START     0x40000   /* dma_buf 起始（BAR0 内偏移，地址约定） */
#define EDU_DMA_BUF_SIZE  4096      /* edu dma_buf 大小 */
#define EDU_DMA_RUN       0x1       /* CMD bit0：触发 */
#define EDU_DMA_TO_PCI    0x2       /* CMD bit1：1=写 dma_buf→guest 内存 */
#define EDU_DMA_FROM_PCI  0x0       /* CMD bit1：0=读 guest 内存→dma_buf */
#define EDU_DMA_IRQ       0x4       /* CMD bit2：DMA 完成时发 DMA_IRQ 中断 */

/* edu 中断寄存器（BAR0 内偏移） */
#define EDU_IRQ_STATUS    0x24      /* 读：中断状态 */
#define EDU_IRQ_CLEAR     0x64      /* 写：写 1 清除对应中断位（deassert INTx） */
#define EDU_DMA_IRQ_BIT   0x100     /* 中断状态里的 DMA 完成位（DMA_IRQ） */

/*
 * DMA 完成中断处理函数（register_irq 注册，io_worker 异步调用）。
 * opaque 为 io_worker 传入的 Worker*，worker->opaque 即 edu 对象。
 * 职责：清设备中断（写 0x64 清 DMA_IRQ，使 INTx 电平回落）→ 置 volatile 完成标志。
 * 无锁：仅写一个 volatile 标志，等待方轮询该标志。
 */
static int __edu_dma_irq_handler(void *opaque)
{
    /* opaque 为 io_worker 传入的 Worker*；worker->opaque 是该向量的上下文，
     * 用 vfio_irq_get_vfio() 取回 Vfio 对象 */
    Vfio *vfio = vfio_irq_get_vfio(opaque);
    Vfio_Pcie_Edu *edu = (Vfio_Pcie_Edu *)vfio;
    uint64_t irq_status = 0;

    /* 读 irq_status(0x24)，若有 DMA 完成位则写 0x64 清除（清掉 irq_status 位） */
    if (edu->parent.read_register(&edu->parent,
            vfio_pcie_bar_addr(&edu->parent, 0, EDU_IRQ_STATUS),
            &irq_status) == 0 &&
        (irq_status & EDU_DMA_IRQ_BIT)) {
        edu->parent.write_register(&edu->parent,
            vfio_pcie_bar_addr(&edu->parent, 0, EDU_IRQ_CLEAR), EDU_DMA_IRQ_BIT);
    }
    /* INTx 电平触发（AUTOMASKED）需 unmask 才能收下一次；MSI 边沿触发无需 */
    if (edu->dma_irq_mode == EDU_IRQ_INTX) {
        vfio->unmask_irq(vfio, 0, 0);
    }

    /* 置完成标志（volatile，无锁；dma_run 轮询此标志） */
    edu->dma_done = 1;
    return 0;
}

/* 等待 DMA 完成中断（无锁：轮询中断回调置位的 volatile 完成标志），超时返回 -1 */
static int __wait_dma_done(Vfio_Pcie_Edu *edu)
{
    int i, max_loops;
    uint64_t irq_status = 0;

    max_loops = (edu->dma_timeout_ms * 1000) / edu->dma_wait_us;
    for (i = 0; i < max_loops; i++) {
        if (edu->dma_done) {
            edu->dma_done = 0;   /* 消费完成标志，为下一次 DMA 复位 */
            return 0;
        }
        usleep(edu->dma_wait_us);
    }
    /* 超时诊断：读 irq_status(0x24)，区分"DMA 未完成"与"DMA 已完成但中断未送达"。
     * edu_raise_irq() 先置 irq_status 再 msi_notify/pci_set_irq，
     * 所以只要 DMA 完成，0x24 里 DMA_IRQ 位必然置位——与中断是否送达无关。 */
    if (edu->parent.read_register(&edu->parent,
            vfio_pcie_bar_addr(&edu->parent, 0, EDU_IRQ_STATUS),
            &irq_status) == 0) {
        dbg_str(DBG_ERROR, "edu dma irq wait timeout, irq_status(0x24)=0x%llx%s",
                (unsigned long long)irq_status,
                (irq_status & EDU_DMA_IRQ_BIT) ?
                    " (DMA_IRQ pending -> DMA 已完成，但完成中断未送达)" :
                    " (DMA_IRQ clear -> DMA 可能根本没完成)");
    } else {
        dbg_str(DBG_ERROR, "edu dma irq wait timeout, read irq_status(0x24) failed");
    }
    return -1;
}

/*
 * dma_run（override Vfio.dma_run）：执行 dma_config 记录的两段式内存搬运，
 * 用 DMA 完成中断 + volatile 标志等待（替代轮询设备寄存器）。
 * edu 硬件只能在 guest 内存与内部 dma_buf 之间搬运（不支持 guest→guest 直接
 * mem-to-mem），故把 guest(dma_src) → dma_buf → guest(dma_dst) 分成两段触发：
 *   第 1 段：guest(dma_src) → dma_buf（设备读 guest，CMD bit1=0）
 *   第 2 段：dma_buf → guest(dma_dst)（设备写 guest，CMD bit1=1）
 * 每段 CMD 都置 EDU_DMA_IRQ，完成时 edu 发 DMA_IRQ → 中断回调置完成标志。
 */
static int __dma_run(Vfio_Pcie_Edu *edu)
{
    Vfio *vfio = (Vfio *)edu;
    Vfio_Pcie *pcie = (Vfio_Pcie *)edu;

    if (edu == NULL || pcie->width != 32) {
        dbg_str(DBG_ERROR, "edu dma_run failed, need width=32 "
                "(先 set_width(32))");
        return -1;
    }
    if (pcie->dma_len == 0 || pcie->dma_len > EDU_DMA_BUF_SIZE) {
        dbg_str(DBG_ERROR, "edu dma_run failed, bad len:%u (1~%d)",
                pcie->dma_len, EDU_DMA_BUF_SIZE);
        return -1;
    }

    /* 懒注册 DMA 完成中断：按 dma_irq_mode 选择 INTx(0)/MSI(1)，vector 0 */
    if (!edu->dma_irq_registered) {
        int irq_index = (edu->dma_irq_mode == EDU_IRQ_MSI) ? 1 : 0;
        if (vfio->register_irq(vfio, irq_index, 0,
                               __edu_dma_irq_handler, edu) < 0) {
            dbg_str(DBG_ERROR, "edu register dma irq failed");
            return -1;
        }
        edu->dma_irq_registered = 1;
    }

    /* 第 1 段：guest(dma_src) → dma_buf（先清残留中断，CMD 带 EDU_DMA_IRQ） */
    if (pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_IRQ_CLEAR),
                             EDU_DMA_IRQ_BIT) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_SRC),
                             pcie->dma_src) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_DST),
                             EDU_DMA_START) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_CNT),
                             (uint64_t)pcie->dma_len) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_CMD),
                             EDU_DMA_RUN | EDU_DMA_FROM_PCI | EDU_DMA_IRQ) < 0 ||
        __wait_dma_done(edu) < 0) {
        return -1;
    }

    /* 第 2 段：dma_buf → guest(dma_dst) */
    if (pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_IRQ_CLEAR),
                             EDU_DMA_IRQ_BIT) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_SRC),
                             EDU_DMA_START) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_DST),
                             pcie->dma_dst) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_CNT),
                             (uint64_t)pcie->dma_len) < 0 ||
        pcie->write_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_DMA_CMD),
                             EDU_DMA_RUN | EDU_DMA_TO_PCI | EDU_DMA_IRQ) < 0 ||
        __wait_dma_done(edu) < 0) {
        return -1;
    }
    return 0;
}

static int __construct(Vfio_Pcie_Edu *module, char *init_str)
{
    module->dma_timeout_ms = 5000;
    module->dma_wait_us = 1000;
    module->dma_irq_mode = EDU_IRQ_INTX;
    module->dma_done = 0;
    module->dma_irq_registered = 0;
    return 0;
}

static int __deconstruct(Vfio_Pcie_Edu *module)
{
    return 0;
}

/*
 * Vfio_Pcie_Edu 注册接口。
 * 接口继承：dma_config/dma_copy 用 value=NULL 继承 Vfio_Pcie 的通用实现；
 * dma_run 用实际实现覆盖（设备相关），经 __object_override_virtual_func 自动
 * 覆盖基类字段，通过 (Vfio *)edu 或 edu->parent 调用都会派发到本实现。
 */
DEFINE_CLASS(
    EXTENDS(Vfio_Pcie_Edu, Vfio_Pcie),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(dma_config, NULL),
    Class_VFunc_Entry(dma_run, __dma_run),
    Class_VFunc_Entry(dma_copy, NULL)
);
