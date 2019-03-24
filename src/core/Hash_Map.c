/**
 * @file map.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/config.h>
#include <libobject/core/hash_map.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Map *map, char *init_str)
{
    Hash_Map *h = (Hash_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    if (h->bucket_size == 0) { h->bucket_size = 20; }

    h->hmap = hash_map_alloc(allocator);

    hash_map_init(h->hmap);


    map->b = OBJECT_NEW(allocator, Hmap_Iterator, NULL);
    map->e = OBJECT_NEW(allocator, Hmap_Iterator, NULL);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(HMAP_DETAIL, "hash map deconstruct, map addr:%p", map);
    object_destroy(map->b);
    object_destroy(map->e);
    hash_map_destroy(((Hash_Map *)map)->hmap);

    return 0;
}

static int __add(Map *map, void *key, void *value)
{
    dbg_str(HMAP_DETAIL, "Hash Map add");
    Hash_Map *hmap = (Hash_Map *)map;

    if (hmap->key_type) {
        hmap->hmap->key_type = hmap->key_type;
    }

    return hash_map_insert(hmap->hmap, key, value);
}

static int __search(Map *map, void *key, void **element)
{
    hash_map_pos_t pos;
    int ret;

    dbg_str(HMAP_IMPORTANT, "Hash Map search");
    ret = hash_map_search(((Hash_Map *)map)->hmap, key, &pos);
    if (ret == 1 && element != NULL) {
        *element = hash_map_pos_get_pointer(&pos);
        return ret;
    }

    return ret;
}

static int __remove(Map *map, void *key, void **element)
{
    hash_map_pos_t pos;
    int ret;

    dbg_str(HMAP_IMPORTANT, "Hash Map remove");
    ret = hash_map_search(((Hash_Map *)map)->hmap, key, &pos);
    if (ret == 1 && element != NULL) {
        *element = hash_map_pos_get_pointer(&pos);
        ret = hash_map_delete(((Hash_Map *)map)->hmap, &pos);
        return ret;
    } else if (ret == 1) {
        ret = hash_map_delete(((Hash_Map *)map)->hmap, &pos);
    } else {
        dbg_str(HMAP_WARNNING, "map remove, not found key :%s", key);
    }

    return ret;
}

static int __del(Map *map, void *key)
{
    hash_map_pos_t pos;
    int ret = -1;
    dbg_str(HMAP_DETAIL, "Hash Map del");

    ret = hash_map_search(((Hash_Map *)map)->hmap, key, &pos);
    if (ret == 1) {
        ret = hash_map_delete(((Hash_Map *)map)->hmap, &pos);
    }

    return ret;
}

static Iterator *__begin(Map *map)
{
    Hmap_Iterator *iter = (Hmap_Iterator *)map->b;

    dbg_str(HMAP_DETAIL, "Hash Map begin");

    hash_map_begin(((Hash_Map *)map)->hmap, &iter->hash_map_pos);

    return (Iterator *)iter;
}

static Iterator *__end(Map *map)
{
    Hmap_Iterator *iter = (Hmap_Iterator *)map->e;

    dbg_str(HMAP_DETAIL, "Hash Map end");

    hash_map_end(((Hash_Map *)map)->hmap, &iter->hash_map_pos);

    return (Iterator *)iter;
}

static class_info_entry_t hash_map_class_info[] = {
    Init_Obj___Entry(0 , Map, map),
    Init_Nfunc_Entry(1 , Hash_Map, construct, __construct),
    Init_Nfunc_Entry(2 , Hash_Map, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Hash_Map, add, __add),
    Init_Vfunc_Entry(4 , Hash_Map, search, __search),
    Init_Vfunc_Entry(5 , Hash_Map, remove, __remove),
    Init_Vfunc_Entry(6 , Hash_Map, del, __del),
    Init_Vfunc_Entry(7 , Hash_Map, begin, __begin),
    Init_Vfunc_Entry(8 , Hash_Map, end, __end),
    Init_Vfunc_Entry(9 , Hash_Map, for_each, NULL),
    Init_U16___Entry(10, Hash_Map, key_size, NULL),
    Init_U16___Entry(11, Hash_Map, value_size, NULL),
    Init_U16___Entry(12, Hash_Map, bucket_size, NULL),
    Init_U8____Entry(13, Hash_Map, key_type, NULL),
    Init_Point_Entry(14, Hash_Map, test, NULL),
    Init_End___Entry(15),
};
REGISTER_CLASS("Hash_Map", hash_map_class_info);

struct test{
    int a;
    int b;
};
static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

static void hash_map_print(void *key, void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(HMAP_DETAIL, "key:%s t->a=%d, t->b=%d", key, t->a, t->b);
}

static void hash_map_print_with_numeric_key(void *key, void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(HMAP_DETAIL, "key:%d t->a=%d, t->b=%d", *(int *)key, t->a, t->b);
}


static int test_obj_hash_map_string_key(TEST_ENTRY *entry)
{
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(DBG_SUC, "hash_map test begin alloc count =%d", allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "40") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "10") ;

    map  = OBJECT_NEW(allocator, Hash_Map, c->buf);

    /*
     *object_dump(map, "Hash_Map", buf, 2048);
     *dbg_str(DBG_DETAIL, "Map dump: %s", buf);
     */

    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);
    map->for_each(map, hash_map_print);

    dbg_str(DBG_DETAIL, "test search:");
    map->search(map, "test2", (void **)&t);
    dbg_str(DBG_DETAIL, "new search test2: t->a=%d, t->b=%d", t->a, t->b);
    ret = assert_equal(t, &t2, sizeof(struct test));
    if (ret == 0) {
        dbg_str(DBG_WARNNING, "hash map search error");
        return ret;
    }

    dbg_str(DBG_DETAIL, "test remove:");
    map->remove(map, "test2", (void **)&t);
    dbg_str(DBG_DETAIL, "remove test2: t->a=%d, t->b=%d", t->a, t->b);
    ret = assert_equal(t, &t2, sizeof(struct test));
    if (ret == 0) {
        dbg_str(DBG_WARNNING, "hash map remove error");
        return ret;
    }

    map->for_each(map, hash_map_print);

    object_destroy(map);

    cfg_destroy(c);

    dbg_str(DBG_SUC, "hash_map test end alloc count =%d", allocator->alloc_count);

    return ret;

}
REGISTER_TEST_FUNC(test_obj_hash_map_string_key);

static int test_obj_hash_map_numeric_key(TEST_ENTRY *entry)
{
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    int key;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);

    dbg_str(DBG_SUC, "hash_map test begin alloc count =%d", allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "4") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "25") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "10") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_type", "1");

    /*
     *dbg_str(DBG_SUC, "config:%s", c->buf);
     */

    map  = OBJECT_NEW(allocator, Hash_Map, c->buf);

    /*
     *object_dump(map, "Hash_Map", buf, 2048);
     *dbg_str(DBG_DETAIL, "Map dump: %s", buf);
     */

    dbg_str(DBG_DETAIL, "test hashmap add:");
    key = 0;
    map->add(map, &key, &t0);
    key = 1;
    map->add(map, &key, &t1);

    /*
     *printf("for_each\n");
     *map->for_each(map, hash_map_print_with_numeric_key);
     */

    dbg_str(DBG_DETAIL, "test hashmap search:");
    ret = map->search(map, &key, (void **)&t);
    dbg_str(DBG_DETAIL, "search ret=%d", ret);
    dbg_str(DBG_DETAIL, "search key=%d, t->a=%d, t->b=%d", key, t->a, t->b);
    ret = assert_equal(t, &t1, sizeof(struct test));
    if (ret == 0) {
        dbg_str(DBG_WARNNING, "hash map search error");
        return ret;
    }

    dbg_str(DBG_DETAIL, "test hashmap del:");
    map->del(map, &key);

    /*
     *printf("for_each\n");
     *map->for_each(map, hash_map_print_with_numeric_key);
     */

    object_destroy(map);
    cfg_destroy(c);

    dbg_str(DBG_SUC, "hash_map test end alloc count =%d", allocator->alloc_count);

    return ret;

}
REGISTER_TEST_FUNC(test_obj_hash_map_numeric_key);

static int test_Hash_Map_set(TEST_ENTRY *entry)
{
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    int ret = 0;
    char buf[2048] = {0};

    map  = OBJECT_NEW(allocator, Hash_Map, NULL);

    map->set(map, "Hash_Map/test", 0x12345678);

    void **addr = map->get(map, "Hash_Map/test");

    if (*addr == 0x12345678) {
        ret = 1;
    }
    object_dump(map, "Hash_Map", buf, 2048);
    dbg_str(DBG_DETAIL, "Map dump: %s", buf);

    object_destroy(map);

    return ret;

}
REGISTER_TEST_FUNC(test_Hash_Map_set);
