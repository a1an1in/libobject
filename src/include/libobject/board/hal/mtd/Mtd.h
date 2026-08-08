#ifndef __MTD_H__
#define __MTD_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <libobject/core/Obj.h>

/*
 * Mtd 类：通用用户态 MTD（Memory Technology Device）驱动基类。
 *
 * 基于 Linux MTD 子系统（/dev/mtd*）实现用户空间 Flash 访问。
 * 核心设计：
 *   - open 时通过 ioctl(MEMGETINFO) 获取设备信息（size/erasesize/writesize/oobsize）。
 *   - erase 通过 ioctl(MEMERASE) 按擦除块擦除（块对齐）。
 *   - read/write 通过 lseek + read/write 访问（写入前需先擦除）。
 *   - 复用 fd + 进程内 pthread_mutex，实现单进程多线程场景下的线程安全。
 *   - 支持多分区（open 时指定设备路径，如 /dev/mtd0 或 /dev/flash/spider_meta）。
 *
 * 继承关系：Obj -> Mtd
 */

typedef struct Mtd_s Mtd;

/* MTD 返回值/错误码 */
typedef enum mtd_return_value {
    MTD_ERR_OK          = 0x0,   /* 成功 */
    MTD_ERR_ERROR       = 0x2,   /* 通用错误 */
    MTD_ERR_OPEN        = 0x3,   /* 打开失败 */
    MTD_ERR_ERASE       = 0x4,   /* 擦除失败 */
    MTD_ERR_WRITE       = 0x5,   /* 写失败 */
    MTD_ERR_READ        = 0x6,   /* 读失败 */
    MTD_ERR_INVALID_ARG = 0x7,   /* 非法参数 */
    MTD_ERR_NOT_INIT    = 0x8,   /* 未初始化 */
} mtd_return_value_t;

/* MTD 设备信息（对应内核 struct mtd_info_user） */
typedef struct mtd_dev_info {
    uint32_t size;        /* 设备总大小（字节） */
    uint32_t erasesize;   /* 擦除块大小（字节） */
    uint32_t writesize;   /* 最小写单元（字节） */
    uint32_t oobsize;     /* OOB 区大小（字节，NAND 有效） */
    uint32_t type;        /* MTD 类型（MTD_NORFLASH/MTD_NANDFLASH 等） */
} mtd_dev_info_t;

struct Mtd_s {
    Obj parent;

    int (*construct)(Mtd *, char *);
    int (*deconstruct)(Mtd *);

    /*virtual methods reimplement*/
    int (*set)(Mtd *module, char *attrib, void *value);
    void *(*get)(Mtd *, char *attrib);
    char *(*to_json)(Mtd *);

    /* MTD device interface */
    /* 打开 /dev/mtdN（device 指定设备路径，如 "/dev/mtd0"），
     * 复用 fd，初始化互斥锁，并通过 MEMGETINFO 获取设备信息 */
    int (*open)(Mtd *mtd, char *device);
    /* 关闭设备，销毁互斥锁 */
    int (*close)(Mtd *mtd);
    /* 获取设备信息（size/erasesize/writesize/oobsize/type） */
    int (*get_info)(Mtd *mtd, mtd_dev_info_t *info);
    /* 擦除：offset 和 size 必须按 erasesize 对齐，按块擦除 */
    int (*erase)(Mtd *mtd, uint32_t offset, uint32_t size);
    /* 读：从 offset 读取 size 字节到 buf，返回实际读取字节数（失败返回负数） */
    int (*read)(Mtd *mtd, uint32_t offset, void *buf, uint32_t size);
    /* 写：从 offset 写入 size 字节（写入前需先擦除），返回实际写入字节数（失败返回负数） */
    int (*write)(Mtd *mtd, uint32_t offset, const void *buf, uint32_t size);

    /*attribs*/
    int fd;                 /* /dev/mtdN 文件描述符（复用），-1 表示未打开 */
    char *dev_path;         /* 设备路径，如 "/dev/mtd0" */
    pthread_mutex_t lock;   /* 进程内互斥锁，保护所有操作 */
    mtd_dev_info_t info;    /* 设备信息（open 时获取） */
};

#endif
