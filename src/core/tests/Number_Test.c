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

static int __test_int_number(Number_Test *test)
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

static int __test_add_case1(Number_Test *test)
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

static int __test_add(Number_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_add_case1(test));
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
    Init_Vfunc_Entry(7 , Number_Test, test_int_number, __test_int_number),
    Init_Vfunc_Entry(8 , Number_Test, test_add, __test_add),
    Init_End___Entry(9 , Number_Test),
};
REGISTER_CLASS("Number_Test", number_test_class_info);
