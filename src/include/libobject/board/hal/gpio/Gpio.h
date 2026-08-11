#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>

/*
 * Gpio 类：通用用户态 GPIO 驱动（基于 Linux GPIO character device）。
 *
 * 基于 /dev/gpiochipN（character device + ioctl）实现用户空间 GPIO 访问，
 * 替代已弃用的 /sys/class/gpio。核心设计：
 *   - open：打开 /dev/gpiochipN，通过 GPIO_GET_CHIPINFO_IOCTL 获取 chip 信息
 *     （name/label/num_lines）。
 *   - request_lines：通过 GPIO_GET_LINEHANDLE_IOCTL 一次请求多条 line
 *     （输入/输出，可配 active_low/open_drain/open_source；同一批次 flags 相同）。
 *   - set/get value(s)：通过 GPIOHANDLE_(SET|GET)_LINE_VALUES_IOCTL 读写电平，
 *     按请求顺序的 index 访问。
 *   - register_event：请求某条 line 的边沿事件（GPIO_GET_LINEEVENT_IOCTL，按 line
 *     offset；事件请求本身会占用该 line，无需先 request_lines）+ 用 io_worker 异步
 *     监听（参考 Uio.register_irq），事件到来时异步回调 handler。
 *   - 复用 fd + 进程内 pthread_mutex，实现单进程多线程场景下的线程安全。
 *
 * 继承关系：Obj -> Gpio
 */

typedef struct Gpio_s Gpio;

/* GPIO 方向 */
typedef enum gpio_direction {
    GPIO_IN  = 0,
    GPIO_OUT = 1,
} gpio_direction_t;

/* GPIO 电平 */
typedef enum gpio_value {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,
} gpio_value_t;

/* 边沿事件类型（与内核 GPIOEVENT_REQUEST_* 一致） */
typedef enum gpio_edge {
    GPIO_EDGE_RISING  = 0x1,   /* 上升沿 */
    GPIO_EDGE_FALLING = 0x2,   /* 下降沿 */
    GPIO_EDGE_BOTH    = 0x3,   /* 双边沿 */
} gpio_edge_t;

/* GPIO chip 信息 */
typedef struct gpio_dev_info {
    char     name[32];    /* chip 名，如 "gpiochip0" */
    char     label[32];   /* chip label，如 "zynq_gpio" */
    uint32_t num_lines;   /* line 数量 */
} gpio_dev_info_t;

/* 单条 line 请求配置 */
typedef struct gpio_line_config {
    int              offset;      /* chip 内 line 偏移 */
    gpio_direction_t dir;         /* 输入/输出 */
    int              active_low;  /* 是否低有效 */
    int              open_drain;  /* 开漏 */
    int              open_source; /* 开源 */
    int              init_val;    /* 输出初始电平（仅输出有效） */
} gpio_line_config_t;

/* GPIO 边沿事件 */
typedef struct gpio_event {
    uint64_t timestamp;   /* 事件时间戳（ns） */
    int      id;          /* GPIO_EDGE_RISING / GPIO_EDGE_FALLING */
} gpio_event_t;

/* GPIO 异步事件回调（参考 Uio 的 uio_irq_handler_t）。
 * opaque 为 io_worker 传入的 worker，可通过 ((Worker *)opaque)->opaque
 * 获取 Gpio 对象，进而读 gpio->event（最新事件）与 gpio->ev_opaque（用户数据）。 */
typedef int (*gpio_irq_handler_t)(void *opaque);

struct Gpio_s {
    Obj parent;

    int (*construct)(Gpio *, char *);
    int (*deconstruct)(Gpio *);

    /*virtual methods reimplement*/
    int (*set)(Gpio *module, char *attrib, void *value);
    void *(*get)(Gpio *, char *attrib);
    char *(*to_json)(Gpio *);

    /* GPIO character device interface */
    /* 打开 /dev/gpiochipN（chip_path 如 "/dev/gpiochip0"），获取 chip 信息 */
    int (*open)(Gpio *gpio, char *chip_path);
    int (*close)(Gpio *gpio);
    /* 获取 chip 信息（name/label/num_lines） */
    int (*get_info)(Gpio *gpio, gpio_dev_info_t *info);
    /* 一次请求多条 line（同一批次 flags 相同），按 configs 顺序记为 index 0..num-1 */
    int (*request_lines)(Gpio *gpio, gpio_line_config_t *configs, int num);
    /* 按请求顺序的 index 读/写单条 line */
    int (*set_value)(Gpio *gpio, int index, int value);
    int (*get_value)(Gpio *gpio, int index, int *value);
    /* 批量读/写（values 与请求顺序对应，num 须等于已请求 line 数） */
    int (*set_values)(Gpio *gpio, int *values, int num);
    int (*get_values)(Gpio *gpio, int *values, int num);
    /* 注册某条 line 的边沿事件（offset 为 chip 内 line 编号）：一条 line 同一时刻
     * 只有一个事件请求（GPIO_GET_LINEEVENT_IOCTL → 一个事件 fd，事件请求本身会
     * 占用该 line，无需先 request_lines）；用 io_worker 异步监听，每次边沿事件
     * 到来时异步回调 handler（handler 通过 gpio->event 读取事件）。参考 Uio.register_irq。 */
    int (*register_event)(Gpio *gpio, int offset, gpio_edge_t edge,
                          gpio_irq_handler_t handler, void *opaque);

    /*attribs*/
    int fd;                 /* /dev/gpiochipN 文件描述符，-1 表示未打开 */
    int req_fd;             /* line handle fd（request_lines 后有效），-1 表示未请求 */
    int ev_fd;              /* line event fd（request_events 后有效），-1 表示未请求 */
    char *dev_path;         /* 设备路径，如 "/dev/gpiochip0" */
    pthread_mutex_t lock;   /* 进程内互斥锁，保护所有操作 */
    gpio_dev_info_t info;   /* chip 信息（open 时获取） */
    int line_offsets[64];   /* 已请求 line 的 offset（按请求顺序） */
    int num_lines;          /* 已请求 line 数量 */

    /* async event (io_worker) fields */
    Worker *ev_worker;          /* io_worker 监听 gpio 事件 fd（异步） */
    gpio_irq_handler_t ev_handler; /* 用户注册的事件处理函数 */
    void *ev_opaque;            /* 传给事件处理函数的用户数据 */
    gpio_event_t event;         /* 最近一次事件（供 handler 读取） */
};

#endif
