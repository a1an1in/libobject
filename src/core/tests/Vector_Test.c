/**
 * @file Vector_Test.c
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
#include <libobject/core/try.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "Simplest_Obj.h"
#include "Vector_Test.h"

static int __construct(Test *test, char *init_str)
{
    Vector_Test *t = (Vector_Test *)test;
    allocator_t *allocator = test->obj.allocator;

    dbg_str(DBG_DETAIL,"Vector_Test construct");

    t->vector = object_new(allocator, "Vector", NULL);
    return 0;
}

static int __deconstrcut(Test *test)
{
    Vector_Test *t = (Vector_Test *)test;

    dbg_str(DBG_DETAIL,"Vector_Test deconstruct");
    if (t->vector != NULL) {
        object_destroy(t->vector);
    }

    return 0;
}

static int __setup(Vector_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    dbg_str(DBG_DETAIL,"Vector_Test set up");

    return 1;
}

static int __teardown(Vector_Test *test)
{
    dbg_str(DBG_DETAIL,"Vector_Test teardown");
    test->vector->reset(test->vector);

    return 1;
}

static int __test_vector_add(Vector_Test *test)
{
    Vector *vector = test->vector;
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int expect_ret = 1;

    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 1);
    vector->remove(vector, 0, (void **)&t);

    ASSERT_EQUAL(test, &t, &expect_ret, sizeof(expect_ret));
}

static int __test_vector_remove(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret = 2;

    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->remove(vector, 2, (void **)&t);

    ASSERT_EQUAL(test, &t, &ret, sizeof(ret));
}

static int __test_vector_count(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret, count, expect_count = 5;

    vector->set(vector, "/Vector/value_type", &value_type);

    count = vector->count(vector);
    printf("count=%d\n", count);
    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    count = vector->count(vector);
    printf("count=%d\n", count);

    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_vector_reset(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret, count, expect_count = 0;

    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->reset(vector);
    count = vector->count(vector);

    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_vector_to_json_case1(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *result = "[0, 1, 2, 3, 4, 5]";

    TRY {
        vector->reset(vector);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);
        vector->add_at(vector, 5, 5);

        THROW_IF(strcmp(vector->to_json(vector), result) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_to_json_case2(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret;
    uint8_t trustee_flag = 1;
    char result[1024];

    TRY {
        vector->reset(vector);
        t0 = object_new(allocator, "String", "Monday");
        t1 = object_new(allocator, "String", "Tuesday");
        t2 = object_new(allocator, "String", "Wednesday");
        t3 = object_new(allocator, "String", "Thursday");
        t4 = object_new(allocator, "String", "Friday");
        t5 = object_new(allocator, "String", "Saturday");

        sprintf(result, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", 
                t0->get_cstr(t0), t1->get_cstr(t1), t2->get_cstr(t2),
                t3->get_cstr(t3), t4->get_cstr(t4), t5->get_cstr(t5));

        vector->set(vector, "/Vector/capacity", &capacity);
        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

        vector->add_at(vector, 0, t0);
        vector->add_at(vector, 1, t1);
        vector->add_at(vector, 2, t2);
        vector->add_at(vector, 3, t3);
        vector->add_at(vector, 4, t4);
        vector->add_at(vector, 5, t5);

        dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
        dbg_str(DBG_DETAIL, "result: %s", result);

        THROW_IF(strcmp(vector->to_json(vector), result) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_to_json_case3(Vector_Test *test)
{
    int ret = 0, help = 0;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Vector *vector = test->vector;
    uint8_t trustee_flag = 1;
    Obj *obj1 = NULL;
    Obj *obj2 = NULL;
    String *string = NULL;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";

    TRY {
        vector->reset(vector);
        obj1 = object_new(allocator, "Simplest_Obj", NULL);
        obj2 = object_new(allocator, "Simplest_Obj", NULL);

        help = 1;
        obj1->set(obj1, "/Simplest_Obj/help", &help);
        obj1->set(obj1, "/Simplest_Obj/name", "simplest obj1");

        help = 2;
        obj2->set(obj2, "/Simplest_Obj/help", &help);
        obj2->set(obj2, "/Simplest_Obj/name", "simplest obj2");


        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

        vector->add(vector, obj1);
        vector->add(vector, obj2);

        string = object_new(allocator, "String", NULL);
        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);

        SET_CATCH_PTR_PAR(string->get_cstr(string), init_data);
        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_vector_to_json_case3 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int __test_vector_to_json(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_to_json_case1(test));
        EXEC(__test_vector_to_json_case2(test));
        EXEC(__test_vector_to_json_case3(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}
static int __test_vector_assign_case1(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *init_data = "[0, 1, 2, 3, 4, 5]";

    TRY {
        vector->reset(vector);
        vector->set(vector, "/Vector/capacity", &capacity);
        vector->assign(vector,  init_data);

        THROW_IF(strcmp(vector->to_json(vector), init_data) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_assign_case2(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    uint8_t trustee_flag = 1;
    int ret;
    char *init_data = "[\"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";

    TRY {
        vector->reset(vector);
        vector->set(vector, "/Vector/capacity", &capacity);
        vector->assign(vector,  init_data);

        dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
        THROW_IF(strcmp(vector->to_json(vector), init_data) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_assign_case3(Vector_Test *test)
{
    int ret;
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";
    String *string, **addr;

    TRY {
        vector->reset(vector);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        addr = vector->get(vector, "/Vector/class_name");
        vector->assign(vector, init_data);

        string = object_new(allocator, "String", NULL);
        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);

        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int __test_vector_assign(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_assign_case1(test));
        EXEC(__test_vector_assign_case2(test));
        EXEC(__test_vector_assign_case3(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_new_case1(Vector_Test *test)
{
    int ret;
    Vector *vector = NULL;
    String *string = NULL;
    allocator_t *allocator = allocator_get_default_alloc();
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, "(Simplest_Obj):");
        string->append(string, init_data, -1);
        vector = object_new(allocator, "Vector", STR2A(string));

        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);
        dbg_str(DBG_DETAIL, "expect:%s, result:%s", string->get_cstr(string), init_data);

        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(vector);
        object_destroy(string);
    }
}

static int __test_vector_new(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_new_case1(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __int_vector_element_cmp(void *element, void *key)
{
    int data;

    data = (int)element;

    return data == *(int *)key ? 1 : 0;
}

static int __test_vector_search(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5, index;
    void *element;

    TRY {
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        EXEC(vector->search(vector, __int_vector_element_cmp, &t, &element, &index));
        SET_CATCH_INT_PAR(index, t);
        THROW_IF(index != 2, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "vector search error, ret=%d, key=%d", ERROR_INT_PAR1(), ERROR_INT_PAR2());
    }

    return 1;
}

static int __test_vector_get_end_index(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;

    TRY {
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        ret = vector->get_end_index(vector);
        SET_CATCH_INT_PAR(ret, 0);
        THROW_IF(ret != 5, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "vector search error, ret=%d, par2=%d", ERROR_INT_PAR1(), ERROR_INT_PAR2());
    } FINALLY {
        dbg_str(DBG_SUC, "%s test suc, end index=%d", __func__, ERROR_INT_PAR1());
    }

    return 1;
}

static int __test_vector_remove_back(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;

    TRY {
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        EXEC(vector->remove_back(vector, &element));
        ret = vector->get_end_index(vector);
        SET_CATCH_INT_PAR(ret, 0);
        THROW_IF(ret != 4, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "vector search error, ret=%d, key=%d", ERROR_INT_PAR1(), ERROR_INT_PAR2());
    } FINALLY {
        dbg_str(DBG_SUC, "%s test suc, end index=%d", __func__, ERROR_INT_PAR1());
    }

    return 1;
}

static int __vector_int_cmp(void *e1, void *e2)
{
    return (int)e1 > (int)e2 ? 1 : 0;
}

static int __test_vector_sort_case1(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *expect = "[-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500]";

    TRY {
        vector->set(vector, "/Vector/value_type", &value_type);
        vector->assign(vector,  init_data);
        dbg_str(DBG_DETAIL, "vector json:%s", vector->to_json(vector));
        vector->sort(vector, 0, __vector_int_cmp);
        SET_CATCH_PTR_PAR(vector->to_json(vector), expect);
        THROW_IF(strcmp(vector->to_json(vector), expect) != 0, -1);
        dbg_str(DBG_DETAIL, "vector json:%s", vector->to_json(vector));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "test_vector_sort_case1 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return 1;
}

static int __test_vector_sort(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_sort_case1(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_filter_case1_condition(void *element, void *cond)
{
    int c = *(int *)cond;
    int e = (int)element;

    return e > c ? 1 : 0;
}

static int __test_vector_filter_case1(Vector_Test *test)
{
    Vector *vector = test->vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *expect  = "[900, 2, 3, 34, 76, 32, 43, 56, 35, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_alloc();
    int cond = 0;

    TRY {
        out = object_new(allocator, "Vector", NULL);
        out->set(out, "value_type", &value_type);

        vector->set(vector, "value_type", &value_type);
        vector->assign(vector,  init_data);
        dbg_str(DBG_DETAIL, "vector before filter:%s", vector->to_json(vector));
        EXEC(vector->filter(vector, __test_vector_filter_case1_condition, &cond, out));

        SET_CATCH_PTR_PAR(init_data, out->to_json(out));
        THROW_IF(strcmp(out->to_json(out), expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_PTR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(out);
    }

    return 1;
}

static int __test_vector_filter(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_filter_case1(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_add_vector_case1(Vector_Test *test)
{
    Vector *vector = test->vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data1  = "[900, 2, 3, -58, 34, 76, 32, 43]";
    char *init_data2  = "[56, -70, 35, -234, 532, 543, 2500]";
    char *expect  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_alloc();
    int cond = 0;

    TRY {
        out = object_new(allocator, "Vector", init_data2);
        vector->assign(vector,  init_data1);

        dbg_str(DBG_DETAIL, "vector src json:%s", out->to_json(out));
        dbg_str(DBG_DETAIL, "vector dst json:%s", vector->to_json(vector));
        EXEC(vector->add_vector(vector, out));

        SET_CATCH_PTR_PAR(expect, vector->to_json(vector));
        THROW_IF(strcmp(vector->to_json(vector), expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_PTR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(out);
    }

    return 1;
}

static int __test_vector_add_vector(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_add_vector_case1(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_vector_copy_case1(Vector_Test *test)
{
    Vector *vector = test->vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    uint8_t trustee_flag = 1;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *expect  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_alloc();
    int cond = 0;

    TRY {
        out = object_new(allocator, "Vector", NULL);
        out->set(out, "/Vector/trustee_flag", &trustee_flag);
        out->set(out, "/Vector/value_type", &value_type);

        EXEC(vector->assign(vector, init_data));
        EXEC(vector->copy(vector, out));

        SET_CATCH_PTR_PAR(expect, out->to_json(out));
        THROW_IF(strcmp(out->to_json(out), expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_PTR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(out);
    }

    return 1;
}

static int __test_vector_copy_case2(Vector_Test *test)
{
    Vector *vector = test->vector, *out = NULL;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]";
    char *expect = "[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]";
    allocator_t *allocator = allocator_get_default_alloc();
    String *json;
    int cond = 0;

    TRY {
        vector->reset(vector);
        vector->assign(vector,  init_data);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");

        out = object_new(allocator, "Vector", NULL);
        THROW_IF(out == NULL, -1);
        out->set(out, "/Vector/trustee_flag", &trustee_flag);
        out->set(out, "/Vector/value_type", &value_type);
        out->set(out, "/Vector/class_name", "Simplest_Obj");

        EXEC(vector->copy(vector, out));
        out->to_json(out);
        json = ((Obj *)out)->json;
        json->replace(json, "\t", "" , -1);
        json->replace(json, "\r", "" , -1);
        json->replace(json, "\n", "" , -1);
        json->replace(json, ", ", ",", -1);

        SET_CATCH_PTR_PAR(expect, STR2A(json));
        THROW_IF(strcmp(STR2A(json), expect) != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        TRY_SHOW_PTR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(out);
    }

    return ret;
}

static int __test_vector_copy(Vector_Test *test)
{
    int ret;

    TRY {
        EXEC(__test_vector_copy_case1(test));
        EXEC(__test_vector_copy_case2(test));
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static class_info_entry_t vector_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Vector_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Vector_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Vector_Test, set, NULL),
    Init_Vfunc_Entry(4 , Vector_Test, get, NULL),
    Init_Vfunc_Entry(5 , Vector_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Vector_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Vector_Test, test_vector_add, __test_vector_add),
    Init_Vfunc_Entry(8 , Vector_Test, test_vector_remove, __test_vector_remove),
    Init_Vfunc_Entry(9 , Vector_Test, test_vector_count, __test_vector_count),
    Init_Vfunc_Entry(10, Vector_Test, test_vector_reset, __test_vector_reset),
    Init_Vfunc_Entry(11, Vector_Test, test_vector_to_json, __test_vector_to_json),
    Init_Vfunc_Entry(12, Vector_Test, test_vector_assign, __test_vector_assign),
    Init_Vfunc_Entry(13, Vector_Test, test_vector_new, __test_vector_new),
    Init_Vfunc_Entry(14, Vector_Test, test_vector_search, __test_vector_search),
    Init_Vfunc_Entry(15, Vector_Test, test_vector_get_end_index, __test_vector_get_end_index),
    Init_Vfunc_Entry(16, Vector_Test, test_vector_remove_back, __test_vector_remove_back),
    Init_Vfunc_Entry(17, Vector_Test, test_vector_sort, __test_vector_sort),
    Init_Vfunc_Entry(18, Vector_Test, test_vector_filter, __test_vector_filter),
    Init_Vfunc_Entry(19, Vector_Test, test_vector_add_vector, __test_vector_add_vector),
    Init_Vfunc_Entry(20, Vector_Test, test_vector_copy, __test_vector_copy),
    Init_End___Entry(21, Vector_Test),
};
REGISTER_CLASS("Vector_Test", vector_test_class_info);
