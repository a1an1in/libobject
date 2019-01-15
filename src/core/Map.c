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
#include <libobject/core/map.h>

static int __construct(Map *map, char *init_str)
{
    dbg_str(OBJ_DETAIL, "map construct, map addr:%p", map);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(OBJ_DETAIL, "map deconstruct, map addr:%p", map);

    return 0;
}

static int __set(Map *map, char *attrib, void *value)
{
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
    } else if (strcmp(attrib, "search_all_same_key") == 0) {
        map->search_all_same_key = value;
    } else if (strcmp(attrib, "remove") == 0) {
        map->remove = value;
    } else if (strcmp(attrib, "del") == 0) {
        map->del = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        map->for_each = value;
    } else if (strcmp(attrib, "for_each_arg") == 0) {
        map->for_each_arg = value;
    } else if (strcmp(attrib, "begin") == 0) {
        map->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        map->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        map->destroy = value;
    } else if (strcmp(attrib, "set_cmp_func") == 0) {
        map->set_cmp_func = value;
    } else if (strcmp(attrib, "size") == 0) {
        map->size = value;
    } else if (strcmp(attrib, "is_empty") == 0) {
        map->is_empty = value;
    }
    else if (strcmp(attrib, "name") == 0) {
        strncpy(map->name, value, strlen(value));
    } else {
        dbg_str(OBJ_DETAIL, "map set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Map *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "for_each") == 0) {
        return obj->for_each;
    } else {
        dbg_str(OBJ_WARNNING, "map get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static void __for_each(Map *map, void (*func)(void *key, void *element))
{
    Iterator *cur, *end;
    void *key, *value;

    dbg_str(OBJ_IMPORTANT, "Map for_each");
    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        func(key, value);
    }

}

static void __for_each_arg(Map *map,void (*func)(void *key, void *element, void *arg),void *arg)
{
    Iterator *cur, *end;
    void *key, *value;

    dbg_str(OBJ_IMPORTANT, "Map for_each");
    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        func(key, value, arg);
    }
}

static Iterator *__begin(Map *map)
{
    dbg_str(OBJ_DETAIL, "Map begin");
}

static Iterator *__end(Map *map)
{
    dbg_str(OBJ_DETAIL, "Map end");
}

static int __destroy(Map *map)
{
    dbg_str(OBJ_DETAIL, "Map destroy");
}

static class_info_entry_t map_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add", NULL, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "search", NULL, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "search_all_same_key", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "del", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each", __for_each, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg", __for_each_arg, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "begin", __begin, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "end", __end, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy", __destroy, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_cmp_func", NULL, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "size", NULL, sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "is_empty", NULL, sizeof(void *)},
    [18] = {ENTRY_TYPE_STRING, "char", "name", NULL, 0}, 
    [19] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Map", map_class_info);

void test_obj_map()
{
#if 0
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Map", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    map = OBJECT_NEW(allocator, Map, set_str);

    object_dump(map, "Map", buf, 2048);
    dbg_str(OBJ_DETAIL, "Map dump: %s", buf);

    object_destroy(map);
    free(set_str);
#else
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Map", CJSON_STRING, "name", "alan map") ;  

    map = OBJECT_NEW(allocator, Map, c->buf);

    object_dump(map, "Map", buf, 2048);
    dbg_str(DBG_DETAIL, "Map dump: %s", buf);

    object_destroy(map);
    cfg_destroy(c);

#endif

}


