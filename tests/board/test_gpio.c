#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/gpio/Gpio.h>

/*
 * 测试 Gpio 类（基于 Linux GPIO character device /dev/gpiochipN）。
 *
 * 依赖：内核 CONFIG_GPIO_CDEV=y 且存在 GPIO 控制器（如 QEMU virt 的 pl061 →
 * /dev/gpiochip0）。若 guest 没有 /dev/gpiochip0，或 line 被占用，
 * 则优雅跳过对应步骤，不视为失败。
 */
static int test_gpio(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Gpio *gpio = NULL;
    gpio_dev_info_t info;
    gpio_line_config_t cfg;
    int val = -1;

    TRY {
        dbg_str(DBG_INFO, "test_gpio");

        gpio = object_new(allocator, "Gpio", NULL);
        THROW_IF(gpio == NULL, -1);

        /* 1. 打开 gpiochip0（无 GPIO 控制器则跳过） */
        if (gpio->open(gpio, "/dev/gpiochip0") < 0) {
            dbg_str(DBG_INFO, "no /dev/gpiochip0 (QEMU 需 pl061 + "
                    "CONFIG_GPIO_CDEV)，跳过 GPIO 测试");
            ret = 1;
        } else {
            EXEC(gpio->get_info(gpio, &info));
            dbg_str(DBG_INFO, "gpiochip0: name:%s, label:%s, lines:%u",
                    info.name, info.label, info.num_lines);

            /* 2. 请求 line0 为输出并做写读往返。
             * 注意：request_lines 成功时经 CATCH 宏返回 1（非 0），
             * 因此用 >= 0 判断成功，不能用 == 0。 */
            memset(&cfg, 0, sizeof(cfg));
            cfg.offset = 0;
            cfg.dir = GPIO_OUT;
            cfg.init_val = GPIO_HIGH;

            if (gpio->request_lines(gpio, &cfg, 1) == 1) {
                EXEC(gpio->set_value(gpio, 0, GPIO_LOW));
                EXEC(gpio->get_value(gpio, 0, &val));
                dbg_str(DBG_INFO, "gpio line0 set 0, read %d", val);
                THROW_IF(val != GPIO_LOW, -1);

                EXEC(gpio->set_value(gpio, 0, GPIO_HIGH));
                EXEC(gpio->get_value(gpio, 0, &val));
                dbg_str(DBG_INFO, "gpio line0 set 1, read %d", val);
                THROW_IF(val != GPIO_HIGH, -1);

                dbg_str(DBG_INFO, "gpio line0 write/read round-trip ok");
            } else {
                dbg_str(DBG_INFO, "gpio line0 request failed (line busy?)，"
                        "跳过写读往返");
            }
            ret = 1;
        }
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(gpio);
    }

    return ret;
}
REGISTER_TEST_CMD(test_gpio);

/* 事件计数器（handler 中累加） */
static int test_gpio_event_count = 0;

/*
 * GPIO 边沿事件异步回调（io_worker）。
 * opaque 为 io_worker 传入的 worker，通过 ((Worker *)opaque)->opaque 拿到 Gpio
 * 对象，读取 gpio->event（最新事件的 id/timestamp）。
 */
static int test_gpio_event_handler(void *opaque)
{
    Worker *worker = (Worker *)opaque;
    Gpio *gpio = (Gpio *)worker->opaque;

    dbg_str(DBG_INFO, "gpio event handler called, id:%d, ts:%llu",
            gpio->event.id, (unsigned long long)gpio->event.timestamp);
    test_gpio_event_count++;
    return 0;
}

/*
 * 测试 Gpio 边沿事件（依赖 QEMU virt.c 里 pl061 line0→line1 的回环接线）：
 *   - 单对象：request_lines(line0=输出，触发源) + register_event(line1) 共存
 *     （handle 与 event 是对象里两套独立 fd，不同 line 可同时持有）；
 *   - 翻转 line0 → 回环驱动 line1 产生边沿 → handler 被异步调用，读 gpio->event。
 * 若无 /dev/gpiochip0 或回环接线缺失（无事件触发），打印后跳过/失败。
 */
static int test_gpio_event(TEST_ENTRY *entry)
{
    int ret = 1, i;
    allocator_t *allocator = allocator_get_default_instance();
    Gpio *gpio = NULL;
    gpio_line_config_t cfg;

    test_gpio_event_count = 0;

    TRY {
        dbg_str(DBG_INFO, "test_gpio_event");

        gpio = object_new(allocator, "Gpio", NULL);
        THROW_IF(gpio == NULL, -1);

        if (gpio->open(gpio, "/dev/gpiochip0") < 0) {
            dbg_str(DBG_INFO, "no /dev/gpiochip0, skip gpio event test");
            ret = 1;
        } else {
            /* 1. 同一对象：请求 line0 为输出（触发源），初值低 */
            memset(&cfg, 0, sizeof(cfg));
            cfg.offset = 0;
            cfg.dir = GPIO_OUT;
            cfg.init_val = GPIO_LOW;
            THROW_IF(gpio->request_lines(gpio, &cfg, 1) < 0, -1);

            /* 2. 同一对象：对 line1 注册双边沿事件（事件请求本身占用 line1，
             *    一条 line 不能同时作 handle 和 event，但 handle(line0)+event(line1)
             *    是不同的 line，可共存） */
            THROW_IF(gpio->register_event(gpio, 1, GPIO_EDGE_BOTH,
                                          test_gpio_event_handler, gpio) < 0, -1);

            /* 3. 翻转 line0 → 回环驱动 line1 产生 上升/下降/上升 三个边沿 */
            THROW_IF(gpio->set_value(gpio, 0, GPIO_HIGH) < 0, -1);
            THROW_IF(gpio->set_value(gpio, 0, GPIO_LOW) < 0, -1);
            THROW_IF(gpio->set_value(gpio, 0, GPIO_HIGH) < 0, -1);

            /* 4. 轮询等待异步 handler 被调用（最多约 2s） */
            for (i = 0; i < 200 && test_gpio_event_count == 0; i++) {
                usleep(10000);
            }
            dbg_str(DBG_INFO, "gpio event count:%d, last id:%d",
                    test_gpio_event_count, gpio->event.id);
            THROW_IF(test_gpio_event_count == 0, -1);
            THROW_IF(gpio->event.id != GPIO_EDGE_RISING &&
                     gpio->event.id != GPIO_EDGE_FALLING, -1);

            dbg_str(DBG_INFO, "gpio event trigger/verify ok, count:%d, id:%d",
                    test_gpio_event_count, gpio->event.id);
            ret = 1;
        }
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(gpio);
    }

    return ret;
}
REGISTER_TEST_CMD(test_gpio_event);
