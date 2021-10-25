#ifndef __DATE_TEST_H__
#define __DATE_TEST_H__

#include <stdio.h>
#include <libobject/core/Date_Time.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>

typedef struct data_test_s Date_Time_Test;

struct data_test_s{
    Test parent;

    int (*construct)(Date_Time_Test *,char *init_str);
    int (*deconstruct)(Date_Time_Test *);
    int (*set)(Date_Time_Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Date_Time_Test *);
    void *(*teardown)(Date_Time_Test *);
    int (*test_assign)(Date_Time_Test *test);
    int (*test_timezone)(Date_Time_Test *test);
    int (*test_next_day)(Date_Time_Test *test);
    int (*test_cmp)(Date_Time_Test *test);
    int (*test_for_each_day)(Date_Time_Test *test);
    int (*test_for_each_month)(Date_Time_Test *test);
    int (*test_for_each_year)(Date_Time_Test *test);
    int (*test_now)(Date_Time_Test *test);
    int (*test_start_of_day)(Date_Time_Test *test);

    Date_Time *date;
};

#endif
