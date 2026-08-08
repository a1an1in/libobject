#ifndef __I2C_H__
#define __I2C_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <linux/i2c.h>
#include <libobject/core/Obj.h>

/*
 * I2c 类：通用用户态 I2C 驱动基类。
 *
 * 基于 Linux i2c-dev 驱动（/dev/i2c-N）实现用户空间 I2C 访问。
 * 核心设计：
 *   - 以 I2C_RDWR ioctl 实现真正的原子事务（单次调用提交多个 i2c_msg，
 *     保证"写寄存器地址 + 读数据"等操作不被其他线程/驱动打断）。
 *   - 复用 fd + 进程内 pthread_mutex，实现单进程多线程场景下的线程安全。
 *   - 支持多总线（open 时指定 bus_number，拼接 /dev/i2c-N）。
 *   - 区分 NACK、总线错误、超时、非法参数等错误码。
 *
 * 继承关系：Obj -> I2c
 */

typedef struct I2c_s I2c;

/* I2C 返回值/错误码 */
typedef enum i2c_return_value {
    I2C_ERR_OK          = 0x0,   /* 成功 */
    I2C_ERR_ERROR       = 0x2,   /* 通用错误 */
    I2C_ERR_WRITE       = 0x3,   /* 写错误 */
    I2C_ERR_READ        = 0x4,   /* 读错误 */
    I2C_ERR_NACK        = 0x5,   /* 无应答 */
    I2C_ERR_TIMEOUT     = 0x6,   /* 超时 */
    I2C_ERR_BUS         = 0x7,   /* 总线错误 */
    I2C_ERR_INVALID_ARG = 0x8,   /* 非法参数 */
} i2c_return_value_t;

struct I2c_s {
    Obj parent;

    int (*construct)(I2c *, char *);
    int (*deconstruct)(I2c *);

    /*virtual methods reimplement*/
    int (*set)(I2c *module, char *attrib, void *value);
    void *(*get)(I2c *, char *attrib);
    char *(*to_json)(I2c *);

    /* I2C device interface */
    /* 打开 /dev/i2c-N（bus_number 指定总线号），复用 fd，初始化互斥锁 */
    int (*open)(I2c *i2c, int bus_number);
    /* 关闭设备，销毁互斥锁 */
    int (*close)(I2c *i2c);

    /* 原子事务（核心）：基于 I2C_RDWR ioctl，一次提交多个 i2c_msg。
     * slave_addr 为 7 位从机地址（0x08-0x77）。
     * msgs 中的 addr 字段会被统一替换为 slave_addr（也可各自指定）。
     * 返回 0 成功，负数为错误码（i2c_return_value_t 取负）。
     */
    int (*transfer)(I2c *i2c, int slave_addr, struct i2c_msg *msgs, int nmsgs);

    /* 便捷封装（基于 transfer 实现，均为原子事务） */
    /* 读：reg 为寄存器地址，write_buf 为写数据（不含寄存器地址，可为空）。
     * 若 reg 为合法寄存器地址（非 0xFFFF），先写 [reg][write_buf...] 再读
     * （读指定寄存器）；若 reg == 0xFFFF，则直接读（不写寄存器地址）。
     * reg 按 reg_width 属性（8 或 16 位）编码发送。 */
    int (*read)(I2c *i2c, int slave_addr, uint16_t reg,
                const void *write_buf, size_t write_size,
                void *read_buf, size_t read_size);
    /* 仅写：reg 为起始寄存器地址，buf 为要写入的数据（不含寄存器地址）。
     * 内部构造 [reg][buf...] 发送，支持连续写多个寄存器（指针自动递增）。
     * reg 按 reg_width 属性（8 或 16 位）编码发送。 */
    int (*write)(I2c *i2c, int slave_addr, uint16_t reg,
                 const void *buf, size_t size);

    /* 配置接口 */
    /* 设置瞬时错误（如总线忙）的重试次数，默认 0 */
    int (*set_retry)(I2c *i2c, int retry);
    /* 设置寄存器地址宽度：8 或 16 位，默认 8 位 */
    int (*set_width)(I2c *i2c, int width);

    /*attribs*/
    int fd;                 /* /dev/i2c-N 文件描述符（复用），-1 表示未打开 */
    int bus_number;         /* I2C 总线号 */
    pthread_mutex_t lock;   /* 进程内互斥锁，保护所有事务操作 */
    int retry;              /* 瞬时错误重试次数 */
    int reg_width;          /* 寄存器地址宽度（位）：8 或 16，默认 8 */
};

#endif
