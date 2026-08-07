/**
 * @file I2c.c
 * @Synopsis  通用用户态 I2C 驱动基类。
 * 基于 Linux i2c-dev 驱动（/dev/i2c-N）实现用户空间 I2C 访问。
 * 核心设计：
 *   - 以 I2C_RDWR ioctl 实现真正的原子事务（单次调用提交多个 i2c_msg）。
 *   - 复用 fd + 进程内 pthread_mutex，实现单进程多线程场景下的线程安全。
 *   - 支持多总线（open 时指定 bus_number）。
 *   - 区分 NACK、总线错误、超时、非法参数等错误码。
 * @author alan lin
 * @version
 * @date 2026-08-07
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
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <libobject/drivers/i2c/I2c.h>
#include <libobject/core/utils/dbg/debug.h>

#define I2C_DEV_PATH_PREFIX   "/dev/i2c-"
#define I2C_DEV_PATH_MAX_LEN  64
#define I2C_ADDR_MIN          0x08   /* 7 位地址合法范围：0x08-0x77 */
#define I2C_ADDR_MAX          0x77
/* 非法寄存器地址哨兵值：read 时 reg == I2C_REG_INVALID 表示直接读（不写寄存器地址） */
#define I2C_REG_INVALID       0xFFFF

/*
 * 校验 7 位 I2C 从机地址是否合法。
 * 返回 0 合法，-1 非法。
 */
static int __check_addr(int addr)
{
    if (addr < I2C_ADDR_MIN || addr > I2C_ADDR_MAX) {
        dbg_str(DBG_ERROR, "invalid i2c addr:0x%x, valid range 0x%x-0x%x",
                addr, I2C_ADDR_MIN, I2C_ADDR_MAX);
        return -1;
    }
    return 0;
}

/*
 * 打开 /dev/i2c-N（bus_number 指定总线号）。
 * 复用 fd：初始化时打开一次，后续操作复用，避免每次 open/close 开销。
 * 同时初始化进程内互斥锁。
 */
static int __open(I2c *i2c, int bus_number)
{
    char dev_path[I2C_DEV_PATH_MAX_LEN] = {0};
    int ret = -1;

    TRY {
        THROW_IF(i2c == NULL, -1);
        THROW_IF(bus_number < 0, -1);

        /* 1. 若已打开，先关闭（支持重新指定总线） */
        if (i2c->fd >= 0) {
            close(i2c->fd);
            i2c->fd = -1;
        }

        /* 2. 拼接设备路径 /dev/i2c-N */
        snprintf(dev_path, sizeof(dev_path), "%s%d",
                 I2C_DEV_PATH_PREFIX, bus_number);

        /* 3. 打开设备文件（复用 fd） */
        i2c->fd = open(dev_path, O_RDWR);
        THROW_IF(i2c->fd < 0, -1);

        i2c->bus_number = bus_number;

        dbg_str(DBG_INFO, "i2c open success, dev:%s, fd:%d",
                dev_path, i2c->fd);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "i2c open failed, dev:%s, errno:%d(%s)",
                dev_path, errno, strerror(errno));
        if (i2c->fd >= 0) {
            close(i2c->fd);
            i2c->fd = -1;
        }
    }

    return ret;
}

/*
 * 关闭设备，销毁互斥锁。
 */
static int __close(I2c *i2c)
{
    if (i2c == NULL) {
        return -1;
    }

    if (i2c->fd >= 0) {
        close(i2c->fd);
        i2c->fd = -1;
    }

    dbg_str(DBG_INFO, "i2c close ok, bus:%d", i2c->bus_number);

    return 0;
}

/*
 * 原子事务（核心）：基于 I2C_RDWR ioctl，一次提交多个 i2c_msg。
 * 单次 ioctl 调用保证整个事务在总线上不被其他驱动/进程打断。
 *
 * 加锁区间覆盖整个 ioctl 调用，保证单进程内多线程互斥。
 * 对瞬时错误（EAGAIN/EBUSY）按 retry 次数重试。
 *
 * 返回 0 成功，负数为错误码（i2c_return_value_t 取负）。
 */
static int __transfer(I2c *i2c, int slave_addr, struct i2c_msg *msgs, int nmsgs)
{
    struct i2c_rdwr_ioctl_data data;
    int i, ret = -1, attempt;

    TRY {
        THROW_IF(i2c == NULL || msgs == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(i2c->fd < 0, -I2C_ERR_ERROR);
        THROW_IF(nmsgs <= 0, -I2C_ERR_INVALID_ARG);
        THROW_IF(__check_addr(slave_addr) < 0, -I2C_ERR_INVALID_ARG);

        /* 统一设置每个 msg 的从机地址 */
        for (i = 0; i < nmsgs; i++) {
            msgs[i].addr = (__u16)slave_addr;
        }

        data.msgs = msgs;
        data.nmsgs = nmsgs;

        /* 加锁，保护整个原子事务 */
        pthread_mutex_lock(&i2c->lock);

        for (attempt = 0; attempt <= i2c->retry; attempt++) {
            ret = ioctl(i2c->fd, I2C_RDWR, &data);
            if (ret >= 0) {
                break;
            }
            /* 瞬时错误（总线忙等）按重试次数重试 */
            if (errno == EAGAIN || errno == EBUSY) {
                dbg_str(DBG_WARN, "i2c transfer busy, retry:%d/%d, errno:%d(%s)",
                        attempt, i2c->retry, errno, strerror(errno));
                continue;
            }
            break;
        }

        pthread_mutex_unlock(&i2c->lock);

        THROW_IF(ret < 0, -1);
        ret = 0;
    } CATCH (ret) {
        if (ret == -1) {
            /* 根据 errno 映射错误码 */
            if (errno == ENXIO || errno == EREMOTEIO) {
                ret = -I2C_ERR_NACK;
            } else if (errno == ETIMEDOUT) {
                ret = -I2C_ERR_TIMEOUT;
            } else if (errno == EIO) {
                ret = -I2C_ERR_BUS;
            } else {
                ret = -I2C_ERR_ERROR;
            }
            dbg_str(DBG_ERROR, "i2c transfer failed, addr:0x%x, nmsgs:%d, "
                    "errno:%d(%s), ret:%d", slave_addr, nmsgs, errno,
                    strerror(errno), ret);
        }
    }

    return ret;
}

/*
 * 读：reg 为寄存器地址，write_buf 为写数据（不含寄存器地址，可为空）。
 *
 * 为什么需要 write_buf：I2C 读指定寄存器前，必须先通过写操作告诉从机
 * 要读哪个寄存器（I2C 协议读操作无法携带寄存器地址）。write_buf 用于
 * 支持"寄存器地址 + 附加命令/参数"的多字节写场景（某些芯片读前需发命令）。
 * 若只需读单个寄存器，write_buf 传 NULL、write_size 传 0 即可。
 *
 * 若 reg 为合法寄存器地址（非 I2C_REG_INVALID），先写 [reg][write_buf...]
 * 再读（读指定寄存器）；若 reg == I2C_REG_INVALID，则直接读（不写寄存器地址）。
 * reg 按 reg_width（8 或 16 位）编码发送。
 * 两个 msg 在一个 I2C_RDWR 事务内完成，保证原子性。
 */
static int __read(I2c *i2c, int slave_addr, uint16_t reg,
                  const void *write_buf, size_t write_size,
                  void *read_buf, size_t read_size)
{
    struct i2c_msg msgs[2];
    uint8_t txbuf[256];
    int reg_len, nmsgs = 1;
    int ret = -1;

    TRY {
        THROW_IF(i2c == NULL || read_buf == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(read_size == 0, -I2C_ERR_INVALID_ARG);

        if (reg != I2C_REG_INVALID) {
            /* 读指定寄存器：先写 [reg][write_buf...] 再读 */
            if (i2c->reg_width == 16) {
                /* 16 位地址：高字节在前（大端） */
                txbuf[0] = (uint8_t)(reg >> 8);
                txbuf[1] = (uint8_t)(reg & 0xFF);
                reg_len = 2;
            } else {
                txbuf[0] = (uint8_t)(reg & 0xFF);
                reg_len = 1;
            }
            THROW_IF(write_size + reg_len > 256, -I2C_ERR_INVALID_ARG);

            if (write_size > 0) {
                THROW_IF(write_buf == NULL, -I2C_ERR_INVALID_ARG);
                memcpy(txbuf + reg_len, write_buf, write_size);
            }

            msgs[0].addr = (__u16)slave_addr;
            msgs[0].flags = 0;
            msgs[0].len = (__u16)(write_size + reg_len);
            msgs[0].buf = txbuf;

            nmsgs = 2;
        }

        /* 读数据 msg（reg == I2C_REG_INVALID 时作为唯一 msg，直接读） */
        msgs[nmsgs - 1].addr = (__u16)slave_addr;
        msgs[nmsgs - 1].flags = I2C_M_RD;
        msgs[nmsgs - 1].len = (__u16)read_size;
        msgs[nmsgs - 1].buf = (__u8 *)read_buf;

        /* 抛出 transfer 的错误码（如 -I2C_ERR_NACK），不抹平为 -1 */
        ret = i2c->transfer(i2c, slave_addr, msgs, nmsgs);
        THROW_IF(ret < 0, ret);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "i2c read failed, addr:0x%x, reg:0x%x, "
                "wsize:%d, rsize:%d", slave_addr, reg,
                (int)write_size, (int)read_size);
    }

    return ret;
}

/*
 * 仅写：reg 为起始寄存器地址，buf 为要写入的数据（不含寄存器地址）。
 * 内部构造 [reg][buf...] 发送，支持连续写多个寄存器（指针自动递增）。
 * reg 按 reg_width（8 或 16 位）编码发送。
 */
static int __write(I2c *i2c, int slave_addr, uint16_t reg,
                   const void *buf, size_t size)
{
    struct i2c_msg msg;
    uint8_t txbuf[256];
    int reg_len, ret = -1;

    TRY {
        THROW_IF(i2c == NULL || buf == NULL, -I2C_ERR_INVALID_ARG);
        THROW_IF(size == 0, -I2C_ERR_INVALID_ARG);

        /* 按 reg_width 编码寄存器地址 */
        if (i2c->reg_width == 16) {
            txbuf[0] = (uint8_t)(reg >> 8);
            txbuf[1] = (uint8_t)(reg & 0xFF);
            reg_len = 2;
        } else {
            txbuf[0] = (uint8_t)(reg & 0xFF);
            reg_len = 1;
        }
        /* 寄存器地址 + 数据，不能超过 i2c_msg 的 len 上限 */
        THROW_IF(size + reg_len > 256, -I2C_ERR_INVALID_ARG);

        /* 构造 [reg][buf...]，buf 只含数据，不含地址 */
        memcpy(txbuf + reg_len, buf, size);

        msg.addr = (__u16)slave_addr;
        msg.flags = 0;
        msg.len = (__u16)(size + reg_len);
        msg.buf = txbuf;

        /* 抛出 transfer 的错误码（如 -I2C_ERR_NACK），不抹平为 -1 */
        ret = i2c->transfer(i2c, slave_addr, &msg, 1);
        THROW_IF(ret < 0, ret);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "i2c write failed, addr:0x%x, reg:0x%x, size:%d",
                slave_addr, reg, (int)size);
    }

    return ret;
}

/*
 * 设置瞬时错误重试次数。
 */
static int __set_retry(I2c *i2c, int retry)
{
    if (i2c == NULL || retry < 0) {
        return -1;
    }
    i2c->retry = retry;
    return 0;
}

/*
 * 设置寄存器地址宽度：8 或 16 位，默认 8 位。
 */
static int __set_width(I2c *i2c, int width)
{
    if (i2c == NULL) {
        return -1;
    }
    if (width != 8 && width != 16) {
        dbg_str(DBG_ERROR, "unsupported reg width:%d, only 8 or 16", width);
        return -1;
    }
    i2c->reg_width = width;
    return 0;
}

static int __construct(I2c *module, char *init_str)
{
    module->fd = -1;
    module->bus_number = -1;
    module->retry = 0;
    module->reg_width = 8;   /* 默认 8 位寄存器地址 */
    pthread_mutex_init(&module->lock, NULL);
    return 0;
}

static int __deconstruct(I2c *module)
{
    if (module->fd >= 0) {
        module->close(module);
    }
    pthread_mutex_destroy(&module->lock);
    return 0;
}

DEFINE_CLASS(I2c,
    Class_Obj___Entry(Obj, parent),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(transfer, __transfer),
    Class_VFunc_Entry(write, __write),
    Class_VFunc_Entry(read, __read),
    Class_VFunc_Entry(set_retry, __set_retry),
    Class_VFunc_Entry(set_width, __set_width)
);
