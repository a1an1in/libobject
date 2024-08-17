#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // 包含uint32_t类型的定义
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/data_structure/bitmap.h>
#include <libobject/core/utils/dbg/debug.h>

bitmap_t *bitmap_alloc(allocator_t *allocator) 
{
    bitmap_t *bitmap;

    bitmap = (bitmap_t *)allocator_mem_zalloc(allocator, sizeof(bitmap_t));
    if (bitmap == NULL) { // 检查内存分配是否成功
        perror("Failed to allocate memory for bitmap");
        return NULL;
    }
    bitmap->allocator = allocator;

    return bitmap; // 返回初始化后的位图结构
}

// 初始化位图
int bitmap_init(bitmap_t *bitmap, uint32_t size) 
{
    uint32_t array_size = size / 32 + 1; // 计算所需的数组大小，每个uint32_t可以存储32位

    bitmap->bits = (uint32_t *)allocator_mem_zalloc(bitmap->allocator, array_size * sizeof(int)); // 分配位图数据存储的内存空间
    if (bitmap->bits == NULL) { // 检查内存分配是否成功
        perror("Failed to allocate memory for bitmap bits");
        return -1;
    }

    bitmap->size = size; // 设置位图的大小

    return 1; // 返回初始化后的位图结构
}

// 获取位图中某一位的值
int bitmap_get(bitmap_t *bitmap, uint32_t pos) 
{
    uint32_t index, bit;

    if (bitmap == NULL || pos >= bitmap->size) { // 检查位图结构是否有效，以及位置是否超出范围
        return -1; // 如果无效，则返回0
    }
    
    index = pos / 32; // 计算位于数组中的索引
    bit = pos % 32; // 计算在uint32_t中的位偏移量
    // dbg_str(DBG_VIP, "bitmap get, pos:%d, index:%d, bit:%d", pos, index, bit);
    
    return (bitmap->bits[index] & (1U << bit)) != 0; // 返回指定位置的位的值（0或1）
}

// 设置位图中某一位的值（将其置为1）
int bitmap_set(bitmap_t *bitmap, uint32_t pos) 
{
    uint32_t index, bit, ret;

    if (bitmap == NULL || pos >= bitmap->size) { // 检查位图结构是否有效，以及位置是否超出范围
        dbg_str(DBG_ERROR, "bitmap:%p, pos:%d", bitmap, pos);
        return -1; // 如果无效，则直接返回
    }
    
    index = pos / 32; // 计算位于数组中的索引
    bit = pos % 32; // 计算在uint32_t中的位偏移量
    
    bitmap->bits[index] |= (1U << bit); // 将指定位置的位设置为1

    ret = bitmap_get(bitmap, pos);
 
    return ret == 1 ? 1 : 0;
}
 
// 清除位图中某一位的值（将其置为0）
int bitmap_clear(bitmap_t *bitmap, uint32_t pos) 
{
    uint32_t index, bit;

    dbg_str(DBG_FATAL, "xxxxxx run at here");
    if (bitmap == NULL || pos >= bitmap->size) { // 检查位图结构是否有效，以及位置是否超出范围
        return -1; // 如果无效，则直接返回
    }
    
    index = pos / 32; // 计算位于数组中的索引
    bit = pos % 32; // 计算在uint32_t中的位偏移量
    
    bitmap->bits[index] &= ~(1U << bit); // 将指定位置的位清除为0

    return 1;
}

// 清除位图中某一位的值（将其置为0）
int bitmap_reset(bitmap_t *bitmap) 
{
    uint32_t index, bit;

    if (bitmap == NULL) { // 检查位图结构是否有效，以及位置是否超出范围
        return -1; // 如果无效，则直接返回
    }
    
    // memset(bitmap->bits, 0, bitmap->size);

    return 1;
}

// 打印输出位图的内容
int bitmap_print(bitmap_t *bitmap) 
{
    int i = 0;

    if (bitmap == NULL) { // 检查位图结构是否有效
        printf("bitmap_t is NULL.\n");
        return -1;
    }
    
    printf("bit_map contents: addr:%p\n", bitmap->bits);
    for (i = 0; i < bitmap->size; ++i) { // 遍历位图的每一位
        printf("%d ", bitmap_get(bitmap, i)); // 获取并输出当前位的值
        
        // 每输出32位换行，以便于查看
        if ((i + 1) % 32 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}
 
// 释放位图内存
int bitmap_destroy(bitmap_t *bitmap) 
{
    if (bitmap == NULL) { // 检查位图结构是否有效
        return -1;
    }
    allocator_mem_free(bitmap->allocator, bitmap->bits); // 释放位图数据存储的内存空间
    allocator_mem_free(bitmap->allocator, bitmap); // 释放位图结构的内存空间

    return 1;
}