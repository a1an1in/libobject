/**
 * @file Mtd.c
 * @Synopsis  通用用户态 MTD（Memory Technology Device）驱动基类。
 * 基于 Linux MTD 子系统（/dev/mtd*）实现用户空间 Flash 访问。
 * 核心设计：
 *   - open 时通过 ioctl(MEMGETINFO) 获取设备信息（size/erasesize/writesize/oobsize）。
 *   - erase 通过 ioctl(MEMERASE) 按擦除块擦除（块对齐）。
 *   - read/write 通过 lseek + read/write 访问（写入前需先擦除）。
 *   - 复用 fd + 进程内 pthread_mutex，实现单进程多线程场景下的线程安全。
 *   - 支持多分区（open 时指定设备路径，如 /dev/mtd0 或 /dev/flash/spider_meta）。
 * @author alan lin
 * @version
 * @date 2026-08-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <libobject/board/hal/mtd/Mtd.h>
#include <libobject/core/utils/dbg/debug.h>

#define MTD_DEV_PATH_MAX_LEN  256

/*
 * 打开 /dev/mtdN（device 指定设备路径，如 "/dev/mtd0"）。
 * 复用 fd：初始化时打开一次，后续操作复用，避免每次 open/close 开销。
 * 同时初始化进程内互斥锁，并通过 MEMGETINFO 获取设备信息。
 */
static int __open(Mtd *mtd, char *device)
{
    mtd_info_t kinfo;   /* 内核 struct mtd_info_user */
    int ret = -1;

    TRY {
        THROW_IF(mtd == NULL || device == NULL, -1);

        /* 1. 若已打开，先关闭（支持重新指定设备） */
        if (mtd->fd >= 0) {
            close(mtd->fd);
            mtd->fd = -1;
        }

        /* 2. 记录设备路径 */
        if (mtd->dev_path == NULL) {
            mtd->dev_path = strdup(device);
        } else {
            free(mtd->dev_path);
            mtd->dev_path = strdup(device);
        }
        THROW_IF(mtd->dev_path == NULL, -1);

        /* 3. 打开设备文件（复用 fd） */
        mtd->fd = open(device, O_RDWR | O_SYNC);
        THROW_IF(mtd->fd < 0, -1);

        /* 4. 获取设备信息（内核 struct mtd_info_user） */
        ret = ioctl(mtd->fd, MEMGETINFO, &kinfo);
        THROW_IF(ret < 0, -1);

        /* 5. 复制到自定义 mtd_dev_info_t */
        mtd->info.size       = kinfo.size;
        mtd->info.erasesize  = kinfo.erasesize;
        mtd->info.writesize  = kinfo.writesize;
        mtd->info.oobsize    = kinfo.oobsize;
        mtd->info.type       = kinfo.type;

        dbg_str(DBG_INFO, "mtd open success, dev:%s, fd:%d, size:0x%x, "
                "erasesize:0x%x, writesize:0x%x, oobsize:0x%x, type:0x%x",
                device, mtd->fd, mtd->info.size, mtd->info.erasesize,
                mtd->info.writesize, mtd->info.oobsize, mtd->info.type);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "mtd open failed, dev:%s, errno:%d(%s)",
                device, errno, strerror(errno));
        if (mtd->fd >= 0) {
            close(mtd->fd);
            mtd->fd = -1;
        }
        if (mtd->dev_path != NULL) {
            free(mtd->dev_path);
            mtd->dev_path = NULL;
        }
    }

    return ret;
}

/*
 * 关闭设备，销毁互斥锁。
 */
static int __close(Mtd *mtd)
{
    if (mtd == NULL) {
        return -1;
    }

    if (mtd->fd >= 0) {
        close(mtd->fd);
        mtd->fd = -1;
    }
    if (mtd->dev_path != NULL) {
        free(mtd->dev_path);
        mtd->dev_path = NULL;
    }

    dbg_str(DBG_INFO, "mtd close ok");

    return 0;
}

/*
 * 获取设备信息（size/erasesize/writesize/oobsize/type）。
 */
static int __get_info(Mtd *mtd, mtd_dev_info_t *info)
{
    if (mtd == NULL || info == NULL) {
        return -1;
    }
    if (mtd->fd < 0) {
        dbg_str(DBG_ERROR, "mtd not initialized");
        return -MTD_ERR_NOT_INIT;
    }

    /* 将内核 mtd_info_t（struct mtd_info_user）字段复制到自定义 mtd_dev_info_t */
    info->size       = mtd->info.size;
    info->erasesize  = mtd->info.erasesize;
    info->writesize  = mtd->info.writesize;
    info->oobsize    = mtd->info.oobsize;
    info->type       = mtd->info.type;
    return 0;
}

/*
 * 擦除：offset 和 size 必须按 erasesize 对齐，按块擦除。
 * 通过 ioctl(MEMERASE) 逐块擦除。
 */
static int __erase(Mtd *mtd, uint32_t offset, uint32_t size)
{
    struct erase_info_user erase;
    uint32_t block, end;
    int ret = -1;

    TRY {
        THROW_IF(mtd == NULL, -1);
        THROW_IF(mtd->fd < 0, -MTD_ERR_NOT_INIT);
        THROW_IF(mtd->info.erasesize == 0, -MTD_ERR_ERROR);

        /* 1. 校验 offset/size 块对齐 */
        THROW_IF(offset % mtd->info.erasesize != 0, -MTD_ERR_INVALID_ARG);
        THROW_IF(size % mtd->info.erasesize != 0, -MTD_ERR_INVALID_ARG);
        THROW_IF(offset + size > mtd->info.size, -MTD_ERR_INVALID_ARG);

        /* 2. 加锁，逐块擦除 */
        pthread_mutex_lock(&mtd->lock);
        end = offset + size;
        for (block = offset; block < end; block += mtd->info.erasesize) {
            erase.start = block;
            erase.length = mtd->info.erasesize;
            ret = ioctl(mtd->fd, MEMERASE, &erase);
            THROW_IF(ret < 0, -MTD_ERR_ERASE);
        }
        pthread_mutex_unlock(&mtd->lock);

        dbg_str(DBG_INFO, "mtd erase ok, offset:0x%x, size:0x%x",
                offset, size);
        ret = 0;
    } CATCH (ret) {
        pthread_mutex_unlock(&mtd->lock);
        dbg_str(DBG_ERROR, "mtd erase failed, offset:0x%x, size:0x%x, "
                "errno:%d(%s)", offset, size, errno, strerror(errno));
    }

    return ret;
}

/*
 * 读：从 offset 读取 size 字节到 buf，返回实际读取字节数（失败返回负数）。
 * 通过 lseek + read 访问。
 */
static int __read(Mtd *mtd, uint32_t offset, void *buf, uint32_t size)
{
    ssize_t nread;
    int ret = -1;

    TRY {
        THROW_IF(mtd == NULL || buf == NULL, -1);
        THROW_IF(mtd->fd < 0, -MTD_ERR_NOT_INIT);
        THROW_IF(offset + size > mtd->info.size, -MTD_ERR_INVALID_ARG);

        /* 1. 加锁，定位到 offset */
        pthread_mutex_lock(&mtd->lock);
        ret = lseek(mtd->fd, offset, SEEK_SET);
        THROW_IF(ret < 0, -MTD_ERR_READ);

        /* 2. 读取数据 */
        nread = read(mtd->fd, buf, size);
        THROW_IF(nread < 0, -MTD_ERR_READ);
        pthread_mutex_unlock(&mtd->lock);

        dbg_str(DBG_DETAIL, "mtd read ok, offset:0x%x, size:0x%x, nread:%zd",
                offset, size, nread);
        ret = (int)nread;
    } CATCH (ret) {
        pthread_mutex_unlock(&mtd->lock);
        dbg_str(DBG_ERROR, "mtd read failed, offset:0x%x, size:0x%x, "
                "errno:%d(%s)", offset, size, errno, strerror(errno));
    }

    return ret;
}

/*
 * 写：从 offset 写入 size 字节（写入前需先擦除），返回实际写入字节数（失败返回负数）。
 * 通过 lseek + write 访问。
 */
static int __write(Mtd *mtd, uint32_t offset, const void *buf, uint32_t size)
{
    ssize_t nwrite;
    int ret = -1;

    TRY {
        THROW_IF(mtd == NULL || buf == NULL, -1);
        THROW_IF(mtd->fd < 0, -MTD_ERR_NOT_INIT);
        THROW_IF(offset + size > mtd->info.size, -MTD_ERR_INVALID_ARG);

        /* 1. 加锁，定位到 offset */
        pthread_mutex_lock(&mtd->lock);
        ret = lseek(mtd->fd, offset, SEEK_SET);
        THROW_IF(ret < 0, -MTD_ERR_WRITE);

        /* 2. 写入数据 */
        nwrite = write(mtd->fd, buf, size);
        THROW_IF(nwrite < 0, -MTD_ERR_WRITE);
        pthread_mutex_unlock(&mtd->lock);

        dbg_str(DBG_DETAIL, "mtd write ok, offset:0x%x, size:0x%x, nwrite:%zd",
                offset, size, nwrite);
        ret = (int)nwrite;
    } CATCH (ret) {
        pthread_mutex_unlock(&mtd->lock);
        dbg_str(DBG_ERROR, "mtd write failed, offset:0x%x, size:0x%x, "
                "errno:%d(%s)", offset, size, errno, strerror(errno));
    }

    return ret;
}

static int __construct(Mtd *module, char *init_str)
{
    module->fd = -1;
    module->dev_path = NULL;
    memset(&module->info, 0, sizeof(module->info));
    pthread_mutex_init(&module->lock, NULL);
    return 0;
}

static int __deconstruct(Mtd *module)
{
    if (module->fd >= 0) {
        module->close(module);
    }
    pthread_mutex_destroy(&module->lock);
    return 0;
}

DEFINE_CLASS(
    EXTENDS(Mtd, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(get_info, __get_info),
    Class_VFunc_Entry(erase, __erase),
    Class_VFunc_Entry(read, __read),
    Class_VFunc_Entry(write, __write)
);
