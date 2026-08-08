#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/service/eeprom_storage/Eeprom_Storage.h>

/*
 * 测试基于 EEPROM 的 key-value 存储服务（Eeprom_Storage 类）。
 *
 * 设计：
 *   - 底层 EEPROM 中存储 JSON 数据（用 cjson 序列化/反序列化）
 *   - 给用户的接口是 key-value 结构：set(key, value) / get(key)
 *   - 组件保留 JSON 缓存：set/get 直接读写缓存（内存），
 *     load/save 才读写 ROM（EEPROM）
 *
 * 依赖 Linux i2c-dev 驱动（/dev/i2c-N）和 I2C EEPROM 从机。
 * 在 QEMU virt 机器中，已挂载 at24c02 EEPROM（地址 0x50，256 字节）。
 *
 * 使用 REGISTER_TEST_CMD 注册为命令（与 test_i2c_eeprom 一致）。
 */

/*
 * 测试 Eeprom_Storage 的 key-value 接口和缓存机制。
 * 逻辑：
 *   1. set_kv 写入多个 key-value（写缓存，不写 ROM）
 *   2. get_kv 读取（从缓存）
 *   3. save 写回 ROM
 *   4. 重新 load 验证持久化
 */
static int test_eeprom_storage(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    char *val = NULL;
    allocator_t *allocator = allocator_get_default_instance();
    Eeprom_Storage *es = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_eeprom_storage");

        /* 1. 创建 Eeprom_Storage 对象并初始化（打开底层 EEPROM 设备） */
        es = object_new(allocator, "Eeprom_Storage", NULL);
        THROW_IF(es == NULL, -1);
        EXEC(es->init(es, bus, 0x50, 256));
        dbg_str(DBG_INFO, "eeprom_storage init success, bus:%d, size:%d",
                bus, es->size);

        /* 2. set_value 写入多个 key-value（写缓存，不立即写 ROM） */
        EXEC(es->set_value(es, "device_name", "my_device"));
        EXEC(es->set_value(es, "firmware_version", "1.0.0"));
        EXEC(es->set_value(es, "serial_number", "SN123456"));
        dbg_str(DBG_INFO, "set_value ok, 3 keys written to cache");

        /* 3. get_value 读取（从缓存） */
        val = es->get_value(es, "device_name");
        THROW_IF(val == NULL, -1);
        THROW_IF(strcmp(val, "my_device") != 0, -1);
        dbg_str(DBG_INFO, "get_value device_name ok, value:%s", val);
        free(val);
        val = NULL;

        val = es->get_value(es, "firmware_version");
        THROW_IF(val == NULL, -1);
        THROW_IF(strcmp(val, "1.0.0") != 0, -1);
        dbg_str(DBG_INFO, "get_value firmware_version ok, value:%s", val);
        free(val);
        val = NULL;

        /* 4. 更新已有 key（替换缓存中的值） */
        EXEC(es->set_value(es, "firmware_version", "1.0.1"));
        val = es->get_value(es, "firmware_version");
        THROW_IF(val == NULL, -1);
        THROW_IF(strcmp(val, "1.0.1") != 0, -1);
        dbg_str(DBG_INFO, "update firmware_version ok, value:%s", val);
        free(val);
        val = NULL;

        /* 5. save 写回 ROM */
        EXEC(es->save(es));
        dbg_str(DBG_INFO, "save ok, cache written to ROM");

        /* 6. 重新 load 验证持久化（模拟重启后从 ROM 恢复） */
        EXEC(es->load(es));
        val = es->get_value(es, "device_name");
        THROW_IF(val == NULL, -1);
        THROW_IF(strcmp(val, "my_device") != 0, -1);
        dbg_str(DBG_INFO, "load verify device_name ok, value:%s", val);
        free(val);
        val = NULL;

        val = es->get_value(es, "firmware_version");
        THROW_IF(val == NULL, -1);
        THROW_IF(strcmp(val, "1.0.1") != 0, -1);
        dbg_str(DBG_INFO, "load verify firmware_version ok, value:%s", val);
        free(val);
        val = NULL;

        /* 7. 关闭在 deconstruct 中自动调用（无需显式 close） */
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (val != NULL) {
            free(val);
        }
        if (es != NULL) {
            object_destroy(es);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_eeprom_storage);
