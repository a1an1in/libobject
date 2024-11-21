#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // 包含uint32_t类型的定义
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>
#include <libobject/mockery/mockery.h>

static int test_enum(TEST_ENTRY *entry, void *argc, void *argv)
{

    printf("BUS_RET_FAIL: %d\n", BUS_RET_FAIL);
    return 0;
}
REGISTER_TEST_CMD(test_enum);