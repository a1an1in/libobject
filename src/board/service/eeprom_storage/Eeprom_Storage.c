/**
 * @file Eeprom_Storage.c
 * @Synopsis  基于 EEPROM 的 key-value 存储服务（服务层）。
 * 底层 EEPROM 中存储 JSON 数据（用 cjson 序列化/反序列化）。
 * 给用户的接口是 key-value 结构：set(key, value) / get(key)。
 * 组件保留 JSON 缓存：set/get 直接读写缓存（内存），
 * load/save 才读写 ROM（EEPROM）。
 * @author alan lin
 * @version
 * @date 2026-08-08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <libobject/board/service/eeprom_storage/Eeprom_Storage.h>
#include <libobject/core/utils/dbg/debug.h>

/* JSON 数据在 EEPROM 中的起始地址 */
#define EEPROM_STORAGE_DATA_ADDR  0x00

/*
 * 设置 key-value：写入 JSON 缓存（不立即写 ROM）。
 * 若 key 已存在则替换其值，否则新增。
 */
static int __set_value(Eeprom_Storage *es, const char *key, const char *value)
{
    cjson_t *item;
    int ret = -1;

    TRY {
        THROW_IF(es == NULL || key == NULL || value == NULL,
                 -EEPROM_STORAGE_ERR_INVALID_ARG);

        /* 若缓存未加载，先创建空的 JSON 对象 */
        if (es->cache == NULL) {
            es->cache = cjson_create_object();
            THROW_IF(es->cache == NULL, -EEPROM_STORAGE_ERR_JSON);
        }

        /* 若 key 已存在，替换其值；否则新增 */
        item = cjson_get_object_item(es->cache, key);
        if (item != NULL) {
            /* 替换：删除旧项，再新增 */
            cjson_delete_item_from_object(es->cache, key);
        }
        cjson_add_string_to_object(es->cache, key, value);

        /* 缓存已有数据，标记为有效，避免 get_value 误触发 load 覆盖缓存 */
        es->cache_valid = true;

        dbg_str(DBG_INFO, "eeprom_storage set_value ok, key:%s, value:%s",
                key, value);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom_storage set_value failed, key:%s", key);
    }

    return ret;
}

/*
 * 获取 key-value：从 JSON 缓存读取（若缓存未加载先 load）。
 * 返回值为 malloc 的字符串，调用者需 free；key 不存在返回 NULL。
 */
static char *__get_value(Eeprom_Storage *es, const char *key)
{
    cjson_t *item;
    char *result = NULL;
    int ret = -1;

    TRY {
        THROW_IF(es == NULL || key == NULL, -EEPROM_STORAGE_ERR_INVALID_ARG);

        /* 若缓存未加载，先从 ROM 加载 */
        if (!es->cache_valid) {
            ret = es->load(es);
            THROW_IF(ret < 0, ret);
        }

        /* 从缓存中查找 key */
        if (es->cache == NULL) {
            THROW_IF(1, -EEPROM_STORAGE_ERR_NOT_FOUND);
        }
        item = cjson_get_object_item(es->cache, key);
        THROW_IF(item == NULL, -EEPROM_STORAGE_ERR_NOT_FOUND);
        THROW_IF(item->valuestring == NULL, -EEPROM_STORAGE_ERR_NOT_FOUND);

        /* 返回字符串副本（调用者 free） */
        result = strdup(item->valuestring);
        THROW_IF(result == NULL, -EEPROM_STORAGE_ERR_ERROR);

        dbg_str(DBG_INFO, "eeprom_storage get_value ok, key:%s, value:%s",
                key, result);
        ret = 0;
    } CATCH (ret) {
        if (ret == -EEPROM_STORAGE_ERR_NOT_FOUND) {
            dbg_str(DBG_WARN, "eeprom_storage get_value key not found, key:%s", key);
        } else {
            dbg_str(DBG_ERROR, "eeprom_storage get_value failed, key:%s", key);
        }
        if (result != NULL) {
            free(result);
            result = NULL;
        }
    }

    return result;
}

/*
 * 从 EEPROM 读取 JSON 到缓存。
 * 读取整个 EEPROM 数据区，解析为 JSON 对象存入缓存。
 */
static int __load(Eeprom_Storage *es)
{
    char *buf = NULL;
    cjson_t *cache = NULL;
    int ret = -1;

    TRY {
        THROW_IF(es == NULL, -EEPROM_STORAGE_ERR_INVALID_ARG);
        THROW_IF(es->eeprom == NULL, -EEPROM_STORAGE_ERR_DEVICE);

        /* 1. 分配缓冲区读取 EEPROM 数据区 */
        buf = (char *)calloc(1, es->size + 1);
        THROW_IF(buf == NULL, -EEPROM_STORAGE_ERR_ERROR);

        /* 2. 从 EEPROM 读取数据 */
        ret = es->eeprom->read(es->eeprom, EEPROM_STORAGE_DATA_ADDR,
                               buf, es->size);
        THROW_IF(ret < 0, -EEPROM_STORAGE_ERR_DEVICE);

        /* 3. 解析 JSON（若数据为空或非法，创建空对象） */
        if (buf[0] == '\0') {
            cache = cjson_create_object();
            THROW_IF(cache == NULL, -EEPROM_STORAGE_ERR_JSON);
        } else {
            cache = cjson_parse(buf);
            if (cache == NULL) {
                /* JSON 解析失败，创建空对象（容错） */
                dbg_str(DBG_WARN, "eeprom_storage load json parse failed, "
                        "create empty cache");
                cache = cjson_create_object();
                THROW_IF(cache == NULL, -EEPROM_STORAGE_ERR_JSON);
            }
        }

        /* 4. 替换缓存 */
        if (es->cache != NULL) {
            cjson_delete(es->cache);
        }
        es->cache = cache;
        es->cache_valid = true;

        dbg_str(DBG_INFO, "eeprom_storage load ok, size:%d", es->size);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom_storage load failed");
        if (cache != NULL) {
            cjson_delete(cache);
        }
    }

    if (buf != NULL) {
        free(buf);
    }

    return ret;
}

/*
 * 把缓存中的 JSON 写回 EEPROM。
 * 将缓存序列化为 JSON 字符串，写入 EEPROM 数据区。
 */
static int __save(Eeprom_Storage *es)
{
    char *json_str = NULL;
    size_t len;
    int ret = -1;

    TRY {
        THROW_IF(es == NULL, -EEPROM_STORAGE_ERR_INVALID_ARG);
        THROW_IF(es->eeprom == NULL, -EEPROM_STORAGE_ERR_DEVICE);

        /* 若缓存为空，创建空对象 */
        if (es->cache == NULL) {
            es->cache = cjson_create_object();
            THROW_IF(es->cache == NULL, -EEPROM_STORAGE_ERR_JSON);
        }

        /* 1. 序列化缓存为 JSON 字符串 */
        json_str = cjson_print_unformatted(es->cache);
        THROW_IF(json_str == NULL, -EEPROM_STORAGE_ERR_JSON);

        len = strlen(json_str);
        /* 2. 判断 JSON 数据是否超出 EEPROM 实际容量（用底层设备容量判断） */
        THROW_IF(len > es->eeprom->size, -EEPROM_STORAGE_ERR_OUT_OF_RANGE);

        /* 3. 写入 EEPROM */
        ret = es->eeprom->write(es->eeprom, EEPROM_STORAGE_DATA_ADDR,
                                json_str, len);
        THROW_IF(ret < 0, -EEPROM_STORAGE_ERR_DEVICE);

        es->cache_valid = true;

        dbg_str(DBG_INFO, "eeprom_storage save ok, len:%d, json:%s",
                (int)len, json_str);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom_storage save failed");
    }

    if (json_str != NULL) {
        free(json_str);
    }

    return ret;
}

/*
 * 初始化：创建并初始化底层 I2c_EEPROM 设备。
 * bus_number 为 I2C 总线号，slave_addr 为从机地址，size 为容量（字节）。
 */
static int __init(Eeprom_Storage *es, int bus_number, int slave_addr, uint32_t size)
{
    allocator_t *allocator;
    int ret = -1;

    TRY {
        THROW_IF(es == NULL, -EEPROM_STORAGE_ERR_INVALID_ARG);
        THROW_IF(size == 0, -EEPROM_STORAGE_ERR_INVALID_ARG);

        /* 1. 创建底层 I2c_EEPROM 设备对象 */
        allocator = es->parent.allocator;
        es->eeprom = object_new(allocator, "I2c_EEPROM", NULL);
        THROW_IF(es->eeprom == NULL, -EEPROM_STORAGE_ERR_DEVICE);

        /* 2. 初始化 EEPROM（打开总线 + 配置从机地址 + 容量） */
        ret = es->eeprom->init(es->eeprom, bus_number, slave_addr, size);
        THROW_IF(ret < 0, -EEPROM_STORAGE_ERR_DEVICE);

        es->size = size;
        es->cache_valid = false;

        dbg_str(DBG_INFO, "eeprom_storage init ok, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
        ret = 0;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "eeprom_storage init failed, bus:%d, slave_addr:0x%x, size:%d",
                bus_number, slave_addr, size);
        if (es->eeprom != NULL) {
            object_destroy(es->eeprom);
            es->eeprom = NULL;
        }
    }

    return ret;
}

static int __construct(Eeprom_Storage *module, char *init_str)
{
    module->eeprom = NULL;
    module->cache = NULL;
    module->size = 0;
    module->cache_valid = false;
    return 0;
}

static int __deconstruct(Eeprom_Storage *module)
{
    /* 销毁 JSON 缓存 */
    if (module->cache != NULL) {
        cjson_delete(module->cache);
        module->cache = NULL;
    }
    /* 销毁底层 I2c_EEPROM 设备对象（close 在 deconstruct 中自动调用） */
    if (module->eeprom != NULL) {
        object_destroy(module->eeprom);
        module->eeprom = NULL;
    }
    return 0;
}

/*
 * Eeprom_Storage 注册具体类。
 * 服务层只依赖 HAL 层设备类（I2c_EEPROM）和 core 的 cjson，
 * 向上提供 key-value 接口（set_value/get_value）和 ROM 读写接口（load/save）。
 */
DEFINE_CLASS(
    EXTENDS(Eeprom_Storage, Obj),
    Class_NFunc_Entry(construct, __construct),
    Class_NFunc_Entry(deconstruct, __deconstruct),
    Class_NFunc_Entry(init, __init),
    Class_NFunc_Entry(set_value, __set_value),
    Class_NFunc_Entry(get_value, __get_value),
    Class_NFunc_Entry(load, __load),
    Class_NFunc_Entry(save, __save)
);
