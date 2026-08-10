#ifndef __SPI_H__
#define __SPI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <linux/spi/spidev.h>
#include <libobject/core/Obj.h>

/*
 * Spi 类：通用用户态 SPI 驱动基类。
 *
 * 基于 Linux spidev 子系统（/dev/spidevX.Y）实现用户空间 SPI 访问。
 * 核心设计：
 *   - transfer() 基于 SPI_IOC_MESSAGE(n) 实现全双工传输，支持一次提交多个
 *     spi_ioc_transfer 实现原子组合（如"写命令+读数据"）。
 *   - open 时显式配置 mode/speed/bits_per_word/lsb_first。
 *   - 复用 fd + pthread_mutex，线程安全。
 *   - 支持多设备（open 时指定 bus 和 chip_select）。
 *
 * 继承关系：Obj -> Spi
 */

typedef struct Spi_s Spi;

/* SPI 返回值/错误码 */
typedef enum spi_return_value {
    SPI_ERR_OK            = 0x0,   /* 成功 */
    SPI_ERR_ERROR         = 0x2,   /* 通用错误 */
    SPI_ERR_OPEN          = 0x3,   /* 打开失败 */
    SPI_ERR_TRANSFER      = 0x4,   /* 传输失败 */
    SPI_ERR_TIMEOUT       = 0x5,   /* 超时 */
    SPI_ERR_INVALID_ARG   = 0x6,   /* 非法参数 */
    SPI_ERR_NOT_INIT      = 0x7,   /* 未初始化 */
} spi_return_value_t;

/* SPI 配置 */
typedef struct spi_config {
    uint32_t    max_speed_hz;   /* 最大时钟速率（Hz），默认 500000 */
    uint8_t     mode;           /* SPI 模式 (SPI_MODE_0/1/2/3)，默认 MODE_0 */
    uint8_t     bits_per_word;  /* 字长，默认 8 */
    uint8_t     lsb_first;      /* 0 = MSB first, 1 = LSB first，默认 0 */
} spi_config_t;

struct Spi_s {
    Obj parent;

    int (*construct)(Spi *, char *);
    int (*deconstruct)(Spi *);

    /*virtual methods reimplement*/
    int (*set)(Spi *module, char *attrib, void *value);
    void *(*get)(Spi *, char *attrib);
    char *(*to_json)(Spi *);

    /* SPI device interface */
    /* 打开 /dev/spidevB.C（bus 指定总线号，cs 指定片选），
     * 复用 fd，初始化互斥锁，显式配置 SPI 参数 */
    int (*open)(Spi *spi, int bus, int cs);
    /* 关闭设备 */
    int (*close)(Spi *spi);

    /* 全双工传输（核心）：一次 ioctl 提交 nmsg 个 spi_ioc_transfer，
     * 实现"写命令+读数据"等原子组合。返回 0 成功，负数为错误码 */
    int (*transfer)(Spi *spi, struct spi_ioc_transfer *trs, int nmsg);

    /* 便捷封装 */
    /* 仅写：发送 tx 数据 */
    int (*write)(Spi *spi, const void *tx, size_t len);
    /* 仅读：接收数据到 rx */
    int (*read)(Spi *spi, void *rx, size_t len);
    /* 原子写+读：先发 tx 命令，再读 rx 数据。一次 ioctl 完成，CS 全程保持。
     * 典型场景：SPI Flash 读 ID(0x9F)、读数据(0x03+addr)、读状态(0x05) */
    int (*write_then_read)(Spi *spi, const void *tx, size_t tx_len,
                           void *rx, size_t rx_len);

    /* 配置接口 */
    /* 设置 SPI 参数（mode/speed/bits/lsb），返回 0 成功 */
    int (*configure)(Spi *spi, spi_config_t *config);

    /*attribs*/
    int fd;                 /* /dev/spidevX.Y 文件描述符（复用），-1 表示未打开 */
    int bus;                /* SPI 总线号 */
    int cs;                 /* 片选号 */
    char *dev_path;         /* 设备路径，如 "/dev/spidev0.0" */
    pthread_mutex_t lock;   /* 进程内互斥锁，保护所有传输操作 */
    spi_config_t config;    /* 当前 SPI 配置 */
};

#endif
