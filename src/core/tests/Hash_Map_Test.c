/**
 * @file Hash_Map_Test.c
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
#include "Hash_Map_Test.h"


struct test{
    int a;
    int b;
};

static struct test *__init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

static int __construct(Hash_Map_Test *test, char *init_str)
{
    test->map = OBJECT_NEW(test->parent.obj.allocator, Hash_Map, NULL);
    return 0;
}

static int __deconstrcut(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;

    object_destroy(map);

    return 0;
}

static int __setup(Hash_Map_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    dbg_str(DBG_DETAIL,"Hash_Map_Test set up");

    return 0;
}

static int __teardown(Hash_Map_Test *test)
{
    dbg_str(DBG_DETAIL,"Hash_Map_Test teardown");

    Map *map = (Map *)test->map;

    map->reset(map);

    return 0;
}

static int __test_count(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 5;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);
    __init_test_instance(&t5, 5, 2);

    map->set_cmp_func(map, string_key_cmp_func);
    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);

    dbg_str(DBG_DETAIL, "run at here");
    ret = map->remove(map, "test2", (void **)&t);
    count = map->count(map);

    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

    map->add(map, "test2", &t2);
    count = map->count(map);
    expect_count++;
    ret = ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
    if (ret == 0) {
        dbg_str(DBG_ERROR, "count=%d, expect_count=%d", count, expect_count);
    }

    return 0;
}

static int __test_reset(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);
    __init_test_instance(&t5, 5, 2);

    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);

    map->reset(map);
    count = map->count(map);

    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

    return 0;
}

static int __test_reset_string_value(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    allocator_t *allocator = test->parent.obj.allocator;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret = 0;
    int count, expect_count = 0, expect_alloc_count;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_STRING;

    expect_alloc_count = allocator->alloc_count;

    t0 = object_new(allocator, "String", NULL);
    t1 = object_new(allocator, "String", NULL);
    t2 = object_new(allocator, "String", NULL);
    t3 = object_new(allocator, "String", NULL);
    t4 = object_new(allocator, "String", NULL);
    t5 = object_new(allocator, "String", NULL);


    dbg_str(DBG_DETAIL, "t0:%p", t0);

    t0->assign(t0, "Monday");
    t1->assign(t1, "Tuesday");
    t2->assign(t2, "Wednesday");
    t3->assign(t3, "Thursday");
    t4->assign(t4, "Friday");
    t5->assign(t5, "Saturday");

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    map->add(map, "test0", t0);
    map->add(map, "test1", t1);
    map->add(map, "test2", t2);
    map->add(map, "test3", t3);
    map->add(map, "test4", t4);
    map->add(map, "test5", t5);

    map->reset(map);
    count = map->count(map);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

    count = allocator->alloc_count;
    ASSERT_EQUAL(test, &count, &expect_alloc_count, sizeof(count));

    return 0;
}

static int __test_reset_object_value(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    allocator_t *allocator = test->parent.obj.allocator;
    Obj *obj1 = NULL;
    Obj *obj2 = NULL;
    int ret = 0, help;
    int count, expect_count = 0, expect_alloc_count;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    expect_alloc_count = allocator->alloc_count;

    obj1 = object_new(allocator, "Simplest_Obj", NULL);
    obj2 = object_new(allocator, "Simplest_Obj", NULL);

    help = 1;
    obj1->set(obj1, "/Simplest_Obj/help", &help);
    obj1->set(obj1, "/Simplest_Obj/name", "simplest obj1");

    help = 2;
    obj2->set(obj2, "/Simplest_Obj/help", &help);
    obj2->set(obj2, "/Simplest_Obj/name", "simplest obj2");

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);

    map->add(map, "test0", obj1);
    map->add(map, "test1", obj2);

    map->reset(map);
    count = map->count(map);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

    count = allocator->alloc_count;
    ASSERT_EQUAL(test, &count, &expect_alloc_count, sizeof(count));

    return 1;
}

static int __test_add(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    struct test t0;
    int ret = 0;
    int count, expect_count = 1;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->add(map, "test0", &t0);

    count = map->count(map);

    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

}

static int __test_remove(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 5;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);
    __init_test_instance(&t5, 5, 2);

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set_cmp_func(map, string_key_cmp_func);
    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);

    dbg_str(DBG_DETAIL, "run at here");
    ret = map->remove(map, "test2", (void **)&t);
    count = map->count(map);

    ASSERT_EQUAL(test, t, &t2, sizeof(t2));
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
    return 0;
}

static int __test_search_string_key(Hash_Map_Test *test)
{
    Iterator *iter, *next, *prev;
    Map *map = (Map *)test->map;
    allocator_t *allocator = allocator_get_default_alloc();
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);
    __init_test_instance(&t5, 5, 2);

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set_cmp_func(map, string_key_cmp_func);
    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);

    dbg_str(DBG_DETAIL, "test search:");
    map->search(map, "test2", (void **)&t);
    dbg_str(DBG_DETAIL, "new search test2: t->a=%d, t->b=%d", t->a, t->b);
    dbg_buf(DBG_DETAIL, "t:", (void *)t, sizeof(t2));
    dbg_buf(DBG_DETAIL, "t2:", (void *)&t2, sizeof(t2));
    ASSERT_EQUAL(test, t, &t2, sizeof(t2));

    return 1;
}

static int __test_search_all_string_key(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);
    __init_test_instance(&t5, 5, 2);

    list = OBJECT_NEW(allocator, Linked_List, NULL);

    map->set_cmp_func(map, string_key_cmp_func);
    map->set(map, "/Map/trustee_flag", &trustee_flag);

    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);

    map->search_all_same_key(map, "test2", list);

    if (list->count(list) == 4) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(list);

    return ret;
}

static int __test_search_int_key(Hash_Map_Test *test)
{
    Iterator *iter, *next, *prev;
    Map *map = (Map *)test->map;
    allocator_t *allocator = allocator_get_default_alloc();
    int key0, key1, key2, key3, key4;
    int ret = 0;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 3, 2);
    __init_test_instance(&t3, 4, 2);
    __init_test_instance(&t4, 5, 2);

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set_cmp_func(map, default_key_cmp_func);

    key0 = 0;
    map->add(map, key0, &t0);
    key1 = 1;
    map->add(map, key1, &t1);
    key2 = 2;
    map->add(map, key2, &t2);
    key3 = 3;
    map->add(map, key3, &t3);

    printf("key0:%p, key1:%p, key2:%p, key3:%p\n", 
           &key0, &key1, &key2, &key3);
    
    key4 = 1;
    ret = map->search(map, key4, (void **)&t);
    if (ret == 1) {
        dbg_str(DBG_SUC, "found key =%d", key4);
    } else {
        goto end;
    }

    key4 = 3;
    ret = map->search(map, key4, (void **)&t);
    if (ret == 1) {
        dbg_str(DBG_SUC, "found key =%d", key4);
    } else {
        goto end;
    }

    key4 = 7;
    ret = map->search(map, key4, (void **)&t);
    if (ret == 1) {
        ret = 0;
        goto end;
    } else {
        ret = 1;
        goto end;
    }

end:

    return ret;
}

static int __test_search_all_int_key(Hash_Map_Test *test)
{
    Map *map = (Map *)test->map;
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    int key0, key1, key2, key3, key4, key5, key6, key7;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int trustee_flag = 0;

    __init_test_instance(&t0, 0, 2);
    __init_test_instance(&t1, 1, 2);
    __init_test_instance(&t2, 2, 2);
    __init_test_instance(&t3, 3, 2);
    __init_test_instance(&t4, 4, 2);

    list = OBJECT_NEW(allocator, Linked_List, NULL);

    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set_cmp_func(map, default_key_cmp_func);

    map->add(map, 0, &t0);
    map->add(map, 1, &t1);
    map->add(map, 1, &t1);
    map->add(map, 1, &t1);
    map->add(map, 4, &t2);
    map->add(map, 5, &t4);

    map->search_all_same_key(map, 1, list);

    if (list->count(list) == 3) {
        ret = 1;
    } else {
        dbg_str(DBG_DETAIL, "list count =%d", list->count(list));
        ret = 0;
    }

    object_destroy(list);

    return ret;
}

static class_info_entry_t hash_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Hash_Map_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Hash_Map_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Hash_Map_Test, set, NULL),
    Init_Vfunc_Entry(4 , Hash_Map_Test, get, NULL),
    Init_Vfunc_Entry(5 , Hash_Map_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Hash_Map_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Hash_Map_Test, test_count, __test_count),
    Init_Vfunc_Entry(8 , Hash_Map_Test, test_reset, __test_reset),
    Init_Vfunc_Entry(9 , Hash_Map_Test, test_reset_string_value, __test_reset_string_value),
    Init_Vfunc_Entry(10, Hash_Map_Test, test_reset_object_value, __test_reset_object_value),
    Init_Vfunc_Entry(11, Hash_Map_Test, test_add, __test_add),
    Init_Vfunc_Entry(12, Hash_Map_Test, test_remove, __test_remove),
    Init_Vfunc_Entry(13, Hash_Map_Test, test_search_string_key, __test_search_string_key),
    Init_Vfunc_Entry(14, Hash_Map_Test, test_search_int_key, __test_search_int_key),
    /*
     *Init_Vfunc_Entry(14, Hash_Map_Test, test_search_all_string_key, __test_search_all_string_key),
     *Init_Vfunc_Entry(16, Hash_Map_Test, test_search_all_int_key, __test_search_all_int_key),
     */
    Init_End___Entry(15, Hash_Map_Test),
};
REGISTER_CLASS("Hash_Map_Test", hash_test_class_info);
