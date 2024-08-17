#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // 包含uint32_t类型的定义
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/bitmap.h>
#include <libobject/mockery/mockery.h>

static int test_bitmap() 
{
    allocator_t *allocator = allocator_get_default_instance();
    bitmap_t *bitmap;
    int ret;

    TRY {
        bitmap = bitmap_alloc(allocator);
        THROW_IF(bitmap == NULL, -1);
        EXEC(bitmap_init(bitmap, BITMAP_DEFAULT_SIZE)); // 初始化一个位图
        
        // 设置一些位
        EXEC(bitmap_set(bitmap, 5));
        EXEC(bitmap_set(bitmap, 10));
        bitmap_set(bitmap, 100);

        ret = bitmap_get(bitmap, 5);
        THROW_IF(ret != 1, -1);
        ret = bitmap_get(bitmap, 10);
        THROW_IF(ret != 1, -1);
        ret = bitmap_get(bitmap, 100);
        THROW_IF(ret != -1, -1);
        
        // 打印输出位图内容
        // bitmap_print(bitmap);
    } CATCH (ret) { } FINALLY {
        bitmap_destroy(bitmap); // 释放位图内存
    }
    
    return 1;
}
REGISTER_TEST_FUNC(test_bitmap);