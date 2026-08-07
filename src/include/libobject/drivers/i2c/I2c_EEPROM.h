#ifndef __I2C_EEPROM_H__
#define __I2C_EEPROM_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/drivers/i2c/I2c.h>

/*
 * I2c_EEPROM 类：I2C EEPROM 设备驱动。
 *
 * 继承 I2c 基类，封装 EEPROM 的从机地址和容量。
 * 读写接口复用 I2c 的 read/write 原子事务。
 *
 * 典型芯片：at24c02（256 字节，8 位地址）、at24c32（4KB，16 位地址）。
 * 寄存器地址宽度通过 I2c 的 set_width 配置（8 或 16 位）。
 *
 * 职责：只负责底层读写（复用 I2c 的 read/write），
 * 存储规划（哪个地址存什么）由上层应用决定。
 *
 * 继承关系：Obj -> I2c -> I2c_EEPROM
 */

typedef struct I2c_EEPROM_s I2c_EEPROM;

struct I2c_EEPROM_s {
    I2c parent;

    int (*construct)(I2c_EEPROM *, char *);
    int (*deconstruct)(I2c_EEPROM *);

    /*virtual methods reimplement*/
    int (*set)(I2c_EEPROM *module, char *attrib, void *value);
    void *(*get)(I2c_EEPROM *, char *attrib);
    char *(*to_json)(I2c_EEPROM *);

    /* EEPROM 初始化接口：打开总线 + 配置从机地址 + 配置容量 */
    int (*init)(I2c_EEPROM *eeprom, int bus_number, int slave_addr, uint32_t size);

    /* EEPROM 读写接口（复用 I2c 的 read/write 原子事务） */
    /* 读：addr 为 EEPROM 存储地址，buf 为读缓冲区，len 为读取字节数 */
    int (*read)(I2c_EEPROM *eeprom, uint16_t addr, void *buf, size_t len);
    /* 写：addr 为 EEPROM 存储地址，buf 为要写入的数据，len 为写入字节数 */
    int (*write)(I2c_EEPROM *eeprom, uint16_t addr, const void *buf, size_t len);

    /* 配置接口 */
    /* 设置寄存器地址宽度（继承 I2c，EEPROM 需配置 8/16 位） */
    int (*set_width)(I2c_EEPROM *eeprom, int width);

    /*attribs*/
    int slave_addr;   /* 从机地址 */
    uint32_t size;    /* EEPROM 容量（字节） */
};

#endif
