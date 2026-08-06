#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <libobject/mockery/mockery.h>
#include <libobject/drivers/uio/Fpga.h>

/*
 * 测试 Fpga 类：继承通用 Uio 类。
 *
 * 设备树节点示例：
 *   fpga@b000000 {
 *       compatible = "generic-uio";
 *       reg = <0x0 0xb000000 0x0 0x1000>;
 *       interrupts = <0 70 4>;
 *       interrupt-parent = <&intc>;
 *   };
 *
 * 内核启动参数需包含：uio_pdrv_genirq.of_id=generic-uio
 * 设备名（/sys/class/uio/uioX/name）默认为节点名 "fpga"。
 */

/*
 * 测试 Fpga 写单个寄存器，写完 read 验证是否符合预期。
 */
static int test_fpga_write_register(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Fpga *fpga = NULL;
    uint64_t val = 0;
    uint64_t expect = 0xDEADBEEF;

    TRY {
        dbg_str(DBG_INFO, "test_fpga_write_register");

        /* 1. 创建 Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Fpga", NULL);
        THROW_IF(fpga == NULL, -1);
        dbg_str(DBG_INFO, "step1: Fpga object created");

        /* 2. 打开 FPGA UIO 设备并 mmap（open_device 内部完成 open + mmap） */
        ret = fpga->open_device(fpga, NULL);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "step2: open_device ok");

        /* 3. 写单个寄存器（继承 Uio 的 write_register） */
        dbg_str(DBG_INFO, "step3: before write_register, fpga->write_register=%p", fpga->write_register);
        ret = fpga->write_register(fpga, 0x0, expect);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "step3: write_register ok");

        /* 4. 读回验证是否符合预期 */
        ret = fpga->read_register(fpga, 0x0, &val);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "write register[0x0] = 0x%llx, read back = 0x%llx",
                (unsigned long long)expect, (unsigned long long)val);
        THROW_IF(val != expect, -1);
        dbg_str(DBG_INFO, "step4: read_register ok");

        /* 全部成功，返回成功标志 */
        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(fpga);
    }

    return ret;
}
REGISTER_TEST_CMD(test_fpga_write_register);

/*
 * 测试 Fpga 批量写寄存器，写完 read 验证是否符合预期。
 */
static int test_fpga_write_registers(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Fpga *fpga = NULL;
    uint64_t buf[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint64_t rbuf[4] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_fpga_write_registers");

        /* 1. 创建 Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Fpga", NULL);
        THROW_IF(fpga == NULL, -1);

        /* 2. 打开 FPGA UIO 设备并 mmap（open_device 内部完成 open + mmap） */
        ret = fpga->open_device(fpga, NULL);
        THROW_IF(ret < 0, -1);

        /* 3. 批量写寄存器（继承 Uio 的 write_registers，len 为寄存器个数，返回实际写入个数） */
        ret = fpga->write_registers(fpga, 0x10, buf, 4);
        THROW_IF(ret != 4, -1);

        /* 4. 批量读回验证是否符合预期 */
        ret = fpga->read_registers(fpga, 0x10, rbuf, 4);
        THROW_IF(ret != 4, -1);
        dbg_str(DBG_INFO, "write registers[0x10] = 0x%llx 0x%llx 0x%llx 0x%llx, read back = 0x%llx 0x%llx 0x%llx 0x%llx",
                (unsigned long long)buf[0], (unsigned long long)buf[1],
                (unsigned long long)buf[2], (unsigned long long)buf[3],
                (unsigned long long)rbuf[0], (unsigned long long)rbuf[1],
                (unsigned long long)rbuf[2], (unsigned long long)rbuf[3]);
        THROW_IF(memcmp(buf, rbuf, sizeof(buf)) != 0, -1);

        /* 5. 关闭（close 在 deconstruct 中自动调用，无需手动 close） */

        /* 全部成功，返回成功标志 */
        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(fpga);
    }

    return ret;
}
REGISTER_TEST_CMD(test_fpga_write_registers);

/*
 * 测试 Fpga 中断：使能中断，wait_irq 等待中断并验证。
 *
 * 设备树 fpga 节点配置为 SPI 70 中断（interrupts = <0 70 4>）。
 * QEMU 中新增了 vfpga 模拟设备（hw/misc/vfpga.c），映射到 0x0b000000。
 * 其中 0xFF0 为中断控制寄存器：写 bit0=1 触发中断，写 bit0=0 清除中断。
 *
 * 本测试自动触发中断：先使能中断，再写 0xFF0 触发 SPI 70 中断，
 * 然后 wait_irq 等待并验证中断计数。
 */
static int test_fpga_wait_irq(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Fpga *fpga = NULL;
    int irq_count;

    TRY {
        dbg_str(DBG_INFO, "test_fpga_wait_irq");

        /* 1. 创建 Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Fpga", NULL);
        THROW_IF(fpga == NULL, -1);

        /* 2. 打开 FPGA UIO 设备并 mmap（open_device 内部完成 open + mmap） */
        ret = fpga->open_device(fpga, NULL);
        THROW_IF(ret < 0, -1);

        /* 3. 使能中断（继承 Uio 的 enable_irq） */
        ret = fpga->enable_irq(fpga);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "enable_irq ok");

        /* 4. 写中断控制寄存器 0xFF0 触发 SPI 70 中断（bit0=1） */
        ret = fpga->write_register(fpga, 0xFF0, 1);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "trigger irq ok (write 0xFF0 = 1)");

        /* 5. 等待中断（继承 Uio 的 wait_irq，超时 10 秒） */
        irq_count = fpga->wait_irq(fpga, 10000);
        THROW_IF(irq_count < 0, -1);
        dbg_str(DBG_INFO, "wait_irq ok, irq_count:%d", irq_count);

        /* 6. 清除中断（写 0xFF0 = 0，高电平触发需复位） */
        ret = fpga->write_register(fpga, 0xFF0, 0);
        THROW_IF(ret < 0, -1);

        /* 7. 禁用中断（继承 Uio 的 disable_irq） */
        ret = fpga->disable_irq(fpga);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "disable_irq ok");

        /* 全部成功，返回成功标志 */
        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(fpga);
    }

    return ret;
}
REGISTER_TEST_CMD(test_fpga_wait_irq);
