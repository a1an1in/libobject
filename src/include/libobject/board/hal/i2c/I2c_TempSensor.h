#ifndef __I2C_TEMPSENSOR_H__
#define __I2C_TEMPSENSOR_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/board/hal/i2c/I2c.h>

/*
 * I2c_TempSensor 类：通用 I2C 温度传感器驱动。
 *
 * 继承 I2c 基类，封装温度传感器的读取协议。
 * 复用 I2c 的 read/write 原子事务。
 *
 * 通用性：温度寄存器地址、分辨率、有效数据位数、字节序等
 * 型号相关参数作为可配置属性，通过 init 或 set 配置，不硬编码特定型号。
 *
 * 典型芯片（通过配置属性适配）：
 *   - tmp105/lm75：温度寄存器 0x00，12 位，0.0625°C/LSB，高字节在前
 *   - tmp100：温度寄存器 0x00，12 位，0.0625°C/LSB，高字节在前
 *
 * 继承关系：Obj -> I2c -> I2c_TempSensor
 */

typedef struct I2c_TempSensor_s I2c_TempSensor;

/* 有效数据对齐方式 */
typedef enum i2c_temp_align {
    I2C_TEMP_ALIGN_LEFT = 0,   /* 有效位在左（高位），需右移无效位（tmp105 等） */
    I2C_TEMP_ALIGN_RIGHT = 1,  /* 有效位在右（低位），无需移位 */
} i2c_temp_align_t;

/* 温度单位 */
typedef enum i2c_temp_unit {
    I2C_TEMP_UNIT_CELSIUS = 0,     /* 摄氏度 */
    I2C_TEMP_UNIT_FAHRENHEIT = 1,  /* 华氏度 */
} i2c_temp_unit_t;

struct I2c_TempSensor_s {
    I2c parent;

    int (*construct)(I2c_TempSensor *, char *);
    int (*deconstruct)(I2c_TempSensor *);

    /*virtual methods reimplement*/
    int (*set)(I2c_TempSensor *module, char *attrib, void *value);
    void *(*get)(I2c_TempSensor *, char *attrib);
    char *(*to_json)(I2c_TempSensor *);

    /* 温度传感器初始化接口：打开总线 + 配置从机地址 */
    int (*init)(I2c_TempSensor *sensor, int bus_number, int slave_addr);

    /* 温度传感器接口 */
    /* 读温度（摄氏度） */
    int (*read)(I2c_TempSensor *sensor, float *temp);
    /* 设置温度上下限（摄氏度） */
    int (*set_limit)(I2c_TempSensor *sensor, float high, float low);

    /* 配置接口 */
    /* 设置寄存器地址宽度（继承 I2c） */
    int (*set_width)(I2c_TempSensor *sensor, int width);

    /*attribs*/
    int slave_addr;        /* 从机地址 */
    uint8_t temp_reg;      /* 温度寄存器地址（默认 0x00） */
    uint8_t temp_high_reg; /* 上限温度寄存器地址（默认 temp_reg+2） */
    uint8_t temp_low_reg;  /* 下限温度寄存器地址（默认 temp_reg+3） */
    uint8_t temp_reg_len;  /* 温度寄存器字节数（默认 2） */
    float resolution;      /* 分辨率 °C/LSB（默认 0.0625） */
    int data_bits;         /* 有效数据位数（默认 12） */
    i2c_temp_align_t data_align;  /* 有效数据对齐方式（默认左对齐） */
    i2c_temp_unit_t temp_unit;    /* 温度单位（默认摄氏度） */
    bool big_endian;       /* 是否高字节在前（默认 true） */
};

#endif
