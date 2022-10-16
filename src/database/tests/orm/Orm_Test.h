#ifndef __ORM_TEST_H__
#define __ORM_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/database/orm/Orm.h>

typedef struct orm_test_s Orm_Test;

struct orm_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);

	/*virtual methods reimplement*/
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_create)(Orm_Test *test);
    int (*test_insert_table)(Orm_Test *test);
    int (*test_del)(Orm_Test *test);
    int (*test_update_model)(Orm_Test *test);
    int (*test_update_json)(Orm_Test *test);
    int (*test_query_table)(Orm_Test *test);
    int (*test_query_model)(Orm_Test *test);
    int (*test_merge_table)(Orm_Test *test);
    int (*test_drop)(Orm_Test *test);
    int (*test_insert_or_update_table)(Orm_Test *test);

    /*attribs*/
    Orm *orm;
};

#endif
