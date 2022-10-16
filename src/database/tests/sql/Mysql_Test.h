#ifndef __MYSQL_TEST_H__
#define __MYSQL_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/database/sql/Mysql_DB.h>

typedef struct mysql_test_s Mysql_Test;

struct mysql_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_connect)(Mysql_Test *test);
    int (*test_create_table)(Mysql_Test *test);
    int (*test_drop_table)(Mysql_Test *test);
    int (*test_insert)(Mysql_Test *test);
    int (*test_del)(Mysql_Test *test);
    int (*test_update)(Mysql_Test *test);
    int (*test_query)(Mysql_Test *test);

    Mysql *mysql;
};

#endif
