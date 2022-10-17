/**
 * @file Number_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "Number_Test.h"

static int __construct(Number_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    test->number = (Number *)object_new(allocator, "Number", NULL);

    return 0;
}

static int __deconstrcut(Number_Test *test)
{
    object_destroy(test->number);

    return 0;
}

static int __setup(Number_Test *test, char *init_str)
{
    return 1;
}

static int __teardown(Number_Test *test)
{
    return 1;
}

static int __test_set_int_number(Number_Test *test)
{
    Number *number = test->number;
    int d = -1, expect_d = -1, ret = 0;
    int len = sizeof(int);

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_SIGNED_INT);
        number->set_value(number,  &d, -1);
        number->get_value(number, &expect_d, &len);

        SET_CATCH_INT_PARS(d, expect_d);
        THROW_IF(d != expect_d, -1);

        d = -2;
        d = NUM2S32(number);
        SET_CATCH_INT_PARS(d, expect_d);
        THROW_IF(d != expect_d, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "int_number error, par1=%d, par2=%d", ERROR_INT_PAR1(), ERROR_INT_PAR2());
    }

    return ret;
}

static int __test_obj_signed_int_add_obj_signed_int(Number_Test *test)
{
    Number *number = test->number, *add;
    allocator_t *allocator = allocator_get_default_alloc();
    int num1 = 1, num2 = 2, expect_d = 3, ret = 0, sum = 0;
    int len = sizeof(int);

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_SIGNED_INT);
        number->set_value(number,  &num1, -1);

        add = object_new(allocator, "Number", NULL);
        add->set_type(add, NUMBER_TYPE_OBJ_SIGNED_INT);
        add->set_value(add, &num2, -1);

        number->add(number, NUMBER_TYPE_OBJ_SIGNED_INT, add, sizeof(int));
        number->get_value(number, &sum, &len);
        SET_CATCH_INT_PARS(expect_d, sum);
        THROW_IF(sum != expect_d, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(add);
    }

    return ret;
}

static int __test_obj_big_number_add_obj_big_number(Number_Test *test)
{
    Number *number = test->number, *add;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[10] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    uint8_t num2[10] = {0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    uint8_t expect_d[11] = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xff};
    uint8_t sum[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, sum);

        add = object_new(allocator, "Number", NULL);
        add->set_type(add, NUMBER_TYPE_OBJ_BIG_NUMBER);
        add->set_value(add, &num2, sizeof(num2));

        number->add(number, NUMBER_TYPE_OBJ_BIG_NUMBER, add, sizeof(int));
        number->get_value(number, &sum, &len);
        THROW_IF(memcmp(sum , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "real:", sum, len);
        
    } FINALLY {
        object_destroy(add);
    }

    return ret;
}

static int __test_obj_big_number_add_obj_big_number_with_carry(Number_Test *test)
{
    Number *number = test->number, *add;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0xff, 0xff, 0xff, 0xff, 0x1, 0x1, 0x1, 0x1};
    uint8_t num2[6] = {0xff, 0xff, 0xff, 0xff};
    uint8_t expect_d[11] = {0xfe, 0xff, 0xff, 0xff, 0x02, 0x01, 0x01, 0x01};
    uint8_t sum[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        add = object_new(allocator, "Number", NULL);
        add->set_type(add, NUMBER_TYPE_OBJ_BIG_NUMBER);
        add->set_value(add, &num2, sizeof(num2));

        number->add(number, NUMBER_TYPE_OBJ_BIG_NUMBER, add, sizeof(int));
        number->get_value(number, &sum, &len);
        THROW_IF(memcmp(sum , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "real:", sum, len);
    } FINALLY {
        object_destroy(add);
    }

    return ret;
}

static int __test_obj_signed_int_add_signed_int(Number_Test *test)
{
    Number *number = test->number;
    int num1 = 1, num2 = 2, expect_d = 3, ret = 0, sum = 0;
    int len = sizeof(int);

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_SIGNED_INT);
        number->set_value(number,  &num1, -1);

        number->add(number, NUMBER_TYPE_SIGNED_INT, &num2, sizeof(int));
        number->get_value(number, &sum, &len);
        SET_CATCH_INT_PARS(expect_d, sum);
        THROW_IF(sum != expect_d, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __test_add(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_obj_signed_int_add_signed_int(test));
        EXEC(__test_obj_signed_int_add_obj_signed_int(test));
        EXEC(__test_obj_big_number_add_obj_big_number(test));
        EXEC(__test_obj_big_number_add_obj_big_number_with_carry(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }
    return ret;
}

static int __test_obj_big_number_sub_obj_big_number(Number_Test *test)
{
    Number *number = test->number, *sub;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0x44, 0x66};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        sub = object_new(allocator, "Number", NULL);
        sub->set_type(sub, NUMBER_TYPE_OBJ_BIG_NUMBER);
        sub->set_value(sub, &num2, sizeof(num2));

        number->sub(number, NUMBER_TYPE_OBJ_BIG_NUMBER, sub, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(sub);
    }

    return ret;
}

static int __test_obj_big_number_sub_obj_big_number_with_size_change(Number_Test *test)
{
    Number *number = test->number, *sub;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x88};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0x44};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        sub = object_new(allocator, "Number", NULL);
        sub->set_type(sub, NUMBER_TYPE_OBJ_BIG_NUMBER);
        sub->set_value(sub, &num2, sizeof(num2));

        number->sub(number, NUMBER_TYPE_OBJ_BIG_NUMBER, sub, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
        THROW_IF(len != 7, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(sub);
    }

    return ret;
}

static int __test_obj_big_number_sub_obj_big_number_with_size_change2(Number_Test *test)
{
    Number *number = test->number, *sub;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x1};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};
    uint8_t expect_d[8] = {0x78, 0x99, 0xbb, 0xdd, 0xff, 0x21, 0xff};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        sub = object_new(allocator, "Number", NULL);
        sub->set_type(sub, NUMBER_TYPE_OBJ_BIG_NUMBER);
        sub->set_value(sub, &num2, sizeof(num2));

        number->sub(number, NUMBER_TYPE_OBJ_BIG_NUMBER, sub, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
        THROW_IF(len != 7, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(sub);
    }

    return ret;
}

static int __test_obj_big_number_sub_obj_big_number_with_neg_result(Number_Test *test)
{
    Number *number = test->number, *sub;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};
    uint8_t expect_d[8] = {0x88, 0x66, 0x44, 0x22, 0xf00, 0xde};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);
        number->set_value(number,  &num1, sizeof(num1));

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        sub = object_new(allocator, "Number", NULL);
        sub->set_type(sub, NUMBER_TYPE_OBJ_BIG_NUMBER);
        sub->set_value(sub, &num2, sizeof(num2));

        number->sub(number, NUMBER_TYPE_OBJ_BIG_NUMBER, sub, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(sub);
    }

    return ret;
}

static int __test_sub(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_obj_big_number_sub_obj_big_number(test));
        EXEC(__test_obj_big_number_sub_obj_big_number_with_size_change(test));
        EXEC(__test_obj_big_number_sub_obj_big_number_with_size_change2(test));
        EXEC(__test_obj_big_number_sub_obj_big_number_with_neg_result(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }
    return ret;
}

static int __test_obj_big_number_mul_obj_big_number_case1(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66};
    uint8_t expect_d[9] = {0x29, 0x64, 0x8f, 0x88, 0x88, 0x92, 0xfc, 0x20, 0x22};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->mul(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_mul_obj_big_number_case2(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t num2[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0x99};
    uint8_t expect_d[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0x71, 0xa3, 0x70, 0x3d, 0x0a, 0xd7, 0xa3, 0x70, 0x14};
    uint8_t result[20] = {0};
    int len = sizeof(result), ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, num2, sizeof(num2));

        number->mul(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_mul(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_obj_big_number_mul_obj_big_number_case1(test));
        EXEC(__test_obj_big_number_mul_obj_big_number_case2(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }
    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case1(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num2[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case2(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[9] = {0x11, 0x22, 0x33, 0x44};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {1};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case3(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0x99, 0x88, 0x77, 0x66};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
        dbg_str(DBG_ERROR, "len:%d", len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case4(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[100] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xff, 0x88, 
                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, 0x22, 
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88,
                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, 0x22,
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88,
                        0x33, 0x44, 0x55, 0x66, 0xff, 0x66, 0xff, 0x88};
    uint8_t num2[20] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22};
    uint8_t result[200] = {0};
    int len = sizeof(result), ret;

    TRY {
        dbg_str(DBG_FLOW, "test_obj_big_number_div_obj_big_number_case4");
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        EXEC(number->mul(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(num1), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(num1)));
        EXEC(number->get_value(number, &result, &len));
        dbg_buf(DBG_DETAIL, "mul result:", result, len);
        a1->set_value(a1, result, len);
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_FLOW, "test_obj_big_number_div_obj_big_number_case4, div begin");
        EXEC(number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int)));
        EXEC(number->get_value(number, &result, &len));
        THROW_IF(memcmp(result, num1, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", num1, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
        dbg_str(DBG_ERROR, "len:%d", len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case5(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[20] = {0x55, 0x66, 0x77, 0x88, 0x99};
    uint8_t num2[20] = {0x88, 0x99};
    uint8_t expect[80] = {0xc7, 0, 0, 1};
    uint8_t result[80] = {0};
    int len = sizeof(result), ret;

    TRY {
        dbg_str(DBG_FLOW, "test_obj_big_number_div_obj_big_number_case5");
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, num2, sizeof(num2));

        EXEC(number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(num1), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(num2)));
        EXEC(number->get_value(number, &result, &len));
        THROW_IF(memcmp(expect, result, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
        dbg_str(DBG_ERROR, "len:%d", len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_div_obj_big_number_case6(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[20] = {0x55, 0x66, 0x77, 0x88, 0x99};
    uint8_t num2[20] = {0x2};
    uint8_t expect[80] = {0x2a, 0xb3, 0x3b, 0xc4, 0x4c};
    uint8_t result[80] = {0};
    int len = sizeof(result), ret;

    TRY {
        dbg_str(DBG_FLOW, "test_obj_big_number_div_obj_big_number_case5");
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, num2, sizeof(num2));

        EXEC(number->div(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(num1), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(num2)));
        EXEC(number->get_value(number, &result, &len));
        THROW_IF(memcmp(expect, result, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
        dbg_str(DBG_ERROR, "len:%d", len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_div(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_obj_big_number_div_obj_big_number_case1(test));
        EXEC(__test_obj_big_number_div_obj_big_number_case2(test));
        EXEC(__test_obj_big_number_div_obj_big_number_case3(test));
        EXEC(__test_obj_big_number_div_obj_big_number_case4(test));
        EXEC(__test_obj_big_number_div_obj_big_number_case5(test));
        EXEC(__test_obj_big_number_div_obj_big_number_case6(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }
    return ret;
}

static int __test_obj_big_number_mod_obj_big_number_case1(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t num2[9] = {0x29, 0x64, 0x8f, 0x88, 0xbb, 0x37, 0x4c, 0x1b};
    uint8_t expect_d[8] = {0};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->mod(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(int), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(int));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , num1, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", num1, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_mod_obj_big_number_case2(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, num1, sizeof(num1));

        number->mod(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(num1), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(num1));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result, expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_obj_big_number_mod_obj_big_number_case3(Number_Test *test)
{
    Number *number = test->number, *a1, *a2;
    allocator_t *allocator = allocator_get_default_alloc();
    uint8_t num1[9] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t num2[8] = {0x11, 0x22, 0x33, 0x44};
    uint8_t expect_d[8] = {0xd1, 0x8c, 0x48, 0x4};
    uint8_t result[11] = {0};
    int len = 11, ret;

    TRY {
        number->clear(number);
        number->set_type(number, NUMBER_TYPE_OBJ_BIG_NUMBER);

        dbg_str(DBG_DETAIL, "num2:%p, expect_d:%p, ret:%p, sum:%p, ", num2, expect_d, &ret, result);

        a1 = object_new(allocator, "Number", NULL);
        a1->set_type(a1, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a1->set_value(a1, &num1, sizeof(num1));

        a2 = object_new(allocator, "Number", NULL);
        a2->set_type(a2, NUMBER_TYPE_OBJ_BIG_NUMBER);
        a2->set_value(a2, &num2, sizeof(num2));

        number->mod(number, NUMBER_TYPE_OBJ_BIG_NUMBER, a1, sizeof(a1), NUMBER_TYPE_OBJ_BIG_NUMBER, a2, sizeof(a2));
        number->get_value(number, &result, &len);
        THROW_IF(memcmp(result , expect_d, len) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_buf(DBG_ERROR, "expect:", expect_d, len);
        dbg_buf(DBG_ERROR, "result:", result, len);
        dbg_str(DBG_ERROR, "len:%d", len);
    } FINALLY {
        object_destroy(a1);
        object_destroy(a2);
    }

    return ret;
}

static int __test_mod(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_obj_big_number_mod_obj_big_number_case1(test));
        EXEC(__test_obj_big_number_mod_obj_big_number_case2(test));
        EXEC(__test_obj_big_number_mod_obj_big_number_case3(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }
    return ret;
}

static class_info_entry_t number_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Number_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Number_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Number_Test, set, NULL),
    Init_Vfunc_Entry(4 , Number_Test, get, NULL),
    Init_Vfunc_Entry(5 , Number_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Number_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Number_Test, test_int_number, __test_set_int_number),
    Init_Vfunc_Entry(8 , Number_Test, test_add, __test_add),
    Init_Vfunc_Entry(9 , Number_Test, test_sub, __test_sub),
    Init_Vfunc_Entry(10, Number_Test, test_mul, __test_mul),
    Init_Vfunc_Entry(11, Number_Test, test_div, __test_div),
    Init_Vfunc_Entry(12, Number_Test, test_mod, __test_mod),
    Init_End___Entry(13, Number_Test),
};
REGISTER_CLASS("Number_Test", number_test_class_info);