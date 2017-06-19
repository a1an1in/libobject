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
#include <libobject/map.h>

static int __construct(Map *map,char *init_str)
{
    dbg_str(OBJ_DETAIL,"map construct, map addr:%p",map);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(OBJ_DETAIL,"map deconstruct,map addr:%p",map);

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
    } else if (strcmp(attrib, "for_each_arg2") == 0) {
        map->for_each_arg2 = value;
    } else if (strcmp(attrib, "begin") == 0) {
        map->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        map->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        map->destroy = value;
    } else {
        dbg_str(OBJ_DETAIL,"map set, not support %s setting",attrib);
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
        dbg_str(OBJ_WARNNING,"map get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __insert(Map *map,void *key,void *value)
{
    dbg_str(OBJ_DETAIL,"Map insert");
}

static int __insert_wb(Map *map,void *key,void *value,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"Map insert wb");
}

static int __search(Map *map,void *key,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"Map search");
}

static int __del(Map *map,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"Map del");
}

static void __for_each(Map *map,void (*func)(Iterator *iter))
{
    Iterator *cur, *end;

    dbg_str(OBJ_IMPORTANT,"Map for_each");
    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end,cur); cur->next(cur)) {
        func(cur);
    }

}

static void __for_each_arg2(Map *map,void (*func)(Iterator *iter,void *arg),void *arg)
{
    Iterator *cur, *end;

    dbg_str(OBJ_DETAIL,"Map for_each arg2");
    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end,cur); cur->next(cur)) {
        func(cur, arg);
    }

}

static Iterator *__begin(Map *map)
{
    dbg_str(OBJ_DETAIL,"Map begin");
}

static Iterator *__end(Map *map)
{
    dbg_str(OBJ_DETAIL,"Map end");
}

static int __destroy(Map *map)
{
    dbg_str(OBJ_DETAIL,"Map destroy");
}

static class_info_entry_t map_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","insert",__insert,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","insert_wb",__insert_wb,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","search",__search,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del",__del,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each",__for_each,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each_arg2",__for_each_arg2,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","begin",__begin,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","end",__end,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","destroy",__destroy,sizeof(void *)},
    [14] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Map",map_class_info);

void test_obj_map()
{
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

    map = OBJECT_NEW(allocator, Map,set_str);

    object_dump(map, "Map", buf, 2048);
    dbg_str(OBJ_DETAIL,"Map dump: %s",buf);

    free(set_str);

}


