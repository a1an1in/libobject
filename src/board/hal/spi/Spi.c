/**
 * @file Spi.c
 * @Synopsis  通用用户态 SPI 驱动基类。
 * 基于 Linux spidev 子系统（/dev/spidevX.Y）实现用户空间 SPI 访问。
 * 核心设计：
 *   - transfer() 基于 SPI_IOC_MESSAGE(n) 实现全双工传输。
 *   - open 时显式配置 mode/speed/bits_per_word/lsb_first。
 *   - 复用 fd + pthread_mutex，线程安全。
 * @author alan lin
 * @version
 * @date 2026-08-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <libobject/board/hal/spi/Spi.h>
#include <libobject/core/utils/dbg/debug.h>

#define SPI_DEV_PATH_MAX_LEN  64

/* 默认 SPI 配置 */
static const spi_config_t default_config = {
    .max_speed_hz  = 500000,
    .mode          = SPI_MODE_0,
    .bits_per_word = 8,
    .lsb_first     = 0,
};

/*
 * 打开 /dev/spidevB.C，显式配置 SPI 参数，复用 fd。
 */
static int __open(Spi *spi, int bus, int cs)
{
    char path[SPI_DEV_PATH_MAX_LEN];
    int ret = -1;

    TRY {
        THROW_IF(spi == NULL || bus < 0 || cs < 0, -SPI_ERR_INVALID_ARG);

        /* 1. 若已打开，先关闭 */
        if (spi->fd >= 0) {
            close(spi->fd);
            spi->fd = -1;
        }

        /* 2. 记录参数 */
        spi->bus = bus;
        spi->cs  = cs;

        /* 3. 构造设备路径 /dev/spidevB.C */
        snprintf(path, sizeof(path), "/dev/spidev%d.%d", bus, cs);
        if (spi->dev_path == NULL) {
            spi->dev_path = strdup(path);
        } else {
            free(spi->dev_path);
            spi->dev_path = strdup(path);
        }
        THROW_IF(spi->dev_path == NULL, -SPI_ERR_ERROR);

        /* 4. 打开设备 */
        spi->fd = open(path, O_RDWR);
        THROW_IF(spi->fd < 0, -SPI_ERR_OPEN);

        /* 5. 应用默认配置 */
        spi->config = default_config;
        EXEC(spi->configure(spi, &spi->config));

        dbg_str(DBG_INFO, "spi open success, dev:%s, fd:%d, "
                "mode:%d, speed:%u, bits:%d",
                path, spi->fd, spi->config.mode,
                spi->config.max_speed_hz, spi->config.bits_per_word);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "spi open failed, dev:%s, errno:%d(%s)",
                path, errno, strerror(errno));
        if (spi->fd >= 0) {
            close(spi->fd);
            spi->fd = -1;
        }
    }

    return ret;
}

/*
 * 关闭设备。
 */
static int __close(Spi *spi)
{
    if (spi == NULL) return -1;

    if (spi->fd >= 0) {
        close(spi->fd);
        spi->fd = -1;
    }
    if (spi->dev_path != NULL) {
        free(spi->dev_path);
        spi->dev_path = NULL;
    }

    dbg_str(DBG_INFO, "spi close ok");
    return 0;
}

/*
 * 全双工传输（核心）：一次 ioctl 提交 nmsg 个 spi_ioc_transfer。
 */
static int __transfer(Spi *spi, struct spi_ioc_transfer *trs, int nmsg)
{
    int ret = -1;

    TRY {
        THROW_IF(spi == NULL || trs == NULL || nmsg <= 0,
                 -SPI_ERR_INVALID_ARG);
        THROW_IF(spi->fd < 0, -SPI_ERR_NOT_INIT);

        pthread_mutex_lock(&spi->lock);
        ret = ioctl(spi->fd, SPI_IOC_MESSAGE(nmsg), trs);
        pthread_mutex_unlock(&spi->lock);

        THROW_IF(ret < 0, -SPI_ERR_TRANSFER);

        dbg_str(DBG_DETAIL, "spi transfer ok, nmsg:%d, bytes:%d",
                nmsg, ret);
        ret = 0;
    } CATCH (ret) {
        pthread_mutex_unlock(&spi->lock);
        dbg_str(DBG_ERROR, "spi transfer failed, nmsg:%d, "
                "errno:%d(%s)", nmsg, errno, strerror(errno));
    }

    return ret;
}

/*
 * 仅写。
 */
static int __write(Spi *spi, const void *tx, size_t len)
{
    struct spi_ioc_transfer tr = {0};

    tr.tx_buf = (uintptr_t)tx;
    tr.rx_buf = 0;
    tr.len    = len;

    return spi->transfer(spi, &tr, 1);
}

/*
 * 仅读（发送 dummy 0x00）。
 */
static int __read(Spi *spi, void *rx, size_t len)
{
    struct spi_ioc_transfer tr = {0};

    tr.tx_buf = 0;
    tr.rx_buf = (uintptr_t)rx;
    tr.len    = len;

    return spi->transfer(spi, &tr, 1);
}

/*
 * 原子写+读：先发 tx 命令，再读 rx 数据。CS 全程保持。
 */
static int __write_then_read(Spi *spi, const void *tx, size_t tx_len,
                              void *rx, size_t rx_len)
{
    struct spi_ioc_transfer tr[2];

    memset(tr, 0, sizeof(tr));
    tr[0].tx_buf = (uintptr_t)tx;
    tr[0].len    = tx_len;

    tr[1].rx_buf = (uintptr_t)rx;
    tr[1].len    = rx_len;

    return spi->transfer(spi, tr, 2);
}

/*
 * 配置 SPI 参数。
 */
static int __configure(Spi *spi, spi_config_t *config)
{
    int ret = -1;

    TRY {
        THROW_IF(spi == NULL || config == NULL, -SPI_ERR_INVALID_ARG);
        THROW_IF(spi->fd < 0, -SPI_ERR_NOT_INIT);

        ret = ioctl(spi->fd, SPI_IOC_WR_MODE, &config->mode);
        THROW_IF(ret < 0, -SPI_ERR_ERROR);

        ret = ioctl(spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &config->max_speed_hz);
        THROW_IF(ret < 0, -SPI_ERR_ERROR);

        ret = ioctl(spi->fd, SPI_IOC_WR_BITS_PER_WORD, &config->bits_per_word);
        THROW_IF(ret < 0, -SPI_ERR_ERROR);

        ret = ioctl(spi->fd, SPI_IOC_WR_LSB_FIRST, &config->lsb_first);
        THROW_IF(ret < 0, -SPI_ERR_ERROR);

        spi->config = *config;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "spi configure failed, errno:%d(%s)",
                errno, strerror(errno));
    }

    return ret;
}

static int __construct(Spi *module, char *init_str)
{
    module->fd       = -1;
    module->bus      = -1;
    module->cs       = -1;
    module->dev_path = NULL;
    module->config   = default_config;
    pthread_mutex_init(&module->lock, NULL);
    return 0;
}

static int __deconstruct(Spi *module)
{
    if (module->fd >= 0) {
        module->close(module);
    }
    pthread_mutex_destroy(&module->lock);
    return 0;
}

DEFINE_CLASS(
    EXTENDS(Spi, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(transfer, __transfer),
    Class_VFunc_Entry(write, __write),
    Class_VFunc_Entry(read, __read),
    Class_VFunc_Entry(write_then_read, __write_then_read),
    Class_VFunc_Entry(configure, __configure)
);
