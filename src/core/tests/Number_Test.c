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
    return 0;
}

static int __teardown(Number_Test *test)
{
    return 0;
}

static int __test_int_number(Number_Test *test)
{
    Number *number = test->number;
    int d = -1, expect_d = -1, ret = 0;

    number->clear(number);
    number->set_type(number, NUMBER_TYPE_SIGNED_INT);

	number->set_format_value(number, "%d", d);
    expect_d = number->get_signed_int_value(number);

    ret = ASSERT_EQUAL(test, &d, &expect_d, sizeof(d));
    if (ret != 1) {
        dbg_str(DBG_ERROR, "test int number, d = %d, expect_d=%d", 
                d, expect_d);
    } else {
        dbg_str(DBG_SUC, "test int number, d = %d, expect_d=%d", 
                d, expect_d);
    }
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
    Init_End___Entry(8 , Number_Test),
};
REGISTER_CLASS("Number_Test", number_test_class_info);
