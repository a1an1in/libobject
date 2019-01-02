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
#include <libobject/core/utils/config/config.h>
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

static int __set(Map *m, char *attrib, void *value)
{
    Hash_Map *map = (Hash_Map *)m;

    if (strcmp(attrib, "set") == 0) {
        map->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        map->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        map->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        map->deconstruct = value;
    } else if (strcmp(attrib, "name") == 0) {
        strncpy(map->name, value, strlen(value));
    } else if (strcmp(attrib, "add") == 0) {
        map->add = value;
    } else if (strcmp(attrib, "search") == 0) {
        map->search = value;
    } else if (strcmp(attrib, "remove") == 0) {
        map->remove = value;
    } else if (strcmp(attrib, "del") == 0) {
        map->del = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        map->for_each = value;
    } else if (strcmp(attrib, "begin") == 0) {
        map->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        map->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        map->destroy = value;
    } else if (strcmp(attrib, "key_size") == 0) {
        map->key_size = *((uint16_t *)value);
    } else if (strcmp(attrib, "value_size") == 0) {
        map->value_size = *((uint16_t *)value);
    } else if (strcmp(attrib, "bucket_size") == 0) {
        map->bucket_size = *((uint16_t *)value);
    } else if (strcmp(attrib, "key_type") == 0) {
        map->key_type = *((uint8_t *)value);
    } else {
        dbg_str(HMAP_WARNNING, "map set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Map *obj, char *attrib)
{
    Hash_Map *map = (Hash_Map *)obj;

    if (strcmp(attrib, "name") == 0) {
        return map->name;
    } else if (strcmp(attrib, "key_size") == 0) {
        return &map->key_size;
    } else if (strcmp(attrib, "value_size") == 0) {
        return &map->value_size;
    } else if (strcmp(attrib, "bucket_size") == 0) {
        return &map->bucket_size;
    } else if (strcmp(attrib, "key_type") == 0) {
        return &map->key_type;
    } else {
        dbg_str(HMAP_WARNNING, "hash map get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }

    return NULL;
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
    [0 ] = {ENTRY_TYPE_OBJ, "Map", "map", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add", __add, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "search", __search, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove", __remove, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "del", __del, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "begin", __begin, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "end", __end, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_UINT16_T, "", "key_size", NULL, sizeof(short)}, 
    [13] = {ENTRY_TYPE_UINT16_T, "", "value_size", NULL, sizeof(short)}, 
    [14] = {ENTRY_TYPE_UINT16_T, "", "bucket_size", NULL, sizeof(short)}, 
    [15] = {ENTRY_TYPE_UINT8_T, "", "key_type", NULL, sizeof(short)}, 
    [16] = {ENTRY_TYPE_END}, 
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
