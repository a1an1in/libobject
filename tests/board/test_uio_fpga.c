#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/uio/Uio_Fpga.h>

/*
 * 测试 Uio_Fpga 类：继承通用 Uio 类。
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
 * 测试 Uio_Fpga 写单个寄存器，写完 read 验证是否符合预期。
 */
static int test_uio_fpga_write_register(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio_Fpga *fpga = NULL;
    uint64_t val = 0;
    uint64_t expect = 0xDEADBEEF;
    char dev_path[128] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_uio_fpga_write_register");

        /* 1. 创建 Uio_Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Uio_Fpga", NULL);
        THROW_IF(fpga == NULL, -1);
        dbg_str(DBG_INFO, "step1: Uio_Fpga object created");

        /* 2. 按设备名 "fpga" 解析出 /dev/uioX 路径，再 open_device(dev_path) */
        ret = uio_find_dev("fpga", dev_path, sizeof(dev_path));
        THROW_IF(ret < 0, -1);
        ret = fpga->open_device(fpga, dev_path);
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
REGISTER_TEST_CMD(test_uio_fpga_write_register);

/*
 * 测试 Uio_Fpga 批量写寄存器，写完 read 验证是否符合预期。
 */
static int test_uio_fpga_write_registers(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio_Fpga *fpga = NULL;
    uint64_t buf[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint64_t rbuf[4] = {0};
    char dev_path[128] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_uio_fpga_write_registers");

        /* 1. 创建 Uio_Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Uio_Fpga", NULL);
        THROW_IF(fpga == NULL, -1);

        /* 2. 按设备名 "fpga" 解析出 /dev/uioX 路径，再 open_device(dev_path) */
        ret = uio_find_dev("fpga", dev_path, sizeof(dev_path));
        THROW_IF(ret < 0, -1);
        ret = fpga->open_device(fpga, dev_path);
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
REGISTER_TEST_CMD(test_uio_fpga_write_registers);

/*
 * 异步中断处理函数（即 io_worker 的 work_callback）。
 * opaque 为 io_worker 传入的 worker，通过 ((Worker *)opaque)->opaque
 * 获取 register_irq 时传入的用户数据（此处为 Uio_Fpga 对象）。
 * 中断次数由 __irq_ev_callback 保存到 uio->irq_count，此处读取并打印。
 */
static int test_uio_fpga_irq_handler(void *opaque)
{
    Worker *worker = (Worker *)opaque;
    Uio_Fpga *fpga = (Uio_Fpga *)worker->opaque;
    Uio *uio = (Uio *)fpga;

    dbg_str(DBG_INFO, "async irq handler called ok, fpga:%p, irq_count:%u",
            fpga, uio->irq_count);

    return 0;
}

/*
 * 测试 Uio_Fpga 异步中断：使能中断，register_irq 注册异步中断处理函数，
 * 触发中断后由 io_worker 异步调用 handler。
 *
 * 设备树 fpga 节点配置为 SPI 70 中断（interrupts = <0 70 4>）。
 * QEMU 中新增了 vfpga 模拟设备（hw/misc/vfpga.c），映射到 0x0b000000。
 * 其中 0xFF0 为中断控制寄存器：写 bit0=1 触发中断，写 bit0=0 清除中断。
 *
 * 本测试自动触发中断：先使能中断，再写 0xFF0 触发 SPI 70 中断，
 * 然后轮询等待异步 handler 被调用。
 */
static int test_uio_fpga_irq(TEST_ENTRY *entry)
{
    int ret = 1;
    int i;
    allocator_t *allocator = allocator_get_default_instance();
    Uio_Fpga *fpga = NULL;
    char dev_path[128] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_uio_fpga_irq");

        /* 1. 创建 Uio_Fpga 对象（继承 Uio） */
        fpga = object_new(allocator, "Uio_Fpga", NULL);
        THROW_IF(fpga == NULL, -1);

        /* 2. 按设备名 "fpga" 解析出 /dev/uioX 路径，再 open_device(dev_path) */
        ret = uio_find_dev("fpga", dev_path, sizeof(dev_path));
        THROW_IF(ret < 0, -1);
        EXEC(fpga->open_device(fpga, dev_path));

        /* 3. 使能中断（继承 Uio 的 enable_irq） */
        EXEC(fpga->enable_irq(fpga));
        dbg_str(DBG_INFO, "enable_irq ok");

        /* 4. 注册异步中断处理函数（继承 Uio 的 register_irq，基于 io_worker） */
        EXEC(fpga->register_irq(fpga, test_uio_fpga_irq_handler, fpga));
        dbg_str(DBG_INFO, "register_irq ok");

        /* 5. 写中断控制寄存器 0xFF0 触发 SPI 70 中断（bit0=1） */
        EXEC(fpga->write_register(fpga, 0xFF0, 1));
        dbg_str(DBG_INFO, "trigger irq ok (write 0xFF0 = 1)");

        /* 6. 轮询等待异步 handler 被调用（最多 10 秒），
         * 中断次数由 __irq_ev_callback 保存到 uio->irq_count */
        for (i = 0; i < 1000 && fpga->parent.irq_count == 0; i++) {
            usleep(10000);
        }
        THROW_IF(fpga->parent.irq_count == 0, -1);
        dbg_str(DBG_INFO, "async irq handled ok, irq_count:%u", fpga->parent.irq_count);

        /* 7. 清除中断（写 0xFF0 = 0，高电平触发需复位） */
        EXEC(fpga->write_register(fpga, 0xFF0, 0));

        /* 8. 禁用中断（继承 Uio 的 disable_irq） */
        EXEC(fpga->disable_irq(fpga));
        dbg_str(DBG_INFO, "disable_irq ok");
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(fpga);
    }

    return ret;
}
REGISTER_TEST_CMD(test_uio_fpga_irq);
