#ifndef __EEPROM_STORAGE_H__
#define __EEPROM_STORAGE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/json/cjson.h>
#include <libobject/board/hal/i2c/I2c_EEPROM.h>

/*
 * Eeprom_Storage 类：基于 EEPROM 的 key-value 存储服务（服务层）。
 *
 * 设计思想：
 *   - 底层 EEPROM 中存储 JSON 数据（用 cjson 序列化/反序列化）
 *   - 给用户的接口是 key-value 结构：set(key, value) / get(key)
 *   - 组件保留 JSON 缓存：set/get 直接读写缓存（内存），
 *     load/save 才读写 ROM（EEPROM）
 *
 * 缓存机制：
 *   - set(key, value)：写入 JSON 缓存（不立即写 ROM）
 *   - get(key)：从 JSON 缓存读取（若缓存未加载，先 load）
 *   - load()：从 EEPROM 读取 JSON 到缓存
 *   - save()：把缓存中的 JSON 写回 EEPROM
 *
 * 继承关系：Obj -> Eeprom_Storage
 */

typedef struct Eeprom_Storage_s Eeprom_Storage;

/* 存储服务返回值/错误码 */
typedef enum eeprom_storage_return_value {
    EEPROM_STORAGE_ERR_OK          = 0x0,   /* 成功 */
    EEPROM_STORAGE_ERR_ERROR       = 0x2,   /* 通用错误 */
    EEPROM_STORAGE_ERR_INVALID_ARG = 0x3,   /* 非法参数 */
    EEPROM_STORAGE_ERR_OUT_OF_RANGE= 0x4,   /* 地址越界 */
    EEPROM_STORAGE_ERR_DEVICE      = 0x5,   /* 底层设备错误 */
    EEPROM_STORAGE_ERR_JSON        = 0x6,   /* JSON 解析/序列化错误 */
    EEPROM_STORAGE_ERR_NOT_FOUND   = 0x7,   /* key 不存在 */
} eeprom_storage_return_value_t;

struct Eeprom_Storage_s {
    Obj parent;

    int (*construct)(Eeprom_Storage *, char *);
    int (*deconstruct)(Eeprom_Storage *);

    /*virtual methods reimplement*/
    int (*set)(Eeprom_Storage *module, char *attrib, void *value);
    void *(*get)(Eeprom_Storage *, char *attrib);
    char *(*to_json)(Eeprom_Storage *);

    /* 初始化接口：打开底层 EEPROM 设备 */
    /* bus_number 为 I2C 总线号，slave_addr 为从机地址，size 为容量（字节） */
    int (*init)(Eeprom_Storage *es, int bus_number, int slave_addr, uint32_t size);

    /* key-value 接口（操作 JSON 缓存） */
    /* 设置：key 为键，value 为字符串值（写入缓存，不立即写 ROM） */
    int (*set_value)(Eeprom_Storage *es, const char *key, const char *value);
    /* 获取：key 为键，返回字符串值（从缓存读取，若缓存未加载先 load）。
     * 返回值为 malloc 的字符串，调用者需 free；key 不存在返回 NULL。 */
    char *(*get_value)(Eeprom_Storage *es, const char *key);

    /* ROM 读写接口 */
    /* 从 EEPROM 读取 JSON 到缓存 */
    int (*load)(Eeprom_Storage *es);
    /* 把缓存中的 JSON 写回 EEPROM */
    int (*save)(Eeprom_Storage *es);

    /*attribs*/
    I2c_EEPROM *eeprom;   /* 内部依赖 HAL 层设备类（I2c_EEPROM） */
    cjson_t *cache;       /* JSON 缓存（cjson 对象） */
    uint32_t size;        /* EEPROM 容量（字节） */
    bool cache_valid;     /* 缓存是否已从 ROM 加载 */
};

#endif
