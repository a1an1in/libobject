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
#include <libobject/core/utils/config.h>
#include <libobject/core/Map.h>

static int __construct(Map *map, char *init_str)
{
    dbg_str(OBJ_DETAIL, "map construct, map addr:%p", map);

    return 1;
}

static int __deconstrcut(Map *map)
{
    dbg_str(OBJ_DETAIL, "map deconstruct, map addr:%p", map);

    return 1;
}

static int __contain_key(Map *map, void *key)
{
}

static int __contain_value(Map *map, void *value)
{
}

static int __contain_key_and_value(Map *map, void *key, void *value)
{
}

static void __for_each(Map *map, void (*func)(void *key, void *element))
{
    Iterator *cur, *end;
    void *key, *value;

    dbg_str(OBJ_INFO, "Map for_each");

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

    dbg_str(OBJ_INFO, "Map for_each");
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

static int __reset(Map *map)
{
    Iterator *cur, *end;
    void *key, *value;
    void *element;
    String **class_name;

    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur = map->begin(map), end = map->end(map)) {
        key = cur->get_kpointer(cur);
        map->remove(map, key, &element);

        if (map->trustee_flag != 1 || element == NULL) {
            continue;
        }

        if (map->value_type == VALUE_TYPE_OBJ_POINTER) {
            object_destroy(element);
        } else if (map->value_type == VALUE_TYPE_STRING_POINTER) {
            object_destroy(element);
        } else if (map->value_type == VALUE_TYPE_ALLOC_POINTER) {
            allocator_mem_free(map->obj.allocator, element);
        } else if (map->value_type == VALUE_TYPE_STRUCT_POINTER) {
            class_name = map->get(map, "/Vector/class_name");
            if (*class_name != NULL) {
                dbg_str(DBG_SUC, "not support reset unkown pointer");
            }
            if (map->value_free_callback != NULL) {
                map->value_free_callback(map->obj.allocator, element); 
            } else (map->value_free_callback == NULLL) {
                allocator_mem_free(map->obj.allocator, element);
            }
        } else if (map->value_type >= VALUE_TYPE_MAX_TYPE) {
            dbg_str(DBG_WARN, "not support reset unkown pointer");
        }

        element = NULL;
    }

    return 1;
}

static int __destroy(Map *map)
{
    dbg_str(OBJ_DETAIL, "Map destroy");
}

static class_info_entry_t map_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Map, construct, __construct),
    Init_Nfunc_Entry(2 , Map, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Map, reconstruct, NULL),
    Init_Vfunc_Entry(4 , Map, set, NULL),
    Init_Vfunc_Entry(5 , Map, get, NULL),
    Init_Vfunc_Entry(6 , Map, add, NULL),
    Init_Vfunc_Entry(7 , Map, contain_key, NULL),
    Init_Vfunc_Entry(8 , Map, contain_value, NULL),
    Init_Vfunc_Entry(9 , Map, contain_key_and_value, NULL),
    Init_Vfunc_Entry(10, Map, search, NULL),
    Init_Vfunc_Entry(11, Map, search_all_same_key, NULL),
    Init_Vfunc_Entry(12, Map, remove, NULL),
    Init_Vfunc_Entry(13, Map, del, NULL),
    Init_Vfunc_Entry(14, Map, for_each, __for_each),
    Init_Vfunc_Entry(15, Map, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(16, Map, begin, __begin),
    Init_Vfunc_Entry(17, Map, end, __end),
    Init_Vfunc_Entry(18, Map, destroy, __destroy),
    Init_Vfunc_Entry(19, Map, count, NULL),
    Init_Vfunc_Entry(20, Map, reset, __reset),
    Init_Vfunc_Entry(21, Map, set_cmp_func, NULL),
    Init_U8____Entry(22, Map, trustee_flag, NULL),
    Init_U8____Entry(23, Map, value_type, NULL),
    Init_U8____Entry(24, Map, key_type, NULL),
    Init_Point_Entry(25, Map, value_free_callback, NULL),
    Init_End___Entry(26, Map),
};
REGISTER_CLASS(Map, map_class_info);
