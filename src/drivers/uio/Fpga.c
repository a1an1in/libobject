/**
 * @file Fpga.c
 * @Synopsis  FPGA 专用驱动，继承通用 Uio 类。
 * 在通用 Uio 之上封装 FPGA 特有的接口（默认设备名 "fpga"）。
 * 寄存器读写、中断等接口继承 Uio 的实现。
 * @author alan lin
 * @version 
 * @date 2026-08-05
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libobject/drivers/uio/Fpga.h>
#include <libobject/core/utils/dbg/debug.h>

#define FPGA_DEFAULT_DEVICE_NAME "fpga"

/*
 * 打开 FPGA UIO 设备并 mmap 映射寄存器空间。
 * name 为空时使用默认设备名 "fpga"。
 * 复用 Uio 的 open 和 mmap。
 */
static int __open_device(Fpga *fpga, char *name)
{
    Uio *uio = (Uio *)fpga;
    char *dev_name = name;
    int ret = -1;

    TRY {
        if (dev_name == NULL || dev_name[0] == '\0') {
            dev_name = FPGA_DEFAULT_DEVICE_NAME;
        }

        /* 1. 打开 UIO 设备 */
        ret = uio->open(uio, dev_name);
        THROW_IF(ret < 0, -1);

        /* 2. mmap 映射 FPGA 寄存器空间 */
        ret = uio->mmap(uio);
        THROW_IF(ret < 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "fpga open_device failed, name:%s", dev_name);
    }

    return ret;
}

static int __construct(Fpga *module, char *init_str)
{
    module->device_name = NULL;
    return 0;
}

static int __deconstruct(Fpga *module)
{
    Uio *uio = (Uio *)module;

    /* 关闭 FPGA UIO 设备（close 在 deconstruct 中自动调用） */
    if (uio->fd >= 0 || uio->base != NULL) {
        uio->close(uio);
    }

    if (module->device_name != NULL) {
        free(module->device_name);
        module->device_name = NULL;
    }
    return 0;
}

/*
 * Fpga 注册 open_device（默认设备名 "fpga"）。
 * read_register/write_register/read_registers/write_registers/
 * enable_irq/register_irq/disable_irq 等接口 value 为 NULL，继承 Uio 的实现。
 */
DEFINE_CLASS(Fpga,
    Class_Obj___Entry(Uio, parent),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open_device, __open_device),
    Class_VFunc_Entry(get_size, NULL),
    Class_VFunc_Entry(set_width, NULL),
    Class_VFunc_Entry(read_register, NULL),
    Class_VFunc_Entry(write_register, NULL),
    Class_VFunc_Entry(read_registers, NULL),
    Class_VFunc_Entry(write_registers, NULL),
    Class_VFunc_Entry(enable_irq, NULL),
    Class_VFunc_Entry(disable_irq, NULL),
    Class_VFunc_Entry(register_irq, NULL)
);
