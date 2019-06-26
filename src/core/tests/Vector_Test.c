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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/tests/Vector_Test.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/tests/Simplest_Test_Obj.h>

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

    return 0;
}

static int __teardown(Vector_Test *test)
{
    dbg_str(DBG_DETAIL,"Vector_Test teardown");
    test->vector->clear(test->vector);

    return 0;
}

static int __test_int_vector_add(Vector_Test *test)
{
    Vector *vector = test->vector;
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret;

    Init_Test_Case(test);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->reconstruct(vector);

    vector->add_at(vector, 0, 1);
    vector->remove(vector, 0, (void **)&t);

    if ((int)t != 1) {
        dbg_str(DBG_ERROR,"r=%d, expect=%d", (int)t, 1);
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_int_vector_remove(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret;

    Init_Test_Case(test);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->remove(vector, 2, (void **)&t);
    vector->peek_at(vector, 2, (void **)&t);

    if ((int)t != 3) {
        dbg_str(DBG_ERROR,"r=%d, expect=%d", (int)t, 3);
        ret = 0;
    } else {
        ret = 1;
    }
    return ret;
}

static int __test_int_vector_count(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret, count;

    Init_Test_Case(test);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    count = vector->count(vector);

    if (count != 5) {
        dbg_str(DBG_ERROR,"r=%d, expect=%d", count, 5);
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_int_vector_clear(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret, count;

    Init_Test_Case(test);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->clear(vector);
    count = vector->count(vector);

    if (count != 0) {
        dbg_str(DBG_ERROR,"r=%d, expect=%d", count, 0);
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_int_vector_to_json(Vector_Test *test)
{
    Vector *vector = test->vector;
    int value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *result = "[0, 1, 2, 3, 4, 5]";

    Init_Test_Case(test);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->add_at(vector, 5, 5);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json((Obj *)vector));
    if (strcmp(result, vector->to_json((Obj *)vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_string_vector_to_json(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret;
    uint8_t trustee_flag = 1;
    char result[1024];

    Init_Test_Case(test);

    t0 = object_new(allocator, "String", NULL);
    t1 = object_new(allocator, "String", NULL);
    t2 = object_new(allocator, "String", NULL);
    t3 = object_new(allocator, "String", NULL);
    t4 = object_new(allocator, "String", NULL);
    t5 = object_new(allocator, "String", NULL);

    t0->assign(t0, "Monday");
    t1->assign(t1, "Tuesday");
    t2->assign(t2, "Wednesday");
    t3->assign(t3, "Thursday");
    t4->assign(t4, "Friday");
    t5->assign(t5, "Saturday");

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

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json((Obj*)vector));
    dbg_str(DBG_DETAIL, "result: %s", result);

    if (strcmp(result, vector->to_json((Obj*)vector)) != 0) {
        ret = 1;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_obj_vector_to_json(Vector_Test *test)
{
    int ret = 0, help = 0;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Vector *vector = test->vector;
    uint8_t trustee_flag = 1;
    Obj *obj0 = NULL;
    Obj *obj1 = NULL;
    Obj *obj2 = NULL;

    Init_Test_Case(test);
    obj0 = object_new(allocator, "Simplest_Test_Obj", NULL);
    obj1 = object_new(allocator, "Simplest_Test_Obj", NULL);
    obj2 = object_new(allocator, "Simplest_Test_Obj", NULL);

    help = 0;
    obj0->set(obj0, "/Simplest_Test_Obj/help", &help);
    obj0->set(obj0, "/Simplest_Test_Obj/option", "test obj0 option");

    help = 1;
    obj1->set(obj1, "/Simplest_Test_Obj/help", &help);
    obj1->set(obj1, "/Simplest_Test_Obj/option", "test obj1 option");

    help = 2;
    obj2->set(obj2, "/Simplest_Test_Obj/help", &help);
    obj2->set(obj2, "/Simplest_Test_Obj/option", "test obj2 option");

    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/class_name", "Simplest_Test_Obj");
    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

    vector->add(vector, obj0);
    vector->add(vector, obj1);
    vector->add(vector, obj2);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json((Obj*)vector));

    ret = 1;

    return ret;

}
static int __test_int_vector_set_init_data(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *init_data = "[0, 1, 2, 3, 4, 5]";

    Init_Test_Case(test);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json((Obj *)vector));
    if (strcmp(init_data, vector->to_json((Obj *)vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;

}

static int __test_string_vector_set_init_data(Vector_Test *test)
{
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    uint8_t trustee_flag = 1;
    int ret;
    char *init_data = "[\"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";

    Init_Test_Case(test);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json((Obj *)vector));
    if (strcmp(init_data, vector->to_json((Obj *)vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;

}

static int __test_obj_vector_set_init_data(Vector_Test *test)
{
    int ret;
    Vector *vector = test->vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int capacity = 19, value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    char *init_data = "[\
        {\
            \"option\": \"test cmd0 option\",\
            \"help\": 0\
        }, {\
            \"option\": \"test cmd1 option\",\
            \"help\": 1\
        }, {\
            \"option\": \"test cmd2 option\",\
            \"help\": 2\
        }]";

    Init_Test_Case(test);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->set(vector, "/Vector/class_name", "Simplest_Test_Obj");
    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector json: %s", vector->to_json((Obj *)vector));
    printf("Vector json: %s\n", vector->to_json((Obj *)vector));
    if (strcmp(init_data, vector->to_json((Obj *)vector)) != 0) {
        ret = 1;
    } else {
        ret = 1;
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
    Init_Vfunc_Entry(7 , Vector_Test, test_int_vector_add, __test_int_vector_add),
    Init_Vfunc_Entry(8 , Vector_Test, test_int_vector_remove, __test_int_vector_remove),
    Init_Vfunc_Entry(9 , Vector_Test, test_int_vector_count, __test_int_vector_count),
    Init_Vfunc_Entry(10, Vector_Test, test_int_vector_clear, __test_int_vector_clear),
    Init_Vfunc_Entry(11, Vector_Test, test_int_vector_to_json, __test_int_vector_to_json),
    Init_Vfunc_Entry(12, Vector_Test, test_string_vector_to_json, __test_string_vector_to_json),
    Init_Vfunc_Entry(13, Vector_Test, test_obj_vector_to_json, __test_obj_vector_to_json),
    Init_Vfunc_Entry(14, Vector_Test, test_int_vector_set_init_data, __test_int_vector_set_init_data),
    Init_Vfunc_Entry(15, Vector_Test, test_string_vector_set_init_data, __test_string_vector_set_init_data),
    Init_Vfunc_Entry(16, Vector_Test, test_obj_vector_set_init_data, __test_obj_vector_set_init_data),
    Init_End___Entry(17, Vector_Test),
};
REGISTER_CLASS("Vector_Test", vector_test_class_info);
