/**
 * @file Date_Time_Test.c
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
#include "Date_Time_Test.h"

static int __construct(Date_Time_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    dbg_str(DBG_DETAIL,"Date_Time_Test construct");
    test->date = object_new(allocator, "Date_Time", NULL);

    return 1;
}

static int __deconstrcut(Date_Time_Test *test)
{
    dbg_str(DBG_DETAIL,"Date_Time_Test deconstrcut");
    OBJECT_DESTROY(test->date);

    return 0;
}

static int __setup(Date_Time_Test *test, char *init_str)
{

    dbg_str(DBG_DETAIL,"Date_Time_Test set up");

    return 0;
}

static int __teardown(Date_Time_Test *test)
{
    dbg_str(DBG_DETAIL,"Date_Time_Test teardown");

    return 0;
}

static int __test_assign(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;

    TRY {
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");
        date = test->date;

        EXEC(date->assign(date, "2021-09-04 13:31:10 UTC-800"));
        str = date->to_format_string(date, "%F %T UTC%z");
        THROW_IF(str == NULL, -1);
        THROW_IF(strlen(str) == 0, -1);
        dbg_str(DBG_DETAIL, "%s", str);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return 1;
}

static int __test_timezone1(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;
    char *expect = "2021-09-04 12:31:10 UTC+0800";

    TRY {
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");
        date = test->date;

        EXEC(date->assign(date, "2021-09-04 13:31:10 UTC+700"));
        str = date->to_format_string(date, "%F %T UTC%z");
        THROW_IF(str == NULL, -1);
        THROW_IF(strlen(str) == 0, -1);

        SET_CATCH_PTR_PAR(str, expect);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_timezone1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_timezone2(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;
    char *expect = "2021-09-03 22:31:10 UTC+0800";

    TRY {
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");
        date = test->date;

        EXEC(date->assign(date, "2021-09-04 13:31:10 UTC-700"));
        str = date->to_format_string(date, "%F %T UTC%z");
        THROW_IF(str == NULL, -1);
        THROW_IF(strlen(str) == 0, -1);
        SET_CATCH_PTR_PAR(str, expect);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_timezone1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_timezone(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;

    TRY {
        EXEC(__test_timezone1(test));
        EXEC(__test_timezone2(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return 1;
}

static class_info_entry_t data_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Date_Time_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Date_Time_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Date_Time_Test, set, NULL),
    Init_Vfunc_Entry(4 , Date_Time_Test, get, NULL),
    Init_Vfunc_Entry(5 , Date_Time_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Date_Time_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Date_Time_Test, test_assign, __test_assign),
    Init_Vfunc_Entry(8 , Date_Time_Test, test_timezone, __test_timezone),
    Init_End___Entry(9 , Date_Time_Test),
};
REGISTER_CLASS("Date_Time_Test", data_test_class_info);
