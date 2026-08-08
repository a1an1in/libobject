#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/service/Storage_Service.h>

/*
 * 测试存储服务（Storage_Service 类）。
 *
 * 存储服务基于 HAL 层设备类（I2c_EEPROM），提供设备无关的 save/load 接口。
 * 用户只调用 save()/load()，不关心底层是 EEPROM、Flash 还是其他设备。
 *
 * 依赖 Linux i2c-dev 驱动（/dev/i2c-N）和 I2C EEPROM 从机。
 * 在 QEMU virt 机器中，已挂载 at24c02 EEPROM（地址 0x50，256 字节）。
 *
 * 使用 REGISTER_TEST_CMD 注册为命令（与 test_i2c_eeprom 一致）。
 */

/*
 * 测试存储服务 save/load（单个 case）。
 * 逻辑：先 save 数据到指定地址，再 load 同一地址，
 * 最后判断 load 读回的值是否等于 save 写入的值。
 */
static int test_storage_service(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    uint16_t addr = 0x00;            /* 存储地址 */
    uint8_t wbuf[4] = {0xAA, 0xBB, 0xCC, 0xDD};  /* 要保存的数据 */
    uint8_t rbuf[4] = {0};           /* 读回的数据 */
    allocator_t *allocator = allocator_get_default_instance();
    Storage_Service *svc = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_storage_service");

        /* 1. 创建 Storage_Service 对象并初始化（打开底层 EEPROM 设备） */
        svc = object_new(allocator, "Storage_Service", NULL);
        THROW_IF(svc == NULL, -1);
        EXEC(svc->init(svc, bus, 0x50, 256));
        dbg_str(DBG_INFO, "storage service init success, bus:%d, size:%d",
                bus, svc->size);

        /* 2. 保存：从地址 0x00 保存 4 字节 */
        EXEC(svc->save(svc, addr, wbuf, sizeof(wbuf)));
        dbg_str(DBG_INFO, "storage save ok, addr:0x%x, data:0x%02x 0x%02x 0x%02x 0x%02x",
                addr, wbuf[0], wbuf[1], wbuf[2], wbuf[3]);

        /* 3. 读取：从地址 0x00 读取 4 字节 */
        EXEC(svc->load(svc, addr, rbuf, sizeof(rbuf)));
        dbg_str(DBG_INFO, "storage load ok, addr:0x%x, data:0x%02x 0x%02x 0x%02x 0x%02x",
                addr, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

        /* 4. 判断读回的值是否等于保存的值 */
        THROW_IF(memcmp(wbuf, rbuf, sizeof(wbuf)) != 0, -1);
        dbg_str(DBG_INFO, "storage save/load verify ok, data matches");

        /* 5. 关闭在 deconstruct 中自动调用（无需显式 close） */
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (svc != NULL) {
            object_destroy(svc);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_storage_service);
