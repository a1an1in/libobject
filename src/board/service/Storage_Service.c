/**
 * @file Storage_Service.c
 * @Synopsis  存储服务（服务层）。
 * 基于 HAL 层设备类（I2c_EEPROM）提供设备无关的存储接口。
 * 用户只调用 save()/load()，不关心底层是 EEPROM、Flash 还是其他设备。
 * @author alan lin
 * @version
 * @date 2026-08-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/board/service/Storage_Service.h>
#include <libobject/core/utils/dbg/debug.h>

/*
 * 保存：addr 为存储地址，data 为要保存的数据，len 为字节数。
 * 屏蔽设备细节，内部调用底层 I2c_EEPROM 的 write。
 */
static int __save(Storage_Service *svc, uint16_t addr, const void *data, size_t len)
{
    int ret = -1;

    TRY {
        THROW_IF(svc == NULL || data == NULL, -STORAGE_SERVICE_ERR_INVALID_ARG);
        THROW_IF(svc->eeprom == NULL, -STORAGE_SERVICE_ERR_DEVICE);
        THROW_IF(len == 0, -STORAGE_SERVICE_ERR_INVALID_ARG);
        /* 地址越界检查 */
        THROW_IF(addr + len > svc->size, -STORAGE_SERVICE_ERR_OUT_OF_RANGE);

        ret = svc->eeprom->write(svc->eeprom, addr, data, len);
        THROW_IF(ret < 0, -STORAGE_SERVICE_ERR_DEVICE);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "storage save failed, addr:0x%x, len:%d",
                addr, (int)len);
    }

    return ret;
}

/*
 * 读取：addr 为存储地址，data 为读缓冲区，len 为字节数。
 * 屏蔽设备细节，内部调用底层 I2c_EEPROM 的 read。
 */
static int __load(Storage_Service *svc, uint16_t addr, void *data, size_t len)
{
    int ret = -1;

    TRY {
        THROW_IF(svc == NULL || data == NULL, -STORAGE_SERVICE_ERR_INVALID_ARG);
        THROW_IF(svc->eeprom == NULL, -STORAGE_SERVICE_ERR_DEVICE);
        THROW_IF(len == 0, -STORAGE_SERVICE_ERR_INVALID_ARG);
        /* 地址越界检查 */
        THROW_IF(addr + len > svc->size, -STORAGE_SERVICE_ERR_OUT_OF_RANGE);

        ret = svc->eeprom->read(svc->eeprom, addr, data, len);
        THROW_IF(ret < 0, -STORAGE_SERVICE_ERR_DEVICE);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "storage load failed, addr:0x%x, len:%d",
                addr, (int)len);
    }

    return ret;
}

/*
 * 初始化：创建并初始化底层 I2c_EEPROM 设备。
 * bus_number 为 I2C 总线号，slave_addr 为从机地址，size 为容量（字节）。
 */
static int __init(Storage_Service *svc, int bus_number, int slave_addr, uint32_t size)
{
    allocator_t *allocator;
    int ret = -1;

    TRY {
        THROW_IF(svc == NULL, -STORAGE_SERVICE_ERR_INVALID_ARG);
        THROW_IF(size == 0, -STORAGE_SERVICE_ERR_INVALID_ARG);

        /* 1. 创建底层 I2c_EEPROM 设备对象 */
        allocator = svc->parent.allocator;
        svc->eeprom = object_new(allocator, "I2c_EEPROM", NULL);
        THROW_IF(svc->eeprom == NULL, -STORAGE_SERVICE_ERR_DEVICE);

        /* 2. 初始化 EEPROM（打开总线 + 配置从机地址 + 容量） */
        ret = svc->eeprom->init(svc->eeprom, bus_number, slave_addr, size);
        THROW_IF(ret < 0, -STORAGE_SERVICE_ERR_DEVICE);

        svc->size = size;

        dbg_str(DBG_INFO, "storage service init ok, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "storage service init failed, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
        if (svc->eeprom != NULL) {
            object_destroy(svc->eeprom);
            svc->eeprom = NULL;
        }
    }

    return ret;
}

static int __construct(Storage_Service *module, char *init_str)
{
    module->eeprom = NULL;
    module->size = 0;
    return 0;
}

static int __deconstruct(Storage_Service *module)
{
    /* 销毁底层 I2c_EEPROM 设备对象（close 在 deconstruct 中自动调用） */
    if (module->eeprom != NULL) {
        object_destroy(module->eeprom);
        module->eeprom = NULL;
    }
    return 0;
}

/*
 * Storage_Service 注册具体类。
 * 服务层只依赖 HAL 层设备类（I2c_EEPROM），向上提供设备无关的 save/load 接口。
 */
DEFINE_CLASS(
    EXTENDS(Storage_Service, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_NFunc_Entry(init, __init),
    Class_NFunc_Entry(save, __save),
    Class_NFunc_Entry(load, __load)
);
