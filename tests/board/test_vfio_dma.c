#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/vfio/Vfio_Pcie_Edu.h>

/*
 * 测试 Vfio_Pcie_Edu 类（QEMU edu 设备的具体 VFIO 驱动）——DMA 演示。
 *
 * 用 QEMU 的 edu 设备（vendor 0x1234, device 0x11e8）+ SMMUv3（IOMMU）：
 *   1. open_device 发现 + 绑定 vfio-pci + 三层 fd（container/group/device）
 *   2. map_bar(0) 映射 BAR0（region mmap，IOMMU 隔离，替代 /dev/mem）
 *   3. 读 ID 寄存器验证 region mmap 路径
 *   4. 分配两个页对齐缓冲 buf_src/buf_dst
 *   5. 用 dma_config/dma_run 做"内存到内存"搬运（显式使用配置/触发接口）：
 *        dma_config(buf_src, buf_dst, len, dir)：
 *          内部 dma_map ×2（用户缓冲 → IOVA）+ 记录配置，不触发
 *        dma_run()：触发搬运并等待完成；edu 内部经 dma_buf 两段中转
 *          第 1 段（VFIO_DMA_TO_DEVICE）：guest(iova_src) → edu dma_buf
 *          第 2 段（VFIO_DMA_FROM_DEVICE）：edu dma_buf → guest(iova_dst)
 *      解除映射由析构自动完成（Vfio_Pcie.__deconstruct 自动 dma_unmap ×2）
 *   6. 校验 buf_dst 内容 == buf_src 原数据（验证 IOVA 读 + IOVA 写两条 DMA 路径）
 *
 * 注意：edu 的 dma_buf 是设备内部缓冲，guest 不能通过 BAR mmap 直接读写它，所以
 * 演示用"src 内存→dma_buf→dst 内存"的两段式搬运，最终结果落在 guest 可读的 buf_dst 上。
 *
 * 若 guest 未加 `-device edu`（找不到设备）、或设备未落在 iommu_group
 * （需 -M virt,iommu=smmuv3 + 绑定 vfio-pci），则跳过，不视为失败。
 * 依赖：内核 CONFIG_VFIO + CONFIG_VFIO_IOMMU_TYPE1 + CONFIG_VFIO_PCI +
 * CONFIG_ARM_SMMU_V3 + root + QEMU `-M virt,iommu=smmuv3 -device edu`。
 */

/* QEMU edu 设备：vendor/device 与 ID 寄存器（hw/misc/edu.c） */
#define EDU_VENDOR    0x1234
#define EDU_DEVICE    0x11e8
#define EDU_REG_ID    0x00   /* 读固定值 0x010000ed */
#define EDU_ID_EXPECT 0x010000edULL

static int test_vfio_dma(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Vfio_Pcie *pcie = NULL;
    Vfio *vfio = NULL;
    uint64_t val = 0;
    int len = 4096, i;
    uint8_t *buf_src = NULL;
    uint8_t *buf_dst = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_vfio_dma");

        /* 1. 创建 Vfio_Pcie_Edu 对象并打开 edu（发现 + 绑定 vfio-pci + 三层 fd）。
         *    未找到 edu 或未落在 iommu_group 则跳过（需 SMMUv3） */
        pcie = (Vfio_Pcie *)object_new(allocator, "Vfio_Pcie_Edu", NULL);
        THROW_IF(pcie == NULL, -1);
        vfio = (Vfio *)pcie;

        if (pcie->open_device(pcie, EDU_VENDOR, EDU_DEVICE) < 0) {
            dbg_str(DBG_INFO, "edu device not found / no iommu_group "
                    "(QEMU 需 -M virt,iommu=smmuv3 -device edu，并绑 vfio-pci)，"
                    "跳过 VFIO DMA 测试");
            ret = 1;
        } else {
            dbg_str(DBG_INFO, "edu device opened via VFIO, num_regions:%u, "
                    "num_irqs:%u", vfio->info.num_regions, vfio->info.num_irqs);

            /* 2. 映射 BAR0（region mmap）+ 寄存器位宽（edu DMA 寄存器是 32 位） */
            EXEC(pcie->map_bar(pcie, 0));
            EXEC(pcie->set_width(pcie, 32));

            /* 3. 读 ID 寄存器（验证 region mmap 路径，期望 0x010000ed） */
            EXEC(pcie->read_register(pcie, vfio_pcie_bar_addr(pcie, 0, EDU_REG_ID),
                                     &val));
            dbg_str(DBG_INFO, "edu id reg[0x00] = 0x%llx",
                    (unsigned long long)val);
            THROW_IF(val != EDU_ID_EXPECT, -1);

            /* 4. 分配两个页对齐缓冲（dma_config 内部会 dma_map） */
            buf_src = mmap(NULL, len, PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            THROW_IF(buf_src == MAP_FAILED, -1);
            buf_dst = mmap(NULL, len, PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            THROW_IF(buf_dst == MAP_FAILED, -1);

            /* 5. 填充源数据（buf_src 放 00 01 02 ... FF 循环），清空目标 */
            for (i = 0; i < len; i++) {
                buf_src[i] = (uint8_t)(i & 0xff);
            }
            memset(buf_dst, 0, len);

            /* 6. 内存到内存搬运：用 dma_config/dma_run 显式执行
             *    - 中断方式可切换：dma_irq_mode = EDU_IRQ_INTX / EDU_IRQ_MSI；
             *      默认 INTx（本 QEMU TCG 环境 MSI→ITS→LPI 投递不可用，
             *      register_irq 能注册、设备会发 MSI，但 guest 收不到中断；
             *      改为 EDU_IRQ_MSI 可在 KVM/真实硬件上验证，边沿触发无需 unmask）；
             *    - dma_config：映射 src/dst 缓冲 → IOVA 并记录配置（内部 dma_map ×2，
             *      不触发；同配置可反复 dma_run 复用，此处演示一次）；
             *    - dma_run    ：触发搬运并等待完成（edu 内部经 dma_buf 两段中转）。 */
            ((Vfio_Pcie_Edu *)pcie)->dma_irq_mode = EDU_IRQ_INTX;
            dbg_str(DBG_INFO, "edu dma irq mode = %s",
                    (((Vfio_Pcie_Edu *)pcie)->dma_irq_mode == EDU_IRQ_MSI)
                    ? "MSI" : "INTx");
            EXEC(pcie->dma_config(pcie, buf_src, buf_dst, len,
                                 VFIO_DMA_TO_DEVICE));
            EXEC(pcie->dma_run(pcie));

            dbg_str(DBG_INFO, "edu DMA done, src[0..3] = %02x %02x %02x %02x, "
                    "dst[0..3] = %02x %02x %02x %02x",
                    buf_src[0], buf_src[1], buf_src[2], buf_src[3],
                    buf_dst[0], buf_dst[1], buf_dst[2], buf_dst[3]);
            THROW_IF(memcmp(buf_dst, buf_src, len) != 0, -1);
        }
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (buf_src != NULL && buf_src != MAP_FAILED) {
            munmap(buf_src, len);
        }
        if (buf_dst != NULL && buf_dst != MAP_FAILED) {
            munmap(buf_dst, len);
        }
        object_destroy(pcie);
    }

    return ret;
}
REGISTER_TEST_CMD(test_vfio_dma);
