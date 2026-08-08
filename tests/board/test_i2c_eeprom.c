#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/i2c/I2c_EEPROM.h>

/*
 * 测试 I2C EEPROM 驱动（I2c_EEPROM 类）。
 *
 * 依赖 Linux i2c-dev 驱动（/dev/i2c-N）和 I2C EEPROM 从机。
 * 在 QEMU virt 机器中，已挂载 at24c02 EEPROM（地址 0x50，256 字节）。
 *
 * 使用 REGISTER_TEST_CMD 注册为命令（与 test_fpga 一致）。
 */

/*
 * 测试 I2C EEPROM 读写（单个 case）。
 * 逻辑：先 write 数据到指定地址，再 read 同一地址，
 * 最后判断 read 读回的值是否等于 write 写入的值。
 */
static int test_i2c_eeprom(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    uint16_t addr = 0x00;            /* EEPROM 存储地址 */
    uint8_t wbuf[4] = {0xAA, 0xBB, 0xCC, 0xDD};  /* 要写入的数据 */
    uint8_t rbuf[4] = {0};           /* 读回的数据 */
    allocator_t *allocator = allocator_get_default_instance();
    I2c_EEPROM *eeprom = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_i2c_eeprom");

        /* 1. 创建 I2c_EEPROM 对象并初始化（打开总线 + 配置从机地址 + 容量） */
        eeprom = object_new(allocator, "I2c_EEPROM", NULL);
        THROW_IF(eeprom == NULL, -1);
        EXEC(eeprom->init(eeprom, bus, 0x50, 256));
        dbg_str(DBG_INFO, "eeprom init success, bus:%d, fd:%d, "
                "slave_addr:0x%x, size:%d",
                bus, eeprom->parent.fd, eeprom->slave_addr, eeprom->size);

        /* 2. 写：从地址 0x00 写 4 字节 */
        EXEC(eeprom->write(eeprom, addr, wbuf, sizeof(wbuf)));
        dbg_str(DBG_INFO, "eeprom write ok, addr:0x%x, data:0x%02x 0x%02x 0x%02x 0x%02x",
                addr, wbuf[0], wbuf[1], wbuf[2], wbuf[3]);

        /* 3. 读：从地址 0x00 读 4 字节 */
        EXEC(eeprom->read(eeprom, addr, rbuf, sizeof(rbuf)));
        dbg_str(DBG_INFO, "eeprom read ok, addr:0x%x, data:0x%02x 0x%02x 0x%02x 0x%02x",
                addr, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

        /* 4. 判断读回的值是否等于写入的值 */
        THROW_IF(memcmp(wbuf, rbuf, sizeof(wbuf)) != 0, -1);
        dbg_str(DBG_INFO, "eeprom write/read verify ok, data matches");

        /* 5. 关闭在 deconstruct 中自动调用（无需显式 close） */
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (eeprom != NULL) {
            object_destroy(eeprom);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_i2c_eeprom);
