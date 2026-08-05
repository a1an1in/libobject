#include <stdio.h>
#include <string.h>
#include <libobject/mockery/mockery.h>
#include <libobject/drivers/uio/Uio.h>

/*
 * 测试 UIO 驱动 + FPGA 寄存器接口。
 *
 * 设备树节点示例：
 *   fpga@50000000 {
 *       compatible = "generic-uio";
 *       reg = <0x0 0x50000000 0x0 0x1000>;
 *       interrupts = <0 70 4>;
 *       interrupt-parent = <&intc>;
 *   };
 *
 * 内核启动参数需包含：uio_pdrv_genirq.of_id=generic-uio
 * 设备名（/sys/class/uio/uioX/name）默认为节点名 "fpga"。
 */
static int test_uio_fpga(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio *uio = NULL;
    uint32_t val = 0, len;
    uint32_t buf[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint32_t rbuf[4] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_uio_fpga");

        /* 1. 创建 Uio 对象 */
        uio = object_new(allocator, "Uio", NULL);
        THROW_IF(uio == NULL, -1);

        /* 2. 打开 UIO 设备（按 /sys/class/uio/uioX/name 匹配 "fpga"） */
        ret = uio->open(uio, "fpga");
        THROW_IF(ret < 0, -1);

        /* 3. mmap 映射 FPGA 寄存器空间 */
        ret = uio->mmap(uio);
        THROW_IF(ret < 0, -1);

        dbg_str(DBG_INFO, "uio size:0x%x", uio->get_size(uio));

        /* 4. 写单个寄存器 */
        ret = uio->write_register(uio, 0x0, 0xDEADBEEF);
        THROW_IF(ret < 0, -1);

        /* 5. 读单个寄存器 */
        ret = uio->read_register(uio, 0x0, &val);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "read register[0x0] = 0x%x", val);

        /* 6. 批量写寄存器 */
        len = sizeof(buf);
        ret = uio->write_registers(uio, 0x10, buf, &len);
        THROW_IF(ret < 0, -1);

        /* 7. 批量读寄存器 */
        len = sizeof(rbuf);
        ret = uio->read_registers(uio, 0x10, rbuf, &len);
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "read registers[0x10] = 0x%x 0x%x 0x%x 0x%x",
                rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

        /* 8. 越界访问保护测试 */
        ret = uio->read_register(uio, 0x100000, &val);
        THROW_IF(ret == 0, -1); /* 期望失败 */

        /* 9. 关闭 */
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
REGISTER_TEST_FUNC(test_uio_fpga);

static int test_uio(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Uio *uio = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_uio");
        uio = object_new(allocator, "Uio", NULL);
        THROW_IF(uio == NULL, -1);
        dbg_str(DBG_INFO, "test_uio: Uio object created success");
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(uio);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_uio);
