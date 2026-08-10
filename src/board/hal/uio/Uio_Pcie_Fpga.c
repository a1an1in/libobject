/**
 * @file Uio_Pcie_Fpga.c
 * @Synopsis  PCIe 板卡上的 FPGA 专用驱动，继承 Uio_Pcie。
 * 不新增接口，直接复用 Uio_Pcie 的全部能力，作为 PCIe FPGA 的基类。
 * @author alan lin
 * @version
 * @date 2026-08-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libobject/board/hal/uio/Uio_Pcie_Fpga.h>
#include <libobject/core/utils/dbg/debug.h>

static int __construct(Uio_Pcie_Fpga *module, char *init_str)
{
    return 0;
}

static int __deconstruct(Uio_Pcie_Fpga *module)
{
    return 0;
}

/*
 * Uio_Pcie_Fpga 不新增接口，全部继承 Uio_Pcie
 * （open_device/get_info/read_config/write_config/bind_uio/get_size/set_width/
 * read-write_register(s)/enable-disable_irq/register_irq value 均为 NULL）。
 */
DEFINE_CLASS(
    EXTENDS(Uio_Pcie_Fpga, Uio_Pcie),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_VFunc_Entry(open_device, NULL),
    Class_VFunc_Entry(get_info, NULL),
    Class_VFunc_Entry(read_config, NULL),
    Class_VFunc_Entry(write_config, NULL),
    Class_VFunc_Entry(bind_uio, NULL),
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
