#ifndef __BIT_MAP_H__
#define __BIT_MAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // 包含uint32_t类型的定义
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/dbg/debug.h>
 
#define BITMAP_DEFAULT_SIZE 100 // 位图大小，假设为100位
 
// 定义位图结构
typedef struct {
    allocator_t *allocator;
    uint32_t *bits; // 使用32位整数作为位图的单元
    uint32_t size;  // 位图的大小（以位为单位）
} bit_map_t;

bit_map_t *bitmap_alloc(allocator_t *allocator);
int bitmap_init(bit_map_t *bitmap, uint32_t size); 
int bitmap_set(bit_map_t *bitmap, uint32_t pos);
int bitmap_clear(bit_map_t *bitmap, uint32_t pos); 
int bitmap_get(bit_map_t *bitmap, uint32_t pos);
int bitmap_print(bit_map_t *bitmap);
int bitmap_destroy(bit_map_t *bitmap); 


#endif