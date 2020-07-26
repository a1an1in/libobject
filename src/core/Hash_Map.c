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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Hash_Map.h>

static int __construct(Map *map, char *init_str)
{
    Hash_Map *h = (Hash_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    if (h->bucket_size == 0) { h->bucket_size = 20; }

    h->hmap = hash_map_alloc(allocator);

    hash_map_init(h->hmap);

    map->b = object_new(allocator, "Hmap_Iterator", NULL);
    map->e = object_new(allocator, "Hmap_Iterator", NULL);

    return 0;
}

static int __deconstruct(Map *map)
{
    dbg_str(HMAP_DETAIL, "hash map deconstruct, map addr:%p", map);

    object_destroy(map->b);
    object_destroy(map->e);
    map->reset(map);
    hash_map_destroy(((Hash_Map *)map)->hmap);

    return 0;
}

static int __reconstruct(Map *map)
{
    Hash_Map *h = (Hash_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    hash_map_destroy(h->hmap);

    if (h->bucket_size == 0) { h->bucket_size = 20; }

    h->hmap = hash_map_alloc(allocator);

    hash_map_init(h->hmap);

    return 0;
}

static int __add(Map *map, void *key, void *value)
{
    Hash_Map *hmap = (Hash_Map *)map;

    dbg_str(HMAP_DETAIL, "Hash Map add");

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
    if (ret != 1) {
        return ret;
    }

    ret = hash_map_delete(((Hash_Map *)map)->hmap, &pos);

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

static int __count(Map *map)
{
    Hash_Map *hmap = (Hash_Map *)map;

    return hash_map_get_count(((Hash_Map *)map)->hmap);
}

static int __set_cmp_func(Map *map, void *func)
{
    Hash_Map *hmap = (Hash_Map *)map;

    hmap->hmap->key_cmp_func = func;

    return 0;
}

static class_info_entry_t hash_map_class_info[] = {
    Init_Obj___Entry(0 , Map, map),
    Init_Nfunc_Entry(1 , Hash_Map, construct, __construct),
    Init_Nfunc_Entry(3 , Hash_Map, deconstruct, __deconstruct),
    Init_Vfunc_Entry(2 , Hash_Map, reconstruct, __reconstruct),
    Init_Vfunc_Entry(4 , Hash_Map, add, __add),
    Init_Vfunc_Entry(5 , Hash_Map, search, __search),
    Init_Vfunc_Entry(6 , Hash_Map, remove, __remove),
    Init_Vfunc_Entry(7 , Hash_Map, del, __del),
    Init_Vfunc_Entry(8 , Hash_Map, begin, __begin),
    Init_Vfunc_Entry(9 , Hash_Map, end, __end),
    Init_Vfunc_Entry(10, Hash_Map, count, __count),
    Init_Vfunc_Entry(11, Hash_Map, reset, NULL),
    Init_Vfunc_Entry(12, Hash_Map, for_each, NULL),
    Init_Vfunc_Entry(13, Hash_Map, set_cmp_func, __set_cmp_func),
    Init_U16___Entry(14, Hash_Map, bucket_size, NULL),
    Init_End___Entry(15, Hash_Map),
};
REGISTER_CLASS("Hash_Map", hash_map_class_info);
