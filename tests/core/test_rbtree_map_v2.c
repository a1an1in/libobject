/**
 * @file RBTree_Map_Test.c
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
#include <libobject/core/Linked_List.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/Map.h>
#include <libobject/net/bus/busd.h>
#include <libobject/mockery/mockery.h>

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

static int test_rbtree_map_v2_count(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 5;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

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

        THROW_IF(count != expect_count, -1);

        map->add(map, "test2", &t2);
        count = map->count(map);
        expect_count++;
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_count);

static int test_rbtree_map_v2_reset(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

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

        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_reset);

static int test_rbtree_map_v2_reset_string_value(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret = 0;
    int count, expect_count = 0, expect_alloc_count;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_STRING_POINTER;

    TRY {
        sleep(2);
        map = object_new(allocator, "RBTree_Map", NULL);
        expect_alloc_count = allocator->alloc_count;

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
        THROW_IF(count != expect_count, -1);
        count = allocator->alloc_count;
        THROW_IF(count != expect_alloc_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_reset_string_value);

static int test_rbtree_map_v2_reset_object_value(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    Obj *obj1 = NULL;
    Obj *obj2 = NULL;
    int ret = 0, help;
    int count, expect_count = 0, expect_alloc_count;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);
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
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_reset_object_value);

static int test_rbtree_map_v2_add(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct test t0;
    int ret = 0;
    int count, expect_count = 1;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);
        __init_test_instance(&t0, 0, 2);

        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->add(map, "test0", &t0);
        count = map->count(map);

        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_add);

static int test_rbtree_map_v2_remove(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int count, expect_count = 5;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);
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

        THROW_IF(memcmp(&t2, t, sizeof(t2)) != 0, -1);
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_remove);

static int test_rbtree_map_v2_search_string_key(TEST_ENTRY *entry)
{
    Iterator *iter, *next, *prev;
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

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
        THROW_IF(memcmp(&t2, t, sizeof(t2)) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_search_string_key);

static int test_rbtree_map_v2_search_all_string_key(TEST_ENTRY *entry)
{
    List *list;
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);
        
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
        THROW_IF(list->count(list) != 4, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_search_all_string_key);

static int test_rbtree_map_v2_search_int_key(TEST_ENTRY *entry)
{
    Iterator *iter, *next, *prev;
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    int key0, key1, key2, key3, key4;
    int ret = 0;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

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

        printf("key0:%p, key1:%p, key2:%p, key3:%p\n",  &key0, &key1, &key2, &key3);
        
        key4 = 1;
        ret = map->search(map, key4, (void **)&t);
        THROW_IF(ret != 1, -1);

        key4 = 3;
        ret = map->search(map, key4, (void **)&t);
        THROW_IF(ret != 1, -1);

        key4 = 7;
        ret = map->search(map, key4, (void **)&t);
        THROW_IF(ret == 1, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_search_int_key);

static int test_rbtree_map_v2_search_all_int_key(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    List *list;
    int key0, key1, key2, key3, key4, key5, key6, key7;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int trustee_flag = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

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
        THROW_IF(list->count(list) != 3, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_search_all_int_key);

static int test_rbtree_map_v2_to_json_for_string(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    char *json;
    int count, expect_count = 0, expect_alloc_count;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_STRING_POINTER;
    char *expect = "[\"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
    int ret = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);
        expect_alloc_count = allocator->alloc_count;

        t0 = object_new(allocator, "String", "Monday");
        t1 = object_new(allocator, "String", "Tuesday");
        t2 = object_new(allocator, "String", "Wednesday");
        t3 = object_new(allocator, "String", "Thursday");
        t4 = object_new(allocator, "String", "Friday");
        t5 = object_new(allocator, "String", "Saturday");

        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->set(map, "/Map/value_type", &value_type);
        map->add(map, "test0", t0);
        map->add(map, "test1", t1);
        map->add(map, "test2", t2);
        map->add(map, "test3", t3);
        map->add(map, "test4", t4);
        map->add(map, "test5", t5);
        
        json = map->to_json(map);
        dbg_str(DBG_VIP, "map to json:%s", json);
        THROW_IF(strcmp(json, expect) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_to_json_for_string);

static int test_rbtree_map_v2_to_json_for_struct(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Map *map;
    struct busd_object t1, t2, t3, t4, t5;
    int count, expect_count = 0, expect_alloc_count;
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    String *s1, *s2;
    char *expect = "[{\"id\":\"1\",\"infos\":\"hello1\",\"fd\":2},\
                     {\"id\":\"2\",\"infos\":\"hello2\",\"fd\":3},\
                     {\"id\":\"3\",\"infos\":\"hello3\",\"fd\":4},\
                     {\"id\":\"4\",\"infos\":\"hello4\",\"fd\":5},\
                     {\"id\":\"5\",\"infos\":\"hello5\",\"fd\":6}]";
    int ret = 0;

    TRY {
        map = object_new(allocator, "RBTree_Map", NULL);

        t1.id[0] = '1', t1.fd = 1, t1.infos = "hello1";
        t2.id[0] = '2', t2.fd = 2, t2.infos = "hello2";
        t3.id[0] = '3', t3.fd = 3, t3.infos = "hello3";
        t4.id[0] = '4', t4.fd = 4, t4.infos = "hello4";
        t5.id[0] = '5', t5.fd = 5, t5.infos = "hello5";

        map->set(map, "/Map/value_type", &value_type);
        map->set(map, "/Map/class_name", "Busd_Object_Struct_Adapter");
        map->add(map, "test1", &t1);
        map->add(map, "test2", &t2);
        map->add(map, "test3", &t3);
        map->add(map, "test4", &t4);
        map->add(map, "test5", &t5);

        s1 = object_new(allocator, "String", map->to_json(map));
        s1->replace(s1, "\t", "", -1);
        s1->replace(s1, "\r", "", -1);
        s1->replace(s1, "\n", "", -1);

        s2 = object_new(allocator, "String", map->to_json(map));
        s2->replace(s2, "\t", "", -1);
        s2->replace(s2, "\r", "", -1);
        s2->replace(s2, "\n", "", -1);

        SET_CATCH_STR_PARS(STR2A(s1), STR2A(s2));
        THROW_IF(strcmp(STR2A(s1), STR2A(s2)) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_rbtree_map_v2_to_json_for_struct error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(s1);
        object_destroy(s2);
        object_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v2_to_json_for_struct);