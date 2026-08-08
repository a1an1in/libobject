#ifndef __STORAGE_SERVICE_H__
#define __STORAGE_SERVICE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/core/Obj.h>
#include <libobject/board/hal/i2c/I2c_EEPROM.h>

/*
 * Storage_Service 类：存储服务（服务层）。
 *
 * 基于 HAL 层设备类（I2c_EEPROM）提供设备无关的存储接口。
 * 用户只调用 save()/load()，不关心底层是 EEPROM、Flash 还是其他设备。
 *
 * 设计思想（服务层 vs HAL 层）：
 *   - HAL 层（I2c_EEPROM）：设备相关，封装具体芯片的寄存器协议（read/write 字节）
 *   - 服务层（Storage_Service）：设备无关，屏蔽设备细节，提供业务功能（save/load）
 *
 * 职责：
 *   - 内部持有 I2c_EEPROM 设备实例，负责其生命周期（construct/init/deconstruct）
 *   - 提供 save()/load() 接口，用户不关心底层设备
 *   - 可跨项目复用（存储服务不依赖具体产品业务）
 *
 * 继承关系：Obj -> Storage_Service
 */

typedef struct Storage_Service_s Storage_Service;

/* 存储服务返回值/错误码 */
typedef enum storage_service_return_value {
    STORAGE_SERVICE_ERR_OK          = 0x0,   /* 成功 */
    STORAGE_SERVICE_ERR_ERROR       = 0x2,   /* 通用错误 */
    STORAGE_SERVICE_ERR_INVALID_ARG = 0x3,   /* 非法参数 */
    STORAGE_SERVICE_ERR_OUT_OF_RANGE= 0x4,   /* 地址越界 */
    STORAGE_SERVICE_ERR_DEVICE      = 0x5,   /* 底层设备错误 */
} storage_service_return_value_t;

struct Storage_Service_s {
    Obj parent;

    int (*construct)(Storage_Service *, char *);
    int (*deconstruct)(Storage_Service *);

    /*virtual methods reimplement*/
    int (*set)(Storage_Service *module, char *attrib, void *value);
    void *(*get)(Storage_Service *, char *attrib);
    char *(*to_json)(Storage_Service *);

    /* 存储服务初始化接口：打开底层 EEPROM 设备 */
    /* bus_number 为 I2C 总线号，slave_addr 为从机地址，size 为容量（字节） */
    int (*init)(Storage_Service *svc, int bus_number, int slave_addr, uint32_t size);

    /* 存储服务接口（设备无关） */
    /* 保存：addr 为存储地址，data 为要保存的数据，len 为字节数 */
    int (*save)(Storage_Service *svc, uint16_t addr, const void *data, size_t len);
    /* 读取：addr 为存储地址，data 为读缓冲区，len 为字节数 */
    int (*load)(Storage_Service *svc, uint16_t addr, void *data, size_t len);

    /*attribs*/
    I2c_EEPROM *eeprom;   /* 内部依赖 HAL 层设备类（I2c_EEPROM） */
    uint32_t size;        /* 存储容量（字节） */
};

#endif
