/**
 * @file I2c_EEPROM.c
 * @Synopsis  I2C EEPROM 设备驱动。
 * 继承 I2c 基类，封装 EEPROM 的从机地址和容量。
 * 读写接口复用 I2c 的 read/write 原子事务。
 * @author alan lin
 * @version
 * @date 2026-08-07
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/board/hal/i2c/I2c_EEPROM.h>
#include <libobject/core/utils/dbg/debug.h>

/*
 * 读：addr 为 EEPROM 存储地址，buf 为读缓冲区，len 为读取字节数。
 * 复用 I2c 的 read 原子事务，并做地址越界检查。
 */
static int __read(I2c_EEPROM *eeprom, uint16_t addr, void *buf, size_t len)
{
    I2c *i2c = (I2c *)eeprom;
    int ret = -1;

    TRY {
        THROW_IF(eeprom == NULL || buf == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(len == 0, -I2C_ERR_INVALID_ARG);
        /* 地址越界检查 */
        THROW_IF(addr + len > eeprom->size, -I2C_ERR_INVALID_ARG);

        EXEC(i2c->read(i2c, eeprom->slave_addr, addr, NULL, 0, buf, len));
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom read failed, addr:0x%x, len:%d",
                addr, (int)len);
    }

    return ret;
}

/*
 * 写：addr 为 EEPROM 存储地址，buf 为要写入的数据，len 为写入字节数。
 * 复用 I2c 的 write 原子事务，并做地址越界检查。
 */
static int __write(I2c_EEPROM *eeprom, uint16_t addr, const void *buf, size_t len)
{
    I2c *i2c = (I2c *)eeprom;
    int ret = -1;

    TRY {
        THROW_IF(eeprom == NULL || buf == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(len == 0, -I2C_ERR_INVALID_ARG);
        /* 地址越界检查 */
        THROW_IF(addr + len > eeprom->size, -I2C_ERR_INVALID_ARG);

        EXEC(i2c->write(i2c, eeprom->slave_addr, addr, buf, len));
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom write failed, addr:0x%x, len:%d",
                addr, (int)len);
    }

    return ret;
}

/*
 * 初始化：打开总线 + 配置从机地址 + 配置容量。
 */
static int __init(I2c_EEPROM *eeprom, int bus_number, int slave_addr, uint32_t size)
{
    I2c *i2c = (I2c *)eeprom;
    int ret = -1;

    TRY {
        THROW_IF(eeprom == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(size == 0, -I2C_ERR_INVALID_ARG);

        /* 1. 打开 /dev/i2c-N（复用 I2c 的 open） */
        EXEC(i2c->open(i2c, bus_number));

        /* 2. 配置从机地址和容量 */
        eeprom->slave_addr = slave_addr;
        eeprom->size = size;

        dbg_str(DBG_INFO, "eeprom init ok, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom init failed, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
    }

    return ret;
}

static int __construct(I2c_EEPROM *module, char *init_str)
{
    module->slave_addr = 0x50;   /* 默认从机地址 0x50 */
    module->size = 256;          /* 默认容量 256 字节（at24c02） */
    return 0;
}

static int __deconstruct(I2c_EEPROM *module)
{
    I2c *i2c = (I2c *)module;

    /* 关闭 I2C 设备（close 在 deconstruct 中自动调用） */
    if (i2c->fd >= 0) {
        i2c->close(i2c);
    }
    return 0;
}

/*
 * I2c_EEPROM 注册具体类。
 * open/close/transfer/set_retry 等接口继承 I2c 的实现（不在此注册）。
 * read/write 覆盖为 EEPROM 封装（带从机地址和越界检查）。
 * set_width 继承 I2c 的实现（EEPROM 需配置 8/16 位地址宽度）。
 */
DEFINE_CLASS(
    EXTENDS(I2c_EEPROM, I2c),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_NFunc_Entry(init, __init),
    Class_NFunc_Entry(read, __read),
    Class_NFunc_Entry(write, __write),
    Class_VFunc_Entry(set_width, NULL)
);
