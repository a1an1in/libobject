/**
 * @file Gpio.c
 * @Synopsis  通用用户态 GPIO 驱动，基于 Linux GPIO character device
 * （/dev/gpiochipN + ioctl），替代已弃用的 /sys/class/gpio。
 * 支持：chip 信息查询、批量请求 line（输入/输出）、电平读写、
 * 边沿事件（中断）监听。无外部依赖（直接用内核 ioctl）。
 * @author alan lin
 * @version
 * @date 2026-08-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <libobject/board/hal/gpio/Gpio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/concurrent/worker_api.h>
#include <libobject/concurrent/Producer.h>

#define GPIO_MAX_LINES   64
#define GPIO_CONSUMER    "libobject-gpio"

static int __open(Gpio *gpio, char *chip_path)
{
    struct gpiochip_info cinfo;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || chip_path == NULL, -1);
        THROW_IF(gpio->fd >= 0, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&cinfo, 0, sizeof(cinfo));

        /* 1. 打开 /dev/gpiochipN */
        EXEC(gpio->fd = open(chip_path, O_RDONLY | O_CLOEXEC));

        if (gpio->dev_path == NULL) {
            gpio->dev_path = strdup(chip_path);
        } else {
            free(gpio->dev_path);
            gpio->dev_path = strdup(chip_path);
        }
        THROW_IF(gpio->dev_path == NULL, -1);

        /* 2. 获取 chip 信息 */
        EXEC(ioctl(gpio->fd, GPIO_GET_CHIPINFO_IOCTL, &cinfo));
        snprintf(gpio->info.name, sizeof(gpio->info.name), "%s", cinfo.name);
        snprintf(gpio->info.label, sizeof(gpio->info.label), "%s", cinfo.label);
        gpio->info.num_lines = cinfo.lines;

        dbg_str(DBG_INFO, "gpio open success, dev:%s, name:%s, label:%s, lines:%u",
                chip_path, gpio->info.name, gpio->info.label,
                gpio->info.num_lines);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio open failed, dev:%s, errno:%d(%s)",
                chip_path, errno, strerror(errno));
        if (gpio != NULL && gpio->fd >= 0) {
            close(gpio->fd);
            gpio->fd = -1;
        }
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __get_info(Gpio *gpio, gpio_dev_info_t *info)
{
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || info == NULL, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        *info = gpio->info;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio get_info failed, errno:%d(%s)",
                errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __request_lines(Gpio *gpio, gpio_line_config_t *configs, int num)
{
    struct gpiohandle_request req;
    uint32_t flags = 0;
    int i, all_out = 1, ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->fd < 0 || configs == NULL, -1);
        THROW_IF(num <= 0 || num > GPIO_MAX_LINES, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&req, 0, sizeof(req));

        for (i = 0; i < num; i++) {
            THROW_IF(configs[i].offset < 0, -1);
            req.lineoffsets[i] = (__u32)configs[i].offset;
            if (configs[i].dir != GPIO_OUT) {
                all_out = 0;
            }
            if (configs[i].active_low) {
                flags |= GPIOHANDLE_REQUEST_ACTIVE_LOW;
            }
            if (configs[i].open_drain) {
                flags |= GPIOHANDLE_REQUEST_OPEN_DRAIN;
            }
            if (configs[i].open_source) {
                flags |= GPIOHANDLE_REQUEST_OPEN_SOURCE;
            }
        }
        /* 同一批次 flags 相同（4.9 限制）：全输出则 OUTPUT，否则 INPUT */
        flags |= all_out ? GPIOHANDLE_REQUEST_OUTPUT : GPIOHANDLE_REQUEST_INPUT;
        req.flags = flags;
        req.lines = (__u32)num;
        for (i = 0; i < num; i++) {
            req.default_values[i] = (__u8)(configs[i].init_val ? 1 : 0);
        }
        snprintf(req.consumer_label, sizeof(req.consumer_label), "%s",
                 GPIO_CONSUMER);

        EXEC(ioctl(gpio->fd, GPIO_GET_LINEHANDLE_IOCTL, &req));
        THROW_IF(req.fd < 0, -1);

        /* 释放旧的 line handle（若已请求） */
        if (gpio->req_fd >= 0) {
            close(gpio->req_fd);
            gpio->req_fd = -1;
        }
        gpio->req_fd = req.fd;
        gpio->num_lines = num;
        for (i = 0; i < num; i++) {
            gpio->line_offsets[i] = configs[i].offset;
        }

        dbg_str(DBG_INFO, "gpio request_lines ok, lines:%d, flags:0x%x, req_fd:%d",
                num, flags, gpio->req_fd);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio request_lines failed, lines:%d, errno:%d(%s)",
                num, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __set_value(Gpio *gpio, int index, int value)
{
    struct gpiohandle_data data;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->req_fd < 0, -1);
        THROW_IF(index < 0 || index >= gpio->num_lines, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&data, 0, sizeof(data));
        data.values[index] = (__u8)(value ? 1 : 0);
        EXEC(ioctl(gpio->req_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL,
                   &data));
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio set_value failed, index:%d, errno:%d(%s)",
                index, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __get_value(Gpio *gpio, int index, int *value)
{
    struct gpiohandle_data data;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->req_fd < 0 || value == NULL, -1);
        THROW_IF(index < 0 || index >= gpio->num_lines, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&data, 0, sizeof(data));
        EXEC(ioctl(gpio->req_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL,
                   &data));
        *value = data.values[index] ? 1 : 0;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio get_value failed, index:%d, errno:%d(%s)",
                index, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __set_values(Gpio *gpio, int *values, int num)
{
    struct gpiohandle_data data;
    int i, ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->req_fd < 0 || values == NULL, -1);
        THROW_IF(num <= 0 || num > gpio->num_lines, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&data, 0, sizeof(data));
        for (i = 0; i < num; i++) {
            data.values[i] = (__u8)(values[i] ? 1 : 0);
        }
        EXEC(ioctl(gpio->req_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL,
                   &data));
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio set_values failed, num:%d, errno:%d(%s)",
                num, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __get_values(Gpio *gpio, int *values, int num)
{
    struct gpiohandle_data data;
    int i, ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->req_fd < 0 || values == NULL, -1);
        THROW_IF(num <= 0 || num > gpio->num_lines, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&data, 0, sizeof(data));
        EXEC(ioctl(gpio->req_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL,
                   &data));
        for (i = 0; i < num; i++) {
            values[i] = data.values[i] ? 1 : 0;
        }
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio get_values failed, num:%d, errno:%d(%s)",
                num, errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

/*
 * io_worker 的事件回调：gpio 事件 fd 可读（有边沿事件）时被异步调用。
 * 读取事件（timestamp/id）保存到 gpio->event，然后调用用户注册的
 * 事件处理函数（即 worker 的 work_callback）。
 */
static void __ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    Gpio *gpio = (Gpio *)worker->opaque;
    struct gpioevent_data kdata;
    ssize_t ret;

    /* 读取事件，清除事件状态 */
    ret = read(gpio->ev_fd, &kdata, sizeof(kdata));
    if (ret != (ssize_t)sizeof(kdata)) {
        dbg_str(DBG_ERROR, "gpio read event failed, errno:%d(%s)",
                errno, strerror(errno));
        return;
    }

    /* 保存最新事件，供 handler 读取 */
    gpio->event.timestamp = kdata.timestamp;
    gpio->event.id = (kdata.id == GPIOEVENT_EVENT_FALLING_EDGE) ?
                     GPIO_EDGE_FALLING : GPIO_EDGE_RISING;
    dbg_str(DBG_DETAIL, "gpio event, id:%d, ts:%llu",
            gpio->event.id, (unsigned long long)gpio->event.timestamp);

    /* 调用用户注册的事件处理函数（即 worker 的 work_callback） */
    if (worker->work_callback != NULL) {
        worker->work_callback(worker);
    }
}

/*
 * 注册某条 line 的边沿事件（offset 为 chip 内 line 编号）：
 *   1. GPIO_GET_LINEEVENT_IOCTL 请求事件 fd（事件请求本身占用该 line，无需先
 *      request_lines，一条 line 不能同时作 line handle 和 line event）；
 *   2. 用 io_worker 监听 ev_fd（EV_READ | EV_PERSIST），事件到来时异步回调
 *      handler（参考 Uio.register_irq）。
 */
static int __register_event(Gpio *gpio, int offset, gpio_edge_t edge,
                            gpio_irq_handler_t handler, void *opaque)
{
    struct gpioevent_request req;
    allocator_t *allocator;
    Worker *worker;
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL || gpio->fd < 0, -1);
        THROW_IF(offset < 0, -1);
        THROW_IF(handler == NULL, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        memset(&req, 0, sizeof(req));
        req.lineoffset = (__u32)offset;
        req.handleflags = GPIOHANDLE_REQUEST_INPUT;
        if (edge & GPIO_EDGE_RISING) {
            req.eventflags |= GPIOEVENT_REQUEST_RISING_EDGE;
        }
        if (edge & GPIO_EDGE_FALLING) {
            req.eventflags |= GPIOEVENT_REQUEST_FALLING_EDGE;
        }
        snprintf(req.consumer_label, sizeof(req.consumer_label), "%s",
                 GPIO_CONSUMER);

        EXEC(ioctl(gpio->fd, GPIO_GET_LINEEVENT_IOCTL, &req));
        THROW_IF(req.fd < 0, -1);

        /* 若已注册过，先注销旧的 io_worker 并关闭旧事件 fd */
        if (gpio->ev_worker != NULL) {
            worker_destroy(gpio->ev_worker);
            gpio->ev_worker = NULL;
        }
        if (gpio->ev_fd >= 0) {
            close(gpio->ev_fd);
            gpio->ev_fd = -1;
        }
        gpio->ev_fd = req.fd;
        gpio->ev_handler = handler;
        gpio->ev_opaque = opaque;

        allocator = gpio->parent.allocator;
        worker = io_worker(allocator, gpio->ev_fd, EV_READ | EV_PERSIST,
                           NULL, NULL, __ev_callback, handler, gpio);
        THROW_IF(worker == NULL, -1);
        gpio->ev_worker = worker;

        dbg_str(DBG_INFO, "gpio register_event ok, offset:%d, "
                "edge:0x%x, ev_fd:%d, worker:%p",
                offset, edge, gpio->ev_fd, worker);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio register_event failed, offset:%d, "
                "errno:%d(%s)", offset, errno, strerror(errno));
        if (gpio != NULL && gpio->ev_fd >= 0) {
            close(gpio->ev_fd);
            gpio->ev_fd = -1;
        }
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __close(Gpio *gpio)
{
    int ret = -1;
    int locked = 0;

    TRY {
        THROW_IF(gpio == NULL, -1);
        THROW_IF(pthread_mutex_lock(&gpio->lock) != 0, -1);
        locked = 1;

        if (gpio->ev_worker != NULL) {
            worker_destroy(gpio->ev_worker);
            gpio->ev_worker = NULL;
        }
        if (gpio->ev_fd >= 0) {
            close(gpio->ev_fd);
            gpio->ev_fd = -1;
        }
        if (gpio->req_fd >= 0) {
            close(gpio->req_fd);
            gpio->req_fd = -1;
        }
        if (gpio->fd >= 0) {
            close(gpio->fd);
            gpio->fd = -1;
        }
        if (gpio->dev_path != NULL) {
            free(gpio->dev_path);
            gpio->dev_path = NULL;
        }
        gpio->num_lines = 0;
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "gpio close failed, errno:%d(%s)",
                errno, strerror(errno));
    } FINALLY {
        if (locked) {
            pthread_mutex_unlock(&gpio->lock);
        }
    }

    return ret;
}

static int __construct(Gpio *module, char *init_str)
{
    module->fd = -1;
    module->req_fd = -1;
    module->ev_fd = -1;
    module->dev_path = NULL;
    module->num_lines = 0;
    module->ev_worker = NULL;
    module->ev_handler = NULL;
    module->ev_opaque = NULL;
    memset(&module->info, 0, sizeof(module->info));
    memset(module->line_offsets, 0, sizeof(module->line_offsets));
    memset(&module->event, 0, sizeof(module->event));
    pthread_mutex_init(&module->lock, NULL);
    return 0;
}

static int __deconstruct(Gpio *module)
{
    if (module->fd >= 0 || module->req_fd >= 0 || module->ev_fd >= 0 ||
        module->ev_worker != NULL) {
        module->close(module);
    }
    pthread_mutex_destroy(&module->lock);
    return 0;
}

DEFINE_CLASS(
    EXTENDS(Gpio, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open, __open),
    Class_VFunc_Entry(close, __close),
    Class_VFunc_Entry(get_info, __get_info),
    Class_VFunc_Entry(request_lines, __request_lines),
    Class_VFunc_Entry(set_value, __set_value),
    Class_VFunc_Entry(get_value, __get_value),
    Class_VFunc_Entry(set_values, __set_values),
    Class_VFunc_Entry(get_values, __get_values),
    Class_VFunc_Entry(register_event, __register_event)
);
