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
 *   2. map_region(0) 映射 region 0（= BAR0，IOMMU 隔离，替代 /dev/mem）
 *   3. 读 ID 寄存器验证 region mmap 路径
 *   4. 分配两个页对齐缓冲 buf_src/buf_dst
 *   5. 用 dma_map + dma_run 做"内存到内存"搬运（库只提供通用 dma_map/dma_unmap 与
 *      设备相关 dma_run；映射/解映射由本测试自己负责，不再有 dma_config/dma_copy）：
 *        dma_map(buf_src) → iova_src；dma_map(buf_dst) → iova_dst   （CPU 侧准备）
 *        填 pcie->dma_src/dma_dst/dma_len                           （投递入参块）
 *        dma_run()：触发搬运并等待完成；edu 内部经 dma_buf 两段中转
 *          第 1 段（VFIO_DMA_TO_DEVICE）：guest(iova_src) → edu dma_buf
 *          第 2 段（VFIO_DMA_FROM_DEVICE）：edu dma_buf → guest(iova_dst)
 *      测试在 FINALLY 里自己 dma_unmap ×2（不依赖析构自动清理）
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
    uint64_t src_iova = 0, dst_iova = 0;
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

            /* 2. 映射 region 0（= BAR0，region mmap；BAR n = region n） */
            EXEC(vfio->map_region(vfio, 0));

            /* 3. 读 ID 寄存器（通用 region_read：index=0=BAR0；位宽用已配置的
             *    默认值——edu 在 construct 里 set_width(32)；期望 0x010000ed） */
            EXEC(vfio->region_read(vfio, 0, EDU_REG_ID, &val));
            dbg_str(DBG_INFO, "edu id reg[0x00] = 0x%llx",
                    (unsigned long long)val);
            THROW_IF(val != EDU_ID_EXPECT, -1);

            /* 4. 分配两个页对齐缓冲（下面 dma_map 会映射它们，需页对齐） */
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

            /* 6. 内存到内存搬运：dma_map 准备 IOVA → 填 dma_* 入参块 → dma_run 触发
             *    - 中断方式可切换：dma_irq_mode = EDU_IRQ_INTX / EDU_IRQ_MSI；
             *      默认 INTx（本 QEMU TCG 环境 MSI→ITS→LPI 投递不可用，
             *      register_irq 能注册、设备会发 MSI，但 guest 收不到中断；
             *      改为 EDU_IRQ_MSI 可在 KVM/真实硬件上验证，边沿触发无需 unmask）；
             *    - dma_map    ：把主机内存 buf_src/buf_dst 映射为 IOVA（CPU 侧准备）；
             *    - dma_* 入参块：把 IOVA/长度投递给设备（dma_run 读取执行）；
             *    - dma_run    ：触发搬运并等待完成（edu 内部经 dma_buf 两段中转）。
             *      映射由本测试负责解除（下面显式 dma_unmap ×2）。 */
            ((Vfio_Pcie_Edu *)pcie)->dma_irq_mode = EDU_IRQ_INTX;
            dbg_str(DBG_INFO, "edu dma irq mode = %s",
                    (((Vfio_Pcie_Edu *)pcie)->dma_irq_mode == EDU_IRQ_MSI)
                    ? "MSI" : "INTx");
            EXEC(vfio->dma_map(vfio, buf_src, (uint64_t)len, &src_iova));
            EXEC(vfio->dma_map(vfio, buf_dst, (uint64_t)len, &dst_iova));
            pcie->dma_src = src_iova;
            pcie->dma_dst = dst_iova;
            pcie->dma_len = (uint32_t)len;
            pcie->dma_dir = VFIO_DMA_TO_DEVICE;
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
        /* 异常路径下若仍有未解除的 IOVA 映射，补一次 dma_unmap（此时对象仍有效） */
        if (vfio != NULL) {
            if (src_iova != 0) {
                vfio->dma_unmap(vfio, src_iova, (uint64_t)len);
                src_iova = 0;
            }
            if (dst_iova != 0) {
                vfio->dma_unmap(vfio, dst_iova, (uint64_t)len);
                dst_iova = 0;
            }
        }
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
