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

        SET_CATCH_STR_PARS(str, expect);
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
        SET_CATCH_STR_PARS(str, expect);
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
    char *str;

    TRY {
        EXEC(__test_timezone1(test));
        EXEC(__test_timezone2(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_next_day(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *start = "2021-09-03 22:31:10 UTC+0800";
    char *expect = "2021-09-04 00:00:00 UTC+0800";
    char *str;

    TRY {
        date = test->date;
        str = date->next_day(date)->to_format_string(date, "%F %T UTC%z");
        SET_CATCH_STR_PARS(str, expect);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_next_day error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_cmp_case1(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-03 22:31:10 UTC+0800";
    char *target = "2021-09-03 22:31:10 UTC+0800";
    char *str;

    TRY {
        date = test->date;
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_cmp_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_cmp_case2(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-03 22:31:11 UTC+0800";
    char *target = "2021-09-03 22:31:10 UTC+0800";
    char *str;

    TRY {
        date = test->date;
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret <= 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_cmp_case2 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_cmp_case3(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-03 22:31:10 UTC+0800";
    char *target = "2021-09-03 22:31:11 UTC+0800";
    char *str;

    TRY {
        date = test->date;
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret > 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_cmp_case3 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_cmp(Date_Time_Test *test)
{
    int ret;
    char *str;

    TRY {
        EXEC(__test_cmp_case1(test));
        EXEC(__test_cmp_case2(test));
        EXEC(__test_cmp_case3(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int for_each_count = 0;
static int __test_for_callback(char *start, char *end, void *opaque)
{
    dbg_str(DBG_DETAIL, "start:%s end:%s", start, end);
    for_each_count++;

    return 1;
}

static int __test_for_each_day_case1(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-03 22:31:10 UTC+0800";
    char *target = "2021-09-05 22:31:11 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 3, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_day_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_day_case2(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-03 22:31:10 UTC+0800";
    char *target = "2021-09-03 22:32:11 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 1, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_day_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_day_case3(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-08-20 22:31:10 UTC+0800";
    char *target = "2021-09-01 00:00:01 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 13, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_day_case3 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_day(Date_Time_Test *test)
{
    int ret;
    char *str;

    TRY {
        EXEC(__test_for_each_day_case1(test));
        EXEC(__test_for_each_day_case2(test));
        EXEC(__test_for_each_day_case3(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_for_each_month_case1(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-01-03 22:31:10 UTC+0800";
    char *target = "2021-09-05 22:31:11 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_month(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 9, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_month_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_month_case2(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2021-09-05 22:31:11 UTC+0800";
    char *target = "2022-01-03 22:31:10 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_month(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 5, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_month_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_month(Date_Time_Test *test)
{
    int ret;
    char *str;

    TRY {
        EXEC(__test_for_each_month_case1(test));
        EXEC(__test_for_each_month_case2(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_for_each_year_case1(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *src = "2019-01-03 22:31:10 UTC+0800";
    char *target = "2021-09-05 22:31:11 UTC+0800";
    char *str;

    TRY {
        for_each_count = 0;
        date = test->date;
        EXEC(date->assign(date, src));
        EXEC(date->for_each_year(date, target, __test_for_callback, test));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 3, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_for_each_year_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static int __test_for_each_year(Date_Time_Test *test)
{
    int ret;
    char *str;

    TRY {
        EXEC(__test_for_each_year_case1(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_now(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;

    TRY {
        date = test->date;
        str = date->now(date)->to_format_string(date, (char *)"%F %T UTC%z");

        SET_CATCH_STR_PARS(str, "");
        dbg_str(DBG_DETAIL, "test now:%s", str);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_STR_PARS(DBG_ERROR);
    }

    return ret;
}

static int __test_start_of_day(Date_Time_Test *test)
{
    int ret;
    Date_Time *date;
    char *str;

    TRY {
        date = test->date;
        date->now(date);
        date->start_of_day(date);
        str = date->to_format_string(date, (char *)"%F %T UTC%z");

        SET_CATCH_STR_PARS(str, "");
        dbg_str(DBG_DETAIL, "test start of day:%s", str);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_STR_PARS(DBG_ERROR);
    }

    return ret;
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
    Init_Vfunc_Entry(9 , Date_Time_Test, test_next_day, __test_next_day),
    Init_Vfunc_Entry(10, Date_Time_Test, test_cmp, __test_cmp),
    Init_Vfunc_Entry(11, Date_Time_Test, test_for_each_day, __test_for_each_day),
    Init_Vfunc_Entry(12, Date_Time_Test, test_for_each_month, __test_for_each_month),
    Init_Vfunc_Entry(13, Date_Time_Test, test_for_each_year, __test_for_each_year),
    Init_Vfunc_Entry(14, Date_Time_Test, test_now, __test_now),
    Init_Vfunc_Entry(15, Date_Time_Test, test_start_of_day, __test_start_of_day),
    Init_End___Entry(16, Date_Time_Test),
};
REGISTER_CLASS("Date_Time_Test", data_test_class_info);
