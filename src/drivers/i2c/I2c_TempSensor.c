/**
 * @file I2c_TempSensor.c
 * @Synopsis  通用 I2C 温度传感器驱动。
 * 继承 I2c 基类，封装温度传感器的读取协议。
 * 复用 I2c 的 read/write 原子事务。
 * 型号相关参数（温度寄存器地址、分辨率、数据位数、字节序）作为可配置属性。
 * @author alan lin
 * @version
 * @date 2026-08-07
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/drivers/i2c/I2c_TempSensor.h>
#include <libobject/core/utils/dbg/debug.h>

/* 默认参数（tmp105/lm75 兼容） */
#define TEMPSENSOR_DEFAULT_TEMP_REG  0x00
#define TEMPSENSOR_DEFAULT_RESOLUTION 0.0625f
#define TEMPSENSOR_DEFAULT_DATA_BITS  12
#define TEMPSENSOR_DEFAULT_BIG_ENDIAN true
#define TEMPSENSOR_DEFAULT_REG_LEN   2
#define TEMPSENSOR_DEFAULT_ALIGN     I2C_TEMP_ALIGN_LEFT
#define TEMPSENSOR_DEFAULT_UNIT      I2C_TEMP_UNIT_CELSIUS

/*
 * 初始化：打开总线 + 配置从机地址。
 * 型号相关参数（temp_reg/resolution/data_bits/big_endian）使用默认值，
 * 可通过 set 属性或直接修改属性配置。
 */
static int __init(I2c_TempSensor *sensor, int bus_number, int slave_addr)
{
    I2c *i2c = (I2c *)sensor;
    int ret = -1;

    TRY {
        THROW_IF(sensor == NULL, -I2C_ERR_INVALID_ARG);

        /* 1. 打开 /dev/i2c-N（复用 I2c 的 open） */
        ret = i2c->open(i2c, bus_number);
        THROW_IF(ret < 0, ret);

        /* 2. 配置从机地址 */
        sensor->slave_addr = slave_addr;

        dbg_str(DBG_INFO, "tempsensor init ok, bus:%d, slave_addr:0x%x",
                bus_number, slave_addr);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "tempsensor init failed, bus:%d, slave_addr:0x%x",
                bus_number, slave_addr);
    }

    return ret;
}

/*
 * 读温度。
 * 读温度寄存器（temp_reg，temp_reg_len 字节），按 data_bits 有效位数、
 * data_align 对齐方式和 resolution 分辨率转换温度值。
 */
static int __read(I2c_TempSensor *sensor, float *temp)
{
    I2c *i2c = (I2c *)sensor;
    uint8_t buf[4] = {0};
    int32_t raw = 0;
    int shift;
    int i;
    int ret = -1;

    TRY {
        THROW_IF(sensor == NULL || temp == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(sensor->temp_reg_len > 4, -I2C_ERR_INVALID_ARG);

        /* 读温度寄存器（temp_reg_len 字节） */
        ret = i2c->read(i2c, sensor->slave_addr, sensor->temp_reg,
                        NULL, 0, buf, sensor->temp_reg_len);
        THROW_IF(ret < 0, ret);

        /* 组合原始值（按字节序） */
        if (sensor->big_endian) {
            for (i = 0; i < sensor->temp_reg_len; i++) {
                raw = (raw << 8) | buf[i];
            }
        } else {
            for (i = sensor->temp_reg_len - 1; i >= 0; i--) {
                raw = (raw << 8) | buf[i];
            }
        }

        /* 按对齐方式处理有效位 */
        if (sensor->data_align == I2C_TEMP_ALIGN_LEFT) {
            /* 有效位在左（高位），右移无效位 */
            shift = sensor->temp_reg_len * 8 - sensor->data_bits;
            raw >>= shift;
        }
        /* 右对齐：有效位已在低位，无需移位 */

        *temp = (float)raw * sensor->resolution;

        /* 单位转换（若为华氏度） */
        if (sensor->temp_unit == I2C_TEMP_UNIT_FAHRENHEIT) {
            *temp = *temp * 9.0f / 5.0f + 32.0f;
        }

        dbg_str(DBG_INFO, "tempsensor read_temp ok, raw:0x%x, temp:%.2f",
                (unsigned int)raw, *temp);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "tempsensor read_temp failed, slave_addr:0x%x",
                sensor->slave_addr);
    }

    return ret;
}

/*
 * 设置温度报警阈值（上限/下限）。
 *
 * 功能：将 high（上限温度）和 low（下限温度）写入温度传感器的
 * 上限/下限寄存器（temp_high_reg / temp_low_reg，可配置），
 * 用于设置硬件报警阈值。
 *
 * 中断/报警机制：
 *   1. 设置阈值后，温度传感器持续监测当前温度。
 *   2. 当温度超过上限（high）或低于下限（low）时，
 *      芯片的 Alert 引脚（中断输出）被拉低/拉高。
 *   3. Alert 引脚通常连接到 MCU 的中断引脚（GPIO EXTI），
 *      触发中断，MCU 在中断处理中读取温度并处理报警。
 *   4. 报警模式由配置寄存器决定：
 *      - 比较模式（Comparator）：温度越界持续报警，回落到阈值内停止。
 *      - 中断模式（Interrupt）：温度越界触发一次报警，需读寄存器清除。
 *
 * 注意：本函数只设置报警阈值（写入寄存器），不直接处理中断。
 * 中断的使能、清除、处理由上层（MCU 中断服务程序）负责。
 */
static int __set_limit(I2c_TempSensor *sensor, float high, float low)
{
    I2c *i2c = (I2c *)sensor;
    uint8_t buf[4] = {0};
    int32_t raw;
    int shift;
    int i;
    int ret = -1;

    TRY {
        THROW_IF(sensor == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(sensor->temp_reg_len > 4, -I2C_ERR_INVALID_ARG);

        /* 单位转换（若为华氏度，转回摄氏度） */
        if (sensor->temp_unit == I2C_TEMP_UNIT_FAHRENHEIT) {
            high = (high - 32.0f) * 5.0f / 9.0f;
            low = (low - 32.0f) * 5.0f / 9.0f;
        }

        /* 温度转 data_bits 位格式 */
        shift = sensor->temp_reg_len * 8 - sensor->data_bits;

        /* 上限 */
        raw = (int32_t)(high / sensor->resolution);
        if (sensor->data_align == I2C_TEMP_ALIGN_LEFT) {
            raw <<= shift;
        }
        if (sensor->big_endian) {
            for (i = sensor->temp_reg_len - 1; i >= 0; i--) {
                buf[i] = (uint8_t)(raw & 0xFF);
                raw >>= 8;
            }
        } else {
            for (i = 0; i < sensor->temp_reg_len; i++) {
                buf[i] = (uint8_t)(raw & 0xFF);
                raw >>= 8;
            }
        }
        ret = i2c->write(i2c, sensor->slave_addr,
                         sensor->temp_high_reg, buf, sensor->temp_reg_len);
        THROW_IF(ret < 0, ret);

        /* 下限 */
        raw = (int32_t)(low / sensor->resolution);
        if (sensor->data_align == I2C_TEMP_ALIGN_LEFT) {
            raw <<= shift;
        }
        if (sensor->big_endian) {
            for (i = sensor->temp_reg_len - 1; i >= 0; i--) {
                buf[i] = (uint8_t)(raw & 0xFF);
                raw >>= 8;
            }
        } else {
            for (i = 0; i < sensor->temp_reg_len; i++) {
                buf[i] = (uint8_t)(raw & 0xFF);
                raw >>= 8;
            }
        }
        ret = i2c->write(i2c, sensor->slave_addr,
                         sensor->temp_low_reg, buf, sensor->temp_reg_len);
        THROW_IF(ret < 0, ret);

        dbg_str(DBG_INFO, "tempsensor set_limit ok, high:%.2f, low:%.2f",
                high, low);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "tempsensor set_limit failed, high:%.2f, low:%.2f",
                high, low);
    }

    return ret;
}

static int __construct(I2c_TempSensor *module, char *init_str)
{
    module->slave_addr = 0x48;   /* 默认从机地址 0x48（tmp105 默认） */
    module->temp_reg = TEMPSENSOR_DEFAULT_TEMP_REG;
    module->temp_high_reg = TEMPSENSOR_DEFAULT_TEMP_REG + 2;  /* 上限寄存器 */
    module->temp_low_reg = TEMPSENSOR_DEFAULT_TEMP_REG + 3;   /* 下限寄存器 */
    module->temp_reg_len = TEMPSENSOR_DEFAULT_REG_LEN;
    module->resolution = TEMPSENSOR_DEFAULT_RESOLUTION;
    module->data_bits = TEMPSENSOR_DEFAULT_DATA_BITS;
    module->data_align = TEMPSENSOR_DEFAULT_ALIGN;
    module->temp_unit = TEMPSENSOR_DEFAULT_UNIT;
    module->big_endian = TEMPSENSOR_DEFAULT_BIG_ENDIAN;
    return 0;
}

static int __deconstruct(I2c_TempSensor *module)
{
    I2c *i2c = (I2c *)module;

    /* 关闭 I2C 设备（close 在 deconstruct 中自动调用） */
    if (i2c->fd >= 0) {
        i2c->close(i2c);
    }
    return 0;
}

/*
 * I2c_TempSensor 注册具体类。
 * open/close/transfer/set_retry 等接口继承 I2c 的实现（不在此注册）。
 * init/read_temp/set_limit 为温度传感器特有接口。
 * set_width 继承 I2c 的实现。
 */
DEFINE_CLASS(
    EXTENDS(I2c_TempSensor, I2c),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_NFunc_Entry(init, __init),
    Class_NFunc_Entry(read, __read),
    Class_NFunc_Entry(set_limit, __set_limit),
    Class_VFunc_Entry(set_width, NULL)
);
