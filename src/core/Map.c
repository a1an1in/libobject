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
#include <libobject/core/object.h>
#include <libobject/core/String.h>
#include <libobject/core/Map.h>

static int __construct(Map *map, char *init_str)
{
    dbg_str(OBJ_DETAIL, "map construct, map addr:%p", map);

    return 1;
}

static int __deconstrcut(Map *map)
{
    dbg_str(OBJ_DETAIL, "map deconstruct, map addr:%p", map);

    object_destroy(map->class_name);
    return 1;
}

static int __contain_key(Map *map, void *key)
{
    return map->search(map, key, NULL);
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
            class_name = map->get(map, "/Map/class_name");
            if (*class_name != NULL) {
                int (*free_method)(allocator_t *allocator, void *info);
                dbg_str(DBG_VIP, "map strcut adapter class name:%s", STR2A(*class_name));
                free_method = object_get_member_of_class(STR2A(*class_name), "free");
                if (free_method == NULL) return -1;
                free_method(map->obj.allocator, element);
                continue;
            } else if (map->value_free_callback != NULL) {
                map->value_free_callback(map->obj.allocator, element); 
            } else if (map->value_free_callback == NULL) {
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

static char *__to_json(Map *map)
{
    int (*policy)(cjson_t *root, void *element);
    Iterator *cur, *end;
    void *key, *value;
    cjson_t *root;
    String *json = (String *)map->obj.json;
    String **class_name;
    void *element = NULL;
    char *out;
    int index = 0;
    int ret = 0;

    TRY {
        if (json == NULL) {
            json = object_new(map->obj.allocator, "String", NULL);
            map->obj.json = json;
        } else {
            json->reset(json);
        }

        cur = map->begin(map), end = map->end(map);
        root = cjson_create_array();

        for (; !end->equal(end, cur); cur->next(cur)) {
            key = cur->get_kpointer(cur);
            element = cur->get_vpointer(cur);
            if (element == NULL) {continue;}

            if (map->value_type == VALUE_TYPE_STRUCT_POINTER) {
                //因为这个是定制化的、多变的， 没法加入g_vector_to_json_policy。
                class_name = map->get(map, "/Map/class_name");
                THROW_IF(*class_name == NULL, -1);
                policy = object_get_member_of_class(STR2A(*class_name), "to_json");
            } else if (map->value_type == VALUE_TYPE_STRING_POINTER || map->value_type == VALUE_TYPE_OBJ_POINTER){
                policy = g_map_to_json_policy[map->value_type].policy;
            } else {
                dbg_str(DBG_ERROR, "map not supported to_json value_type:%d", map->value_type);
                THROW(-1);
            }
            
            if (policy == NULL) { continue; }
            EXEC(policy(root, element));
            element = NULL;
        }

        out = cjson_print(root);
        json->assign(json, out);
    } CATCH (ret) {} FINALLY {
        cjson_delete(root);
        free(out);
    }

    return ret < 0 ? NULL: json->get_cstr(json);
}

static class_info_entry_t map_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Map, construct, __construct),
    Init_Nfunc_Entry(2 , Map, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Map, reconstruct, NULL),
    Init_Vfunc_Entry(4 , Map, set, NULL),
    Init_Vfunc_Entry(5 , Map, get, NULL),
    Init_Vfunc_Entry(6 , Map, add, NULL),
    Init_Vfunc_Entry(7 , Map, contain_key, __contain_key),
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
    Init_Vfunc_Entry(21, Map, to_json, __to_json),
    Init_Vfunc_Entry(22, Map, set_cmp_func, NULL),
    Init_U8____Entry(23, Map, trustee_flag, NULL),
    Init_U8____Entry(24, Map, value_type, NULL),
    Init_U8____Entry(25, Map, key_type, NULL),
    Init_Point_Entry(26, Map, value_free_callback, NULL),
    Init_Str___Entry(27, Map, class_name, NULL),
    Init_End___Entry(28, Map),
};
REGISTER_CLASS(Map, map_class_info);

static int __map_to_json_string_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    String *s = (String *)element;

    item = cjson_create_string(s->get_cstr(s));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __map_to_json_object_pointer_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    Obj *o = (Obj *)element;

    item = cjson_parse(o->to_json(o));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

map_to_json_policy_t g_map_to_json_policy[ENTRY_TYPE_MAX_TYPE] = {
    [VALUE_TYPE_STRING_POINTER] = {.policy = __map_to_json_string_policy},
    [VALUE_TYPE_OBJ_POINTER]    = {.policy = __map_to_json_object_pointer_policy},
};