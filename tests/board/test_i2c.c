#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/i2c/I2c.h>

/*
 * 测试通用用户态 I2C 驱动（I2c 类）的 read / write 接口。
 *
 * 依赖 Linux i2c-dev 驱动（/dev/i2c-N）。
 * 注意：本测试需要真实 I2C 总线（或 QEMU 中模拟的 I2C 控制器）。
 * 若指定总线不存在，open 会失败，测试会失败。
 *
 * 使用 REGISTER_TEST_CMD 注册为命令（与 test_uio_fpga 一致），
 * 仅在具备 /dev/i2c-N 的环境中运行。
 */

/*
 * 测试 I2C 驱动 read / write 接口（单个 case）。
 * 逻辑：先 write 数据到指定寄存器，再 read 同一寄存器，
 * 最后判断 read 读回的值是否等于 write 写入的值。
 * 只有三者都成功，才能证明 I2C 读写事务正确。
 */
static int test_i2c(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    int slave_addr = 0x50;           /* 测试用从机地址 */
    uint8_t reg = 0x00;              /* 起始寄存器地址 */
    uint8_t wbuf[2] = {0xAA, 0xBB};  /* 要写入的数据 */
    uint8_t rbuf[2] = {0};           /* 读回的数据 */
    allocator_t *allocator = allocator_get_default_instance();
    I2c *i2c = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_i2c");

        /* 1. 创建 I2c 对象并打开 /dev/i2c-N */
        i2c = object_new(allocator, "I2c", NULL);
        THROW_IF(i2c == NULL, -1);
        EXEC(i2c->open(i2c, bus));
        dbg_str(DBG_INFO, "i2c open success, bus:%d, fd:%d", bus, i2c->fd);

        /* 2. 写：从寄存器 0x00 开始写 2 个字节 */
        EXEC(i2c->write(i2c, slave_addr, reg, wbuf, sizeof(wbuf)));
        dbg_str(DBG_INFO, "write ok, addr:0x%x, reg:0x%x, data:0x%02x 0x%02x",
                slave_addr, reg, wbuf[0], wbuf[1]);

        /* 3. 读：从寄存器 0x00 开始读 2 个字节 */
        EXEC(i2c->read(i2c, slave_addr, reg, NULL, 0, rbuf, sizeof(rbuf)));
        dbg_str(DBG_INFO, "read ok, addr:0x%x, reg:0x%x, data:0x%02x 0x%02x",
                slave_addr, reg, rbuf[0], rbuf[1]);

        /* 4. 判断读回的值是否等于写入的值 */
        THROW_IF(memcmp(wbuf, rbuf, sizeof(wbuf)) != 0, -1);
        dbg_str(DBG_INFO, "write/read verify ok, data matches");

        /* 5. 关闭 */
        EXEC(i2c->close(i2c));
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (i2c != NULL) {
            object_destroy(i2c);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_i2c);
