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
#include <libdbg/debug.h>
#include <libobject/map_hash.h>

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
        /*
         *dbg_str(DBG_DETAIL,"hash map set for each addr:%p",value);
         */
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
    } else {
        dbg_str(OBJ_WARNNING,"hash map get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __insert(Map *map,void *key,void *value)
{
    dbg_str(OBJ_DETAIL,"Hash Map insert");

    return hash_map_insert(((Hash_Map *)map)->hmap,key,value);
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
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","insert",__insert,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","insert_wb",__insert_wb,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","search",__search,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","begin",__begin,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","end",__end,sizeof(void *)},
    [11] = {ENTRY_TYPE_UINT16_T,"","key_size",NULL,sizeof(short)},
    [12] = {ENTRY_TYPE_UINT16_T,"","value_size",NULL,sizeof(short)},
    [13] = {ENTRY_TYPE_UINT16_T,"","bucket_size",NULL,sizeof(short)},
    [14] = {ENTRY_TYPE_FUNC_POINTER,"","for_each",NULL,sizeof(void *)},
    [15] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Hash_Map",hash_map_class_info);

static void hash_map_print(Iterator *iter)
{
    Hmap_Iterator *i = (Hmap_Iterator *)iter;
    dbg_str(DBG_DETAIL,"key:%s value:%s",i->get_kpointer(iter), i->get_vpointer(iter));
}

void test_obj_hash_map()
{
    Iterator *iter, *next,*prev;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Hash_Map", e = cjson_create_object());{
            cjson_add_number_to_object(e, "key_size", 10);
            cjson_add_number_to_object(e, "value_size", 25);
            cjson_add_number_to_object(e, "bucket_size", 15);
        }
    }
    set_str = cjson_print(root);

#if 1
    dbg_str(DBG_SUC, "hash_map test begin alloc count =%d",allocator->alloc_count);

    Map *map;
    map  = OBJECT_NEW(allocator, Hash_Map,set_str);
    iter = OBJECT_NEW(allocator, Hmap_Iterator,set_str);

    object_dump(map, "Hash_Map", buf, 2048);
    dbg_str(DBG_DETAIL,"Map dump: %s",buf);

    map->insert(map,"abc","hello world");
    map->insert(map,"test","sdfsafsdaf");
    map->for_each(map,hash_map_print);

    map->search(map,"abc",iter);
    dbg_str(DBG_DETAIL,"search data:%s",iter->get_vpointer(iter));
    map->del(map,iter);

    map->for_each(map,hash_map_print);

#else
    Hash_Map *map;
    map  = OBJECT_NEW(allocator, Hash_Map,set_str);
    iter = OBJECT_NEW(allocator, Hmap_Iterator,set_str);

    object_dump(map, "Hash_Map", buf, 2048);
    dbg_str(OBJ_DETAIL,"Map dump: %s",buf);

    map->insert((Map *)map,"abc","hello world");
    map->insert((Map *)map,"test","sdfsafsdaf");
    map->for_each((Map *)map,hash_map_print);

#endif
    object_destroy(map);
    object_destroy(iter);

    dbg_str(DBG_SUC, "hash_map test end alloc count =%d",allocator->alloc_count);

    free(set_str);
}


