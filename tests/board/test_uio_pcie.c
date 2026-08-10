#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/uio/Uio_Pcie.h>

/*
 * 测试 Uio_Pcie 类（基于 UIO 的 PCIe 用户态驱动）——寄存器访问路径。
 *
 * 用 QEMU 的 edu 设备（vendor 0x1234, device 0x11e8，无内核驱动）：
 *   open_device 发现 → bind_uio(0) mmap BAR0 → 读 ID 寄存器(0x00=0x010000ed)
 *   → 写/读 addr4 寄存器(0x04, ~val) 往返校验，完整覆盖寄存器访问路径。
 *
 * 若 guest 未加 `-device edu`（找不到该设备），则跳过寄存器访问，不视为失败。
 * 依赖：内核 CONFIG_UIO_PCI_GENERIC=y + root + QEMU `-device edu`。
 */

/* QEMU edu 设备：vendor/device 与关键寄存器。
 * 注意：PCI_VENDOR_ID_QEMU = 0x1234（include/hw/pci/pci.h），不是 0x1b36 */
#define EDU_VENDOR    0x1234
#define EDU_DEVICE    0x11e8
#define EDU_REG_ID    0x00   /* 读固定值 0x010000ed（源码 0x010000edu 中 'u' 是无符号后缀） */
#define EDU_REG_ADDR4 0x04   /* 写 ~val，读回校验 */
#define EDU_ID_EXPECT 0x010000edULL

static int test_uio_pcie(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio_Pcie *pcie = NULL;
    uint64_t val = 0, wval = 0x12345678;

    TRY {
        dbg_str(DBG_INFO, "test_uio_pcie");

        pcie = object_new(allocator, "Uio_Pcie", NULL);
        THROW_IF(pcie == NULL, -1);

        /* 1. 按 edu 的 vendor/device 打开；未找到则跳过（需 QEMU -device edu） */
        if (pcie->open_device(pcie, EDU_VENDOR, EDU_DEVICE) < 0) {
            dbg_str(DBG_INFO, "edu device not found "
                    "(QEMU 需加 -device edu)，跳过寄存器访问测试");
            ret = 1;
        } else {
            dbg_str(DBG_INFO, "edu device opened");

            /* 2. 绑定 uio_pci_generic + mmap BAR0 */
            EXEC(pcie->bind_uio(pcie, 0));
            EXEC(pcie->set_width(pcie, 32));

            /* 3. 读 ID 寄存器（期望 0x010000ed）。
             *    地址用 pcie_bar_addr(pcie, bar, off) 编码：高位是 BAR、低位是偏移，
             *    多 BAR 设备可同时访问多个 BAR（edu 只有 BAR0，高位为 0）。 */
            EXEC(pcie->read_register(pcie, pcie_bar_addr(pcie, 0, EDU_REG_ID),
                                     &val));
            dbg_str(DBG_INFO, "edu id reg[0x00] = 0x%llx",
                    (unsigned long long)val);
            THROW_IF(val != EDU_ID_EXPECT, -1);

            /* 4. 写 addr4 寄存器再读回校验（addr4 = ~wval）。
             *    注意按 32 位取反：val 是 uint64_t，32 位寄存器读回后高 32 位为 0，
             *    而 ~wval 是 64 位（0xFFFFFFFF...），直接比较会误判失败。 */
            EXEC(pcie->write_register(pcie,
                                      pcie_bar_addr(pcie, 0, EDU_REG_ADDR4),
                                      wval));
            EXEC(pcie->read_register(pcie,
                                     pcie_bar_addr(pcie, 0, EDU_REG_ADDR4),
                                     &val));
            dbg_str(DBG_INFO, "edu addr4 reg[0x04] write 0x%llx, read 0x%llx",
                    (unsigned long long)wval, (unsigned long long)val);
            THROW_IF((uint32_t)val != (uint32_t)~wval, -1);

            dbg_str(DBG_INFO, "edu register read/write round-trip ok");
            ret = 1;
        }
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(pcie);
    }

    return ret;
}
REGISTER_TEST_CMD(test_uio_pcie);
