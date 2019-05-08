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

static int __contain_key(Map *map,void *key)
{
}

static int __contain_value(Map *map,void *value)
{
}

static int __contain_key_and_value(Map *map,void *key, void *value)
{
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
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Map, construct, __construct),
    Init_Nfunc_Entry(2 , Map, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Map, set, NULL),
    Init_Vfunc_Entry(4 , Map, get, NULL),
    Init_Vfunc_Entry(5 , Map, add, NULL),
    Init_Vfunc_Entry(6 , Map, contain_key, NULL),
    Init_Vfunc_Entry(7 , Map, contain_value, NULL),
    Init_Vfunc_Entry(8 , Map, contain_key_and_value, NULL),
    Init_Vfunc_Entry(9 , Map, search, NULL),
    Init_Vfunc_Entry(10, Map, search_all_same_key, NULL),
    Init_Vfunc_Entry(11, Map, remove, NULL),
    Init_Vfunc_Entry(12, Map, del, NULL),
    Init_Vfunc_Entry(13, Map, for_each, __for_each),
    Init_Vfunc_Entry(14, Map, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(15, Map, begin, __begin),
    Init_Vfunc_Entry(16, Map, end, __end),
    Init_Vfunc_Entry(17, Map, destroy, __destroy),
    Init_Vfunc_Entry(18, Map, set_cmp_func, NULL),
    Init_Nfunc_Entry(19, Map, set_target_name, NULL),
    Init_Str___Entry(20, Map, name, NULL),
    Init_End___Entry(21, Map),
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


