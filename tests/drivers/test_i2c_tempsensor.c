#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/drivers/i2c/I2c_TempSensor.h>

/*
 * 测试 I2C 温度传感器驱动（I2c_TempSensor 类）。
 *
 * 依赖 Linux i2c-dev 驱动（/dev/i2c-N）和 I2C 温度传感器从机。
 * 在 QEMU virt 机器中，需挂载 tmp105 温度传感器（地址 0x48）。
 *
 * 使用 REGISTER_TEST_CMD 注册为命令（与 test_fpga 一致）。
 */

/*
 * 测试 I2C 温度传感器读取（tmp105，单个 case）。
 * 逻辑：init 打开总线，read_temp 读取温度，验证温度值合理。
 */
static int test_i2c_tempsensor_tmp105(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    float temp = 0;
    allocator_t *allocator = allocator_get_default_instance();
    I2c_TempSensor *sensor = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_i2c_tempsensor_tmp105");

        /* 1. 创建 I2c_TempSensor 对象并初始化（打开总线 + 配置从机地址） */
        sensor = object_new(allocator, "I2c_TempSensor", NULL);
        THROW_IF(sensor == NULL, -1);
        EXEC(sensor->init(sensor, bus, 0x48));
        dbg_str(DBG_INFO, "tempsensor init success, bus:%d, fd:%d, "
                "slave_addr:0x%x",
                bus, sensor->parent.fd, sensor->slave_addr);

        /* 2. 读温度 */
        EXEC(sensor->read(sensor, &temp));
        dbg_str(DBG_INFO, "tempsensor read ok, temp:%.2f C", temp);

        /* 3. 验证温度值合理（-55 ~ 125°C，tmp105 量程） */
        THROW_IF(temp < -55.0f || temp > 125.0f, -1);
        dbg_str(DBG_INFO, "tempsensor temp verify ok, temp:%.2f C", temp);

        /* 4. 关闭在 deconstruct 中自动调用（无需显式 close） */
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (sensor != NULL) {
            object_destroy(sensor);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_i2c_tempsensor_tmp105);

/*
 * 测试 I2C 温度传感器驱动（I2c_TempSensor 类）的通用性。
 *
 * 本用例使用与 tmp105 参数不同的 emc1413 温度传感器（地址 0x4C）：
 *   - tmp105：温度寄存器 0x00，2 字节，12 位有效位，0.0625°C/LSB，左对齐
 *   - emc1413：温度寄存器 0x00，1 字节，8 位有效位，1°C/LSB，右对齐
 *
 * 通过配置不同的通用化属性（temp_reg_len/resolution/data_bits/data_align），
 * 验证 I2c_TempSensor 驱动不硬编码特定型号，能适配不同温度传感器。
 *
 * 在 QEMU virt 机器中，已挂载 emc1413 温度传感器（地址 0x4C，设为 30°C）。
 */
static int test_i2c_tempsensor_emc141x(TEST_ENTRY *entry)
{
    int ret = 1;
    int bus = 0;                     /* 测试用总线号 */
    float temp = 0;
    allocator_t *allocator = allocator_get_default_instance();
    I2c_TempSensor *sensor = NULL;

    TRY {
        dbg_str(DBG_INFO, "test_i2c_tempsensor_emc141x");

        /* 1. 创建 I2c_TempSensor 对象并初始化（打开总线 + 配置从机地址） */
        sensor = object_new(allocator, "I2c_TempSensor", NULL);
        THROW_IF(sensor == NULL, -1);
        EXEC(sensor->init(sensor, bus, 0x4C));
        dbg_str(DBG_INFO, "tempsensor init success, bus:%d, fd:%d, "
                "slave_addr:0x%x",
                bus, sensor->parent.fd, sensor->slave_addr);

        /* 2. 配置 emc1413 的通用化属性（与 tmp105 默认值不同）：
         *    - 温度寄存器 0x00（EMC141X_TEMP_HIGH0）
         *    - 1 字节（8 位整数温度）
         *    - 分辨率 1°C/LSB
         *    - 8 位有效数据，右对齐（无需移位） */
        sensor->temp_reg = 0x00;
        sensor->temp_reg_len = 1;
        sensor->resolution = 1.0f;
        sensor->data_bits = 8;
        sensor->data_align = I2C_TEMP_ALIGN_RIGHT;
        sensor->big_endian = true;

        /* 3. 读温度 */
        EXEC(sensor->read(sensor, &temp));
        dbg_str(DBG_INFO, "tempsensor read ok, temp:%.2f C", temp);

        /* 4. 验证温度值约为 30°C（emc1413 设为 30°C，8 位整数，1°C/LSB） */
        THROW_IF(temp < 29.0f || temp > 31.0f, -1);
        dbg_str(DBG_INFO, "tempsensor temp verify ok, temp:%.2f C", temp);

        /* 5. 关闭在 deconstruct 中自动调用（无需显式 close） */
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (sensor != NULL) {
            object_destroy(sensor);
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_i2c_tempsensor_emc141x);
