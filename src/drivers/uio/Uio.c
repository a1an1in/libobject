/**
 * @file Uio.c
 * @Synopsis  因为uio会依赖worker而且是driver的基础， 所有单独
 * 立个库， 负责driver开发。
 * @author alan lin
 * @version 
 * @date 2024-03-26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>
#include <libobject/drivers/uio/Uio.h>
#include <libobject/core/utils/dbg/debug.h>

#define UIO_SYSFS_PATH   "/sys/class/uio"
#define UIO_DEV_PATH     "/dev"
#define UIO_MAX_NAME_LEN 256
#define UIO_MAX_DEV_NUM  256

/*
 * 根据 UIO 设备名（/sys/class/uio/uioX/name）匹配设备编号 X，
 * 返回 /dev/uioX 路径。
 */
static int __find_uio_dev(char *name, char *dev_path, int dev_path_len)
{
    char sysfs_name[UIO_MAX_NAME_LEN] = {0};
    char sysfs_path[UIO_MAX_NAME_LEN] = {0};
    char buf[UIO_MAX_NAME_LEN] = {0};
    int i, fd, len;

    for (i = 0; i < UIO_MAX_DEV_NUM; i++) {
        snprintf(sysfs_path, sizeof(sysfs_path), "%s/uio%d/name",
                 UIO_SYSFS_PATH, i);
        fd = open(sysfs_path, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        len = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        if (len <= 0) {
            continue;
        }
        buf[len] = '\0';
        /* name 文件内容以 '\n' 结尾，去掉换行 */
        if (buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
        if (strcmp(buf, name) == 0) {
            snprintf(dev_path, dev_path_len, "%s/uio%d", UIO_DEV_PATH, i);
            return 0;
        }
    }

    return -1;
}

static int __open(Uio *uio, char *name)
{
    char dev_path[UIO_MAX_NAME_LEN] = {0};
    int ret = -1;

    TRY {
        THROW_IF(uio == NULL || name == NULL, -1);

        /* 1. 通过 /sys/class/uio/uioX/name 匹配设备编号 */
        ret = __find_uio_dev(name, dev_path, sizeof(dev_path));
        THROW_IF(ret < 0, -1);

        /* 2. 打开 /dev/uioX，O_SYNC 保证寄存器访问不被缓存 */
        uio->fd = open(dev_path, O_RDWR | O_SYNC);
        THROW_IF(uio->fd < 0, -1);

        /* 3. 记录设备信息 */
        if (uio->name == NULL) {
            uio->name = strdup(name);
        } else {
            free(uio->name);
            uio->name = strdup(name);
        }
        if (uio->dev_path == NULL) {
            uio->dev_path = strdup(dev_path);
        } else {
            free(uio->dev_path);
            uio->dev_path = strdup(dev_path);
        }

        dbg_str(DBG_INFO, "uio open success, name:%s, dev:%s, fd:%d",
                name, dev_path, uio->fd);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "uio open failed, name:%s, errno:%d(%s)",
                name, errno, strerror(errno));
        if (uio->fd >= 0) {
            close(uio->fd);
            uio->fd = -1;
        }
    }

    return ret;
}

static int __mmap(Uio *uio)
{
    char sysfs_path[UIO_MAX_NAME_LEN] = {0};
    char buf[UIO_MAX_NAME_LEN] = {0};
    unsigned long long addr = 0, size = 0;
    int fd, len, ret = -1;

    TRY {
        THROW_IF(uio == NULL || uio->fd < 0, -1);

        /* 1. 从 /sys/class/uio/uioX/maps/map0/size 读取映射大小 */
        snprintf(sysfs_path, sizeof(sysfs_path),
                 "%s/uio%d/maps/map0/size", UIO_SYSFS_PATH,
                 atoi(uio->dev_path + strlen(UIO_DEV_PATH) + 3));
        fd = open(sysfs_path, O_RDONLY);
        THROW_IF(fd < 0, -1);
        len = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        THROW_IF(len <= 0, -1);
        buf[len] = '\0';
        size = strtoull(buf, NULL, 0);

        /* 2. 页对齐后的映射大小 */
        size = (size + getpagesize() - 1) & ~(getpagesize() - 1);

        /* 3. mmap 映射 FPGA 寄存器空间 */
        uio->base = mmap(NULL, size, PROT_READ | PROT_WRITE,
                         MAP_SHARED, uio->fd, 0);
        THROW_IF(uio->base == MAP_FAILED, -1);

        uio->size = size;
        dbg_str(DBG_INFO, "uio mmap success, base:%p, size:0x%x",
                uio->base, uio->size);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "uio mmap failed, errno:%d(%s)",
                errno, strerror(errno));
        if (uio->base != NULL && uio->base != MAP_FAILED) {
            munmap(uio->base, uio->size);
            uio->base = NULL;
        }
    }

    return ret;
}

static int __get_size(Uio *uio)
{
    if (uio == NULL) {
        return -1;
    }
    return (int)uio->size;
}

static int __set_width(Uio *uio, int width)
{
    if (uio == NULL) {
        return -1;
    }
    if (width != 32 && width != 64) {
        dbg_str(DBG_ERROR, "unsupported register width:%d, only 32 or 64", width);
        return -1;
    }

    uio->width = width;
    dbg_str(DBG_INFO, "set register width:%d", width);

    return 0;
}

static int __read_register(Uio *uio, uint64_t offset, uint64_t *data)
{
    uint8_t volatile *base;

    if (uio == NULL || uio->base == NULL || data == NULL) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }
    if (offset >= uio->size) {
        return -1;
    }

    /* 用一个字节基址指针，按位宽强制转换为对应类型访问 */
    base = (uint8_t volatile *)uio->base + offset;
    if (uio->width == 64) {
        *data = *(uint64_t volatile *)base;
    } else {
        *data = *(uint32_t volatile *)base;
    }

    return 0;
}

static int __write_register(Uio *uio, uint64_t offset, uint64_t data)
{
    uint8_t volatile *base;

    if (uio == NULL || uio->base == NULL) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }
    if (offset >= uio->size) {
        return -1;
    }

    base = (uint8_t volatile *)uio->base + offset;
    if (uio->width == 64) {
        *(uint64_t volatile *)base = data;
    } else {
        *(uint32_t volatile *)base = (uint32_t)data;
    }

    return 0;
}

static int __read_registers(Uio *uio, uint64_t offset, uint64_t *data,
                            uint32_t len)
{
    uint8_t volatile *base;
    uint32_t i, count, reg_size;

    if (uio == NULL || uio->base == NULL || data == NULL) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }
    if (offset >= uio->size) {
        dbg_str(DBG_ERROR, "read offset out of range, offset:0x%llx, size:0x%x",
                (unsigned long long)offset, uio->size);
        return -1;
    }

    /* len 表示期望读取的寄存器个数 */
    reg_size = (uio->width == 64) ? 8 : 4;
    count = len;

    /* 防止读取超出 UIO 范围，截断寄存器个数 */
    if (offset + count * reg_size > uio->size) {
        count = (uio->size - offset) / reg_size;
    }

    /* 用一个字节基址指针，按位宽用不同步长和类型访问 */
    base = (uint8_t volatile *)uio->base + offset;
    for (i = 0; i < count; i++) {
        if (uio->width == 64) {
            data[i] = *(uint64_t volatile *)(base + i * 8);
        } else {
            data[i] = *(uint32_t volatile *)(base + i * 4);
        }
    }

    /* 返回实际读取的寄存器个数 */
    return (int)count;
}

static int __write_registers(Uio *uio, uint64_t offset, uint64_t *data,
                             uint32_t len)
{
    uint8_t volatile *base;
    uint32_t i, count, reg_size;

    if (uio == NULL || uio->base == NULL || data == NULL) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }
    if (offset >= uio->size) {
        dbg_str(DBG_ERROR, "write offset out of range, offset:0x%llx, size:0x%x",
                (unsigned long long)offset, uio->size);
        return -1;
    }

    /* len 表示期望写入的寄存器个数 */
    reg_size = (uio->width == 64) ? 8 : 4;
    count = len;

    /* 防止写入超出 UIO 范围，截断寄存器个数 */
    if (offset + count * reg_size > uio->size) {
        count = (uio->size - offset) / reg_size;
    }

    /* 用一个字节基址指针，按位宽用不同步长和类型访问 */
    base = (uint8_t volatile *)uio->base + offset;
    for (i = 0; i < count; i++) {
        if (uio->width == 64) {
            *(uint64_t volatile *)(base + i * 8) = data[i];
        } else {
            *(uint32_t volatile *)(base + i * 4) = (uint32_t)data[i];
        }
    }

    /* 返回实际写入的寄存器个数 */
    return (int)count;
}

/*
 * 使能中断：向 /dev/uioX 写入 1，使能 UIO 中断。
 */
static int __enable_irq(Uio *uio)
{
    uint32_t enable = 1;
    ssize_t ret;

    if (uio == NULL || uio->fd < 0) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }

    ret = write(uio->fd, &enable, sizeof(enable));
    if (ret != sizeof(enable)) {
        dbg_str(DBG_ERROR, "enable irq failed, errno:%d(%s)",
                errno, strerror(errno));
        return -1;
    }

    uio->irq_enabled = 1;
    return 0;
}

/*
 * 禁用中断：向 /dev/uioX 写入 0。
 */
static int __disable_irq(Uio *uio)
{
    uint32_t disable = 0;
    ssize_t ret;

    if (uio == NULL || uio->fd < 0) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }

    ret = write(uio->fd, &disable, sizeof(disable));
    if (ret != sizeof(disable)) {
        dbg_str(DBG_ERROR, "disable irq failed, errno:%d(%s)",
                errno, strerror(errno));
        return -1;
    }

    uio->irq_enabled = 0;
    return 0;
}

/*
 * 等待中断：阻塞读取 /dev/uioX，返回中断计数。
 * timeout_ms < 0 表示无限等待；0 表示非阻塞；>0 表示超时毫秒。
 */
static int __wait_irq(Uio *uio, int timeout_ms)
{
    struct pollfd pfd;
    uint32_t irq_count = 0;
    ssize_t ret;
    int poll_ret;

    if (uio == NULL || uio->fd < 0) {
        dbg_str(DBG_ERROR, "uio not initialized");
        return -1;
    }

    if (timeout_ms >= 0) {
        pfd.fd = uio->fd;
        pfd.events = POLLIN;
        poll_ret = poll(&pfd, 1, timeout_ms);
        if (poll_ret == 0) {
            dbg_str(DBG_WARN, "wait irq timeout");
            return -1;
        }
        if (poll_ret < 0) {
            dbg_str(DBG_ERROR, "poll failed, errno:%d(%s)",
                    errno, strerror(errno));
            return -1;
        }
    }

    ret = read(uio->fd, &irq_count, sizeof(irq_count));
    if (ret != sizeof(irq_count)) {
        dbg_str(DBG_ERROR, "read irq failed, errno:%d(%s)",
                errno, strerror(errno));
        return -1;
    }

    return (int)irq_count;
}

static int __close(Uio *uio)
{
    if (uio == NULL) {
        return -1;
    }

    if (uio->base != NULL && uio->base != MAP_FAILED) {
        munmap(uio->base, uio->size);
        uio->base = NULL;
    }
    if (uio->fd >= 0) {
        close(uio->fd);
        uio->fd = -1;
    }
    if (uio->name != NULL) {
        free(uio->name);
        uio->name = NULL;
    }
    if (uio->dev_path != NULL) {
        free(uio->dev_path);
        uio->dev_path = NULL;
    }

    return 0;
}

static int __construct(Uio *module, char *init_str)
{
    module->fd = -1;
    module->base = NULL;
    module->size = 0;
    module->width = 32;   /* 默认 32 位寄存器 */
    module->irq_enabled = 0;
    return 0;
}

static int __deconstruct(Uio *module)
{
    if (module->fd >= 0 || module->base != NULL) {
        module->close(module);
    }
    return 0;
}

DEFINE_CLASS(Uio,
    Class_Obj___Entry(Obj, parent),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(mmap, __mmap),
    Class_VFunc_Entry(get_size, __get_size),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(set_width, __set_width),
    Class_VFunc_Entry(read_register, __read_register),
    Class_VFunc_Entry(write_register, __write_register),
    Class_VFunc_Entry(read_registers, __read_registers),
    Class_VFunc_Entry(write_registers, __write_registers),
    Class_VFunc_Entry(enable_irq, __enable_irq),
    Class_VFunc_Entry(disable_irq, __disable_irq),
    Class_VFunc_Entry(wait_irq, __wait_irq)
);
