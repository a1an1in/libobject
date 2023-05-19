#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libobject/core/utils/bn/big_number.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

static int
test_bn_cmp(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};

    TRY {
        EXEC(bn_cmp(num1, sizeof(num1), num2, sizeof(num2), &ret));
        THROW_IF(ret >= 0, 0);
        EXEC(bn_cmp(num2, sizeof(num2), num1, sizeof(num1), &ret));
        THROW_IF(ret < 0, 0);
        EXEC(bn_cmp(num1, sizeof(num1), num1, sizeof(num1), &ret));
        THROW_IF(ret != 0, 0);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_bn_cmp, ret=%d", ret);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_bn_cmp);

static int
test_mul_u32(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    uint8_t num[8] = {0x11, 0x22, 0x33, 0xff};
    uint8_t expect[16] = {0xc6, 0x92, 0x5f, 0xae, 0x65};
    uint8_t result[16] = {0};
    uint32_t word = 0x66;
    uint32_t *test;

    TRY {
        test = num;
        dbg_str(DBG_DETAIL, "test:%x", *test);
        EXEC(bn_mul_u32(result, sizeof(result), num, 4, word));
        THROW_IF(memcmp(result , expect, 16) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, 16);
        dbg_buf(DBG_ERROR, "result:", result, 16);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_mul_u32);

static int
test_bn_mul_u32_a_size_is_not_multiple_4(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    uint8_t num[5] = {0x11, 0x22, 0x33, 0x44, 0x99};
    uint8_t expect[16] = {0xc6, 0x92, 0x5f, 0x2c, 0x11, 0x3d};
    uint8_t result[16] = {0};
    uint32_t word = 0x66;
    uint32_t *test;

    TRY {
        test = num;
        dbg_str(DBG_DETAIL, "test:%x", *test);
        EXEC(bn_mul_u32(result, sizeof(result), num, sizeof(num), word));
        THROW_IF(memcmp(result , expect, sizeof(expect)) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, 16);
        dbg_buf(DBG_ERROR, "result:", result, 16);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_bn_mul_u32_a_size_is_not_multiple_4);

static int
test_bn_random(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    uint8_t dest[1024] = {0};
    int dest_size;

    TRY {
       EXEC(bn_rand(dest, sizeof(dest), &dest_size, 100, 1, 1));
       THROW_IF(dest_size != (100 + 7) / 8, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_bn_random, ret=%d", ret);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_bn_random);