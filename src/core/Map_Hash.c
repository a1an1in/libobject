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
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/map_hash.h>
#include <libobject/utils/config/config.h>

static int __construct(Map *map,char *init_str)
{
    Hash_Map *h = (Hash_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    if (h->key_size == 0)    { h->key_size = 10;    }
    if (h->value_size == 0)  { h->value_size = 100; }
    if (h->bucket_size == 0) { h->bucket_size = 20; }

    dbg_str(DBG_DETAIL,"hash map construct, key_size=%d,value_size=%d,bucket_size=%d",
            h->key_size,h->value_size,h->bucket_size);

    h->hmap = hash_map_alloc(allocator);

    hash_map_init(h->hmap,
                  h->key_size,//uint32_t key_size,
                  h->value_size,
                  h->bucket_size);


    map->b = OBJECT_NEW(allocator, Hmap_Iterator,NULL);
    map->e = OBJECT_NEW(allocator, Hmap_Iterator,NULL);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(OBJ_DETAIL,"hash map deconstruct,map addr:%p",map);
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
        strncpy(map->name,value,strlen(value));
    } else if (strcmp(attrib, "insert") == 0) {
        map->insert = value;
    } else if (strcmp(attrib, "insert_wb") == 0) {
        map->insert_wb = value;
    } else if (strcmp(attrib, "search") == 0) {
        map->search = value;
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
        dbg_str(OBJ_WARNNING,"map set, not support %s setting",attrib);
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
        dbg_str(OBJ_WARNNING,"hash map get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __insert(Map *map,void *key,void *value)
{
    dbg_str(OBJ_DETAIL,"Hash Map insert");
    Hash_Map *hmap = (Hash_Map *)map;

    if (hmap->key_type) {
        hmap->hmap->key_type = hmap->key_type;
    }
    return hash_map_insert(hmap->hmap,key,value);
}

static int __insert_wb(Map *map,void *key,void *value,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"Hash Map insert wb");
    return hash_map_insert_wb(((Hash_Map *)map)->hmap,key,value,
                              &((Hmap_Iterator *)iter)->hash_map_pos);
}

static int __search(Map *map,void *key,Iterator *iter)
{
    dbg_str(OBJ_IMPORTANT,"Hash Map search");
    return hash_map_search(((Hash_Map *)map)->hmap, key,
                           &((Hmap_Iterator *)iter)->hash_map_pos);
}

static int __del(Map *map,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"Hash Map del");
    return hash_map_delete(((Hash_Map *)map)->hmap,
                            &((Hmap_Iterator *)iter)->hash_map_pos);
}

static Iterator *__begin(Map *map)
{
    Hmap_Iterator *iter = (Hmap_Iterator *)map->b;

    dbg_str(OBJ_DETAIL,"Hash Map begin");

    hash_map_begin(((Hash_Map *)map)->hmap, &iter->hash_map_pos);

    return (Iterator *)iter;
}

static Iterator *__end(Map *map)
{
    Hmap_Iterator *iter = (Hmap_Iterator *)map->e;

    dbg_str(OBJ_DETAIL,"Hash Map end");

    hash_map_end(((Hash_Map *)map)->hmap, &iter->hash_map_pos);

    return (Iterator *)iter;
}

static class_info_entry_t hash_map_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Map","map",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","insert",__insert,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","insert_wb",__insert_wb,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","search",__search,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del",__del,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","begin",__begin,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","end",__end,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_UINT16_T,"","key_size",NULL,sizeof(short)},
    [13] = {ENTRY_TYPE_UINT16_T,"","value_size",NULL,sizeof(short)},
    [14] = {ENTRY_TYPE_UINT16_T,"","bucket_size",NULL,sizeof(short)},
    [15] = {ENTRY_TYPE_UINT8_T,"","key_type",NULL,sizeof(short)},
    [16] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Hash_Map",hash_map_class_info);

struct test{
    int a;
    int b;
};
static struct test *genearte_test_instance(allocator_t *allocator, int a, int b)
{
    struct test *t;

    t = allocator_mem_alloc(allocator, sizeof(struct test));
    t->a = a;
    t->b = b;

    return t;
}

static void hash_map_print(Iterator *iter)
{
    Hmap_Iterator *i = (Hmap_Iterator *)iter;
    struct test *t;
     
    t = i->get_vpointer(iter);
    dbg_str(DBG_DETAIL,"key:%s t->a=%d, t->b=%d",i->get_kpointer(iter), t->a, t->b);
}

void test_obj_hash_map_string_key()
{
    Iterator *iter, *next,*prev;
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    struct test *t, *t0, *t1, *t2, *t3, *t4, *t5;

    t0 = genearte_test_instance(allocator, 0, 2);
    t1 = genearte_test_instance(allocator, 1, 2);
    t2 = genearte_test_instance(allocator, 2, 2);
    t3 = genearte_test_instance(allocator, 3, 2);
    t4 = genearte_test_instance(allocator, 4, 2);
    t5 = genearte_test_instance(allocator, 5, 2);

    dbg_str(DBG_SUC, "hash_map test begin alloc count =%d",allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "40") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "10") ;

    map  = OBJECT_NEW(allocator, Hash_Map,c->buf);
    iter = OBJECT_NEW(allocator, Hmap_Iterator,NULL);

    object_dump(map, "Hash_Map", buf, 2048);
    dbg_str(DBG_DETAIL,"Map dump: %s",buf);

    map->insert(map,"test0", t0);
    map->insert(map,"test1", t1);
    map->insert(map,"test2", t2);
    map->insert(map,"test3", t3);
    map->insert(map,"test4", t4);
    map->insert(map,"test5", t5);
    map->for_each(map,hash_map_print);

    map->search(map,"test2",iter);
    t = iter->get_vpointer(iter);
    dbg_str(DBG_DETAIL,"search test2: t->a=%d, t->b=%d", t->a, t->b);
    map->del(map,iter);

    map->for_each(map,hash_map_print);

    object_destroy(map);
    object_destroy(iter);

    cfg_destroy(c);

    dbg_str(DBG_SUC, "hash_map test end alloc count =%d",allocator->alloc_count);

}

static void hash_map_print_with_numeric_key(Iterator *iter)
{
    struct test *t;
    Hmap_Iterator *i = (Hmap_Iterator *)iter;
    int *key = (int *)i->get_kpointer(iter);

    t = i->get_vpointer(iter);
    dbg_str(DBG_DETAIL,"key:%d test->a=%d, t->b=%d", *key, t->a, t->b);
}

void test_obj_hash_map_numeric_key()
{
    Iterator *iter, *next,*prev;
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    int key;
    int ret;
    struct test *t, *t0, *t1, *t2, *t3, *t4, *t5;

    t0 = genearte_test_instance(allocator, 0, 2);
    t1 = genearte_test_instance(allocator, 1, 2);
    /*
     *t2 = genearte_test_instance(allocator, 2, 2);
     *t3 = genearte_test_instance(allocator, 3, 2);
     *t4 = genearte_test_instance(allocator, 4, 2);
     *t5 = genearte_test_instance(allocator, 5, 2);
     */

    dbg_str(DBG_SUC, "hash_map test begin alloc count =%d",allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "4") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "25") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "10") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_type", "1");

    map  = OBJECT_NEW(allocator, Hash_Map,c->buf);
    iter = OBJECT_NEW(allocator, Hmap_Iterator,NULL);

    object_dump(map, "Hash_Map", buf, 2048);
    dbg_str(DBG_DETAIL,"Map dump: %s",buf);

    printf("insert\n");
    key = 0;
    map->insert(map, &key,t0);
    key = 1;
    map->insert(map, &key,t1);

    printf("for_each\n");
    map->for_each(map,hash_map_print_with_numeric_key);

    printf("search\n");
    ret = map->search(map, &key, iter);
    t = iter->get_vpointer(iter);
    dbg_str(DBG_DETAIL,"search ret=%d",ret);
    dbg_str(DBG_DETAIL,"search key=%d, t->a=%d, t->b=%d",key, t->a, t->b);

    printf("del\n");
    map->del(map,iter);

    printf("for_each\n");
    map->for_each(map,hash_map_print_with_numeric_key);

    object_destroy(map);
    object_destroy(iter);

    cfg_destroy(c);

    dbg_str(DBG_SUC, "hash_map test end alloc count =%d",allocator->alloc_count);

}
void test_obj_hash_map()
{
    /*
     *test_obj_hash_map_string_key();
     */
    test_obj_hash_map_numeric_key();
}



