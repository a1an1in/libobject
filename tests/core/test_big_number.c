#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libobject/core/utils/bn/big_number.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>

static int
test_bn_cmp(TEST_ENTRY *entry)
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

static int test_bn_mul_u32(TEST_ENTRY *entry)
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
REGISTER_TEST_FUNC(test_bn_mul_u32);

static int test_bn_mul_u32_a_size_is_not_multiple_4(TEST_ENTRY *entry)
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

static int test_bn_random(TEST_ENTRY *entry)
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

static int test_bn_add_simple(TEST_ENTRY *entry)
{
    uint8_t num1[10] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    uint8_t num2[10] = {0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    uint8_t expect_d[11] = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xff};
    uint8_t sum[11] = {0};
    int len = sizeof(num1), ret;

    TRY {
        memcpy(sum, num1, sizeof(num1));
        EXEC(bn_add(sum, sizeof(sum), &len, num2, sizeof(num2)));
        THROW_IF(memcmp(sum, expect_d, len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "real:", sum, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_add_simple);

static int test_bn_add_with_carry(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0xff, 0xff, 0xff, 0xff, 0x1, 0x1, 0x1, 0x1};
    uint8_t num2[6] = {0xff, 0xff, 0xff, 0xff};
    uint8_t expect_d[11] = {0xfe, 0xff, 0xff, 0xff, 0x02, 0x01, 0x01, 0x01};
    uint8_t sum[11] = {0};
    int len = 0, ret;

    TRY {
        memcpy(sum, num1, sizeof(num1));
        EXEC(bn_add(sum, sizeof(sum), &len, num2, sizeof(num2)));
        THROW_IF(memcmp(sum, expect_d, len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "real:", sum, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_add_with_carry);

static int test_bn_sub_basic(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0x44, 0x66};
    int neg_flag = 0;
    int len = sizeof(num1), ret;

    TRY {
        EXEC(bn_sub(num1, sizeof(num1), &len, &neg_flag, num2, sizeof(num2)));
        THROW_IF(memcmp(num1, expect_d, len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", num1, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_sub_basic);

static int test_bn_sub_with_size_change_case1(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x88};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0x44};
    uint8_t result[11] = {0};
    int neg_flag = 0;
    int len = sizeof(num1), ret;

    TRY {
        EXEC(bn_sub(num1, sizeof(num1), &len, &neg_flag, num2, sizeof(num2)));
        THROW_IF(memcmp(num1, expect_d, len) != 0, -1);
        THROW_IF(len != (sizeof(num1) - 1), -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", num1, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_sub_with_size_change_case1);

static int test_bn_sub_with_size_change_case2(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x1};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0xff};
    uint8_t result[11] = {0};
    int neg_flag = 0;
    int len = sizeof(num1), ret;

    TRY {
        EXEC(bn_sub(num1, sizeof(num1), &len, &neg_flag, num2, sizeof(num2)));
        THROW_IF(memcmp(num1, expect_d, len) != 0, -1);
        THROW_IF(len != (sizeof(num1) - 1), -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", num1, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_sub_with_size_change_case2);

static int test_bn_sub_with_neg_result(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};
    uint8_t expect_d[6] = {0x88, 0x66, 0x44, 0x22, 0x00, 0xde};
    uint8_t result[11] = {0};
    int neg_flag = 0;
    int len = sizeof(num1), ret;

    TRY {
        EXEC(bn_sub(num1, sizeof(num1), &len, &neg_flag, num2, sizeof(num2)));
        THROW_IF(memcmp(num1, expect_d, len) != 0, -1);
        THROW_IF(len != sizeof(expect_d), -1);
        THROW_IF(neg_flag != 1, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", num1, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_sub_with_neg_result);

static int test_bn_mul_case1(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66};
    uint8_t expect_d[9] = {0x29, 0x64, 0x8f, 0x88, 0x88, 0x92, 0xfc, 0x20, 0x22};
    uint8_t result[11] = {0};
    int len = 0, ret;

    TRY {
        EXEC(bn_mul(result, sizeof(result), &len, num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_mul_case1);

static int test_bn_mul_case2(TEST_ENTRY *entry)
{
    uint8_t num1[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t num2[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0x99};
    uint8_t expect_d[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0x71, 0xa3, 0x70, 0x3d, 0x0a, 0xd7, 0xa3, 0x70, 0x14};
    uint8_t result[20] = {0};
    int len = 0, ret;

    TRY {
        EXEC(bn_mul(result, sizeof(result), &len, num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(result, expect_d, len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_mul_case2);

static int test_bn_div_case1(TEST_ENTRY *entry)
{
    uint8_t num2[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0};
    uint8_t quotient[11] = {0};
    uint8_t remainder[11] = {0};
    int quotient_len = 11, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, expect_d, quotient_len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, quotient_len);
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case1);

static int test_bn_div_case2(TEST_ENTRY *entry)
{
    uint8_t num1[9] = {0x11, 0x22, 0x33, 0x44};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {1};
    uint8_t quotient[11] = {0};
    uint8_t remainder[11] = {0};
    int quotient_len = 11, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, expect_d, quotient_len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, quotient_len);
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case2);

static int test_bn_div_case3(TEST_ENTRY *entry)
{
    uint8_t num1[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0x99, 0x88, 0x77, 0x66};
    uint8_t quotient[11] = {0};
    uint8_t remainder[11] = {0};
    int quotient_len = 11, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, expect_d, quotient_len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect_d, quotient_len);
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case3);

static int test_bn_div_case4(TEST_ENTRY *entry)
{
    uint8_t num1[100] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xff, 0x88, 
                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, 0x22, 
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88,
                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, 0x22,
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88,
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88};
    uint8_t num2[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t quotient[200] = {0}, remainder[200] = {0}, result[200] = {0};
    int quotient_len = 0, remainder_len = 0, len = 0, ret;

    TRY {
        EXEC(bn_mul(result, sizeof(result), &len, num1, sizeof(num1), num2, sizeof(num2)));
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
                    remainder, sizeof(remainder), &remainder_len,
                    result, sizeof(result), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, num1, sizeof(num1)) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", num1, sizeof(num1));
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case4);

static int test_bn_div_case5(TEST_ENTRY *entry)
{
    uint8_t num1[20] = {0x55, 0x66, 0x77, 0x88, 0x99};
    uint8_t num2[20] = {0x88, 0x99};
    uint8_t expect[20] = {0xc7, 0, 0, 1};
    uint8_t quotient[20] = {0};
    uint8_t remainder[20] = {0};
    int quotient_len = 0, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, expect, quotient_len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, quotient_len);
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case5);

static int test_bn_div_case6(TEST_ENTRY *entry)
{
    uint8_t num1[20] = {0x55, 0x66, 0x77, 0x88, 0x99};
    uint8_t num2[20] = {0x2};
    uint8_t expect[80] = {0x2a, 0xb3, 0x3b, 0xc4, 0x4c};
    uint8_t quotient[20] = {0};
    uint8_t remainder[20] = {0};
    int quotient_len = 0, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(quotient, expect, quotient_len) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, quotient_len);
        dbg_buf(DBG_ERROR, "result:", quotient, quotient_len);
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_div_case6);

static int test_bn_mod_case1(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t num2[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t expect_d[8] = {0};
    uint8_t quotient[20] = {0};
    uint8_t remainder[20] = {0};
    int quotient_len = 0, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(remainder, num1, sizeof(num1)) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", num1, sizeof(num1));
        dbg_buf(DBG_ERROR, "result:", remainder, sizeof(remainder));
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_mod_case1);

static int test_bn_mod_case2(TEST_ENTRY *entry)
{
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect[8] = {0};
    uint8_t quotient[20] = {0};
    uint8_t remainder[20] = {0};
    int quotient_len = 0, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num1, sizeof(num1)));
        THROW_IF(memcmp(remainder, expect, sizeof(expect)) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, sizeof(expect));
        dbg_buf(DBG_ERROR, "result:", remainder, sizeof(remainder));
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_mod_case2);

static int test_bn_mod_case3(TEST_ENTRY *entry)
{
    uint8_t num1[9] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect[8] = {0xd1, 0x8c, 0x48, 0x4};
    uint8_t result[9] = {0};
    uint8_t quotient[9] = {0};
    uint8_t remainder[9] = {0};
    int quotient_len = 0, remainder_len = 0, ret;

    TRY {
        EXEC(bn_div(quotient, sizeof(quotient), &quotient_len, 
             remainder, sizeof(remainder), &remainder_len,
             num1, sizeof(num1), num2, sizeof(num2)));
        THROW_IF(memcmp(remainder, expect, sizeof(expect)) != 0, -1);
    } CATCH (ret) {
        dbg_buf(DBG_ERROR, "expect:", expect, sizeof(expect));
        dbg_buf(DBG_ERROR, "result:", remainder, sizeof(remainder));
    } FINALLY {}

    return ret;
}
REGISTER_TEST_FUNC(test_bn_mod_case3);