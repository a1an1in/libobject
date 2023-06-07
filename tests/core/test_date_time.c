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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Date_Time.h>

static int test_datetime_assign(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");

        EXEC(date->assign(date, "2021-09-04 13:31:10"));
        
        THROW_IF(date->tm.tm_mday != 4, -1);
        THROW_IF(date->tm.tm_hour != 13, -1);
        THROW_IF(date->tm.tm_min != 31, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(date);
    }

    return 1;
}
REGISTER_TEST_FUNC(test_datetime_assign);

static int test_datetime_next_day(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *start = "2021-09-03 22:31:10", *expect = "2021-09-04 00:00:00";
    char *str;

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, start));

        str = date->next_day(date)->to_format_string(date, "%F %T");
        SET_CATCH_STR_PARS(str, expect);
        dbg_str(DBG_DETAIL, "test_next_day, start=%s, expect=%s", str, expect);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_next_day error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_next_day);

static int test_datetime_cmp_case1(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-09-03 22:31:10", *target = "2021-09-03 22:31:10 UTC+800";

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_cmp_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_cmp_case1);

static int test_datetime_cmp_case2(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-09-03 22:31:11", *target = "2021-09-03 22:31:10";

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret <= 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_cmp_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_cmp_case2);

static int test_datetime_cmp_case3(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-09-03 22:31:10", *target = "2021-09-03 22:31:11";

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, src));
        ret = date->cmp(date, target);

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(ret > 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_cmp_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_cmp_case3);


static int for_each_count = 0;
static int test_datetime_for_callback(char *start, char *end, void *opaque)
{
    dbg_str(DBG_DETAIL, "start:%s end:%s", start, end);
    for_each_count++;

    return 1;
}

static int test_datetime_for_each_day_case1(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-09-03 22:31:10", *target = "2021-09-05 22:31:11";

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);

        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 3, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_for_each_day_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_day_case1);

static int test_datetime_for_each_day_case2(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-09-03 22:31:10", *target = "2021-09-03 22:32:11";

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);

        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 1, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_for_each_day_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_day_case2);

static int test_datetime_for_each_day_case3(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-08-20 22:31:10", *target = "2021-09-01 00:00:01";

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);
        
        EXEC(date->assign(date, src));
        EXEC(date->for_each_day(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 13, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_for_each_day_case3 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_day_case3);

static int test_datetime_for_each_month_case1(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-01-03 22:31:10 UTC+800", *target= "2021-09-05 22:31:11 UTC+800";
    int time_zone;
    char *str;

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);

        EXEC(date->assign(date, src));
        EXEC(date->for_each_month(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 9, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_for_each_month_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_month_case1);

static int test_datetime_for_each_month_case2(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2021-03-05 22:31:11", *target = "2021-09-03 22:31:10";
    int time_zone;
    char *str;

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);

        EXEC(date->assign(date, src));
        EXEC(date->for_each_month(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 7, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_datetime_for_each_month_case2 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_month_case2);

static int test_datetime_for_each_year(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2019-01-03 22:31:10";
    char *target = "2021-09-05 22:31:11";
    char *str;

    TRY {
        for_each_count = 0;
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, src));
        EXEC(date->for_each_year(date, target, test_datetime_for_callback, NULL));

        SET_CATCH_STR_PARS(src, target);
        THROW_IF(for_each_count != 3, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_for_each_year_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_for_each_year);

static int test_datetime_start_of_day(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "2019-01-03 22:31:10", *expect = "2019-01-03 00:00:00";
    char *str;

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        EXEC(date->assign(date, src));
        date->start_of_day(date);
        str = date->to_format_string(date, (char *)"%F %T");

        SET_CATCH_STR_PARS(str, expect);
        dbg_str(DBG_DETAIL, "test start of day:%s", str);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {
        TRY_SHOW_STR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_start_of_day);

static int test_datetime_now(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *str, time1[1024];
    int sec, day;

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        str = date->now(date)->to_format_string(date, (char *)"%F %T");
        strcpy(time1, str);
        dbg_str(DBG_DETAIL, "test_datetime_now time1:%s", time1);
        sec = date->tm.tm_sec;
        day = date->tm.tm_mday;
        sleep(1);
        str = date->now(date)->to_format_string(date, (char *)"%F %T");
        dbg_str(DBG_DETAIL, "test_datetime_now time2:%s", str);
        THROW_IF((date->tm.tm_sec - sec + 60) % 60 != 1, -1);
        THROW_IF(day != date->tm.tm_mday, -1);

        SET_CATCH_STR_PARS(time1, str);
   
    } CATCH (ret) {
        TRY_SHOW_STR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_now);

static int test_datetime_zonetime2local_case1(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *str;
    char *expect = "2021-09-04 12:31:10 UTC+0800";
    char tmp[1024];

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");

        str = date->zonetime2local(date, "2021-09-04 13:31:10 UTC+700");
        THROW_IF(str == NULL, -1);
        THROW_IF(strlen(str) == 0, -1);
        strcpy(tmp, str);

        str = date->zonetime2local(date, expect);

        SET_CATCH_STR_PARS(tmp, str);
        THROW_IF(strcmp(str, tmp) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_timezone1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_zonetime2local_case1);

static int test_datetime_zonetime2local_case2(TEST_ENTRY *entry)
{
    int ret;
    Date_Time *date;
    allocator_t *allocator = allocator_get_default_instance();
    char *str;
    char *expect = "2021-09-03 22:31:10 UTC+0800";
    char tmp[1024];

    TRY {
        date = object_new(allocator, "Date_Time", NULL);
        dbg_str(DBG_DETAIL,"Date_Time_Test assign");

        str = date->zonetime2local(date, "2021-09-04 13:31:10 UTC-700");
        THROW_IF(str == NULL, -1);
        THROW_IF(strlen(str) == 0, -1);
        strcpy(tmp, str);

        str = date->zonetime2local(date, expect);

        SET_CATCH_STR_PARS(tmp, str);
        THROW_IF(strncmp(str, tmp, strlen(str)) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_datetime_zonetime2local_case2 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(date);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_datetime_zonetime2local_case2);

