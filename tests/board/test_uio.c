#include <stdio.h>
#include <string.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/uio/Uio.h>
#include <libobject/board/hal/uio/Uio_Fpga.h>

/*
 * 测试 UIO 驱动 + FPGA 寄存器接口。
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
 *
 * 注意：本测试依赖 QEMU 中的 vfpga 模拟设备（hw/misc/vfpga.c），
 * 因此使用 REGISTER_TEST_CMD 注册为命令（而非 REGISTER_TEST_FUNC），
 * 仅在 QEMU 环境中运行。
 */

/*
 * 打开 UIO 设备并 mmap 映射 FPGA 寄存器空间。
 * 返回已初始化的 Uio 对象，失败返回 NULL。
 */
static Uio *__uio_open_and_mmap(allocator_t *allocator)
{
    Uio *uio = NULL;
    char dev_path[128] = {0};
    int ret;

    TRY {
        /* 1. 创建 Uio 对象 */
        uio = object_new(allocator, "Uio", NULL);
        THROW_IF(uio == NULL, -1);

        /* 2. 按设备名 "fpga" 解析成 /dev/uioX 并打开（统一按 /dev 路径） */
        ret = uio_find_dev("fpga", dev_path, sizeof(dev_path));
        THROW_IF(ret < 0, -1);
        ret = uio->open(uio, dev_path);
        THROW_IF(ret < 0, -1);

        /* 3. mmap 映射 FPGA 寄存器空间 */
        ret = uio->mmap(uio);
        THROW_IF(ret < 0, -1);

        dbg_str(DBG_INFO, "uio open success, size:0x%x", uio->get_size(uio));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "uio open/mmap failed");
        if (uio != NULL) {
            object_destroy(uio);
            uio = NULL;
        }
    }

    return uio;
}

/*
 * 测试写单个寄存器（默认 32 位），写完 read 验证是否符合预期。
 */
static int test_write_register(Uio *uio)
{
    int ret = 1;
    uint64_t val = 0;
    uint64_t expect = 0xDEADBEEF;

    TRY {
        /* 1. 写单个寄存器（默认 32 位） */
        ret = uio->write_register(uio, 0x0, expect);
        THROW_IF(ret < 0, -1);

        /* 2. 读回验证是否符合预期 */
        ret = uio->read_register(uio, 0x0, &val);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "write register[0x0] = 0x%llx, read back = 0x%llx",
                (unsigned long long)expect, (unsigned long long)val);
        THROW_IF(val != expect, -1);

        /* 3. 越界访问保护测试 */
        ret = uio->read_register(uio, 0x100000, &val);
        THROW_IF(ret == 0, -1); /* 期望失败 */

        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test write_register failed");
    }

    return ret;
}

/*
 * 测试批量写寄存器（默认 32 位），写完 read 验证是否符合预期。
 */
static int test_write_registers(Uio *uio)
{
    int ret = 1;
    uint64_t buf[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint64_t rbuf[4] = {0};

    TRY {
        /* 1. 批量写寄存器（默认 32 位，len 为寄存器个数，返回实际写入个数） */
        ret = uio->write_registers(uio, 0x10, buf, 4);
        THROW_IF(ret != 4, -1);

        /* 2. 批量读回验证是否符合预期 */
        ret = uio->read_registers(uio, 0x10, rbuf, 4);
        THROW_IF(ret != 4, -1);
        dbg_str(DBG_INFO, "write registers[0x10] = 0x%llx 0x%llx 0x%llx 0x%llx, read back = 0x%llx 0x%llx 0x%llx 0x%llx",
                (unsigned long long)buf[0], (unsigned long long)buf[1],
                (unsigned long long)buf[2], (unsigned long long)buf[3],
                (unsigned long long)rbuf[0], (unsigned long long)rbuf[1],
                (unsigned long long)rbuf[2], (unsigned long long)rbuf[3]);
        THROW_IF(memcmp(buf, rbuf, sizeof(buf)) != 0, -1);

        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test write_registers failed");
    }

    return ret;
}

/*
 * 测试 64 位位宽：写单个 64 位寄存器，写完 read 验证是否符合预期。
 */
static int test_write_register_64(Uio *uio)
{
    int ret = 1;
    uint64_t val = 0;
    uint64_t expect = 0xDEADBEEF12345678ULL;

    TRY {
        /* 1. 设置 64 位寄存器位宽 */
        ret = uio->set_width(uio, 64);
        THROW_IF(ret < 0, -1);

        /* 2. 写单个 64 位寄存器 */
        ret = uio->write_register(uio, 0x0, expect);
        THROW_IF(ret < 0, -1);

        /* 3. 读回验证是否符合预期 */
        ret = uio->read_register(uio, 0x0, &val);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "write 64-bit register[0x0] = 0x%llx, read back = 0x%llx",
                (unsigned long long)expect, (unsigned long long)val);
        THROW_IF(val != expect, -1);

        /* 4. 越界访问保护测试 */
        ret = uio->read_register(uio, 0x100000, &val);
        THROW_IF(ret == 0, -1); /* 期望失败 */

        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test write_register_64 failed");
    }

    return ret;
}

/*
 * 测试 64 位位宽：批量写 64 位寄存器，写完 read 验证是否符合预期。
 */
static int test_write_registers_64(Uio *uio)
{
    int ret = 1;
    uint64_t buf[4] = {0x1111111122222222ULL, 0x3333333344444444ULL,
                       0x5555555566666666ULL, 0x7777777788888888ULL};
    uint64_t rbuf[4] = {0};

    TRY {
        /* 1. 设置 64 位寄存器位宽 */
        ret = uio->set_width(uio, 64);
        THROW_IF(ret < 0, -1);

        /* 2. 批量写 64 位寄存器（len 为寄存器个数，返回实际写入个数） */
        ret = uio->write_registers(uio, 0x10, buf, 4);
        THROW_IF(ret != 4, -1);

        /* 3. 批量读回验证是否符合预期 */
        ret = uio->read_registers(uio, 0x10, rbuf, 4);
        THROW_IF(ret != 4, -1);
        dbg_str(DBG_INFO, "write 64-bit registers[0x10] = 0x%llx 0x%llx 0x%llx 0x%llx, read back = 0x%llx 0x%llx 0x%llx 0x%llx",
                (unsigned long long)buf[0], (unsigned long long)buf[1],
                (unsigned long long)buf[2], (unsigned long long)buf[3],
                (unsigned long long)rbuf[0], (unsigned long long)rbuf[1],
                (unsigned long long)rbuf[2], (unsigned long long)rbuf[3]);
        THROW_IF(memcmp(buf, rbuf, sizeof(buf)) != 0, -1);

        ret = 1;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test write_registers_64 failed");
    }

    return ret;
}

/*
 * 测试 UIO 驱动 + FPGA 寄存器接口（单个 case，依次执行各子测试）。
 * 依赖 QEMU 中的 vfpga 模拟设备，使用 REGISTER_TEST_CMD 注册。
 */
static int test_uio(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio *uio = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_uio");

        /* 1. 打开 UIO 设备并 mmap */
        uio = __uio_open_and_mmap(allocator);
        THROW_IF(uio == NULL, -1);

        /* 2. 写单个寄存器（32 位） */
        ret = test_write_register(uio);
        THROW_IF(ret < 0, -1);

        /* 3. 批量写寄存器（32 位） */
        ret = test_write_registers(uio);
        THROW_IF(ret < 0, -1);

        /* 4. 写单个 64 位寄存器 */
        ret = test_write_register_64(uio);
        THROW_IF(ret < 0, -1);

        /* 5. 批量写 64 位寄存器 */
        ret = test_write_registers_64(uio);
        THROW_IF(ret < 0, -1);

        /* 6. 关闭 */
        ret = uio->close(uio);
        THROW_IF(ret < 0, -1);

        /* 全部成功，返回成功标志 */
        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(uio);
    }

    return ret;
}
REGISTER_TEST_CMD(test_uio);
