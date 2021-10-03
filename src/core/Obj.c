/**
 * @file object.c
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
#include <libobject/core/object.h>
#include <libobject/core/policy.h>
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>
#include <libobject/core/Number.h>
#include <libobject/core/utils/dbg/debug.h>

static int __construct(Obj *obj, char *init_str)
{
    dbg_str(OBJ_DETAIL, "obj construct, obj addr:%p", obj);

    return 1;
}

static int __deconstrcut(Obj *obj)
{
    dbg_str(OBJ_DETAIL, "obj deconstruct, obj addr:%p", obj);

    if (obj->json != NULL) {
        object_destroy(obj->json);
    }

    allocator_mem_free(obj->allocator, obj);

    return 1;
}

/**
 * @Synopsis  
 *
 * @Param obj
 * @Param attrib
 *        if you want call this func, must designate class name in the attrib, like the format:"ClassName/attrib_name" to
 *        tell the object which class's attrib you want to set.
 * @Param value
 *
 * @Returns   
 */
static int __set(Obj *obj, char *attrib, void *value)
{
    class_info_entry_t *entry;
    allocator_t *allocator = obj->allocator;
    uint8_t *base = (uint8_t *)obj;
    char *buf = NULL;  
    char **out = NULL;  
    int cnt;
    char *target_name;
    int ret;

    TRY {
        cnt = compute_slash_count(attrib);
        if (cnt > 0 ) {
            out = allocator_mem_alloc(allocator, sizeof(char *) * (cnt));
            THROW_IF(out == NULL, -1);
            buf = allocator_mem_alloc(allocator, strlen(attrib) + 1);
            THROW_IF(buf == NULL, -1);
            strcpy(buf, attrib);
            str_split(buf, "/", out, &cnt);
            target_name = out[cnt - 2];
            attrib = out[cnt - 1];
        } else {
            target_name = obj->target_name;
        }

        entry = object_get_entry_of_class(target_name, attrib);
        THROW_IF(entry == NULL, -1);
        THROW_IF(entry->type >= ENTRY_TYPE_MAX_TYPE || g_obj_set_policy[entry->type].policy == NULL, -1);

        EXEC(g_obj_set_policy[entry->type].policy(obj, entry, value));
    } CATCH (ret) {
    } FINALLY {
        allocator_mem_free(allocator, out);
        allocator_mem_free(allocator, buf);
    }

    return ret;
}

/**
 * @Synopsis  
 *
 * @Param obj
 * @Param attrib
 *        if you want call this func, must designate class name in the attrib, like the format:"ClassName/attrib_name" to tell the object which class's attrib you want to get.
 * @Param value
 *
 * @Returns   
 */
static void *__get(Obj *obj, char *attrib)
{
    class_info_entry_t *entry;
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    void *addr;
    char *buf = NULL;  
    char **out = NULL;  
    int cnt, ret = 1;
    char *target_name;

    TRY {
        cnt = compute_slash_count(attrib);
        if (cnt > 0 ) {
            out = allocator_mem_alloc(allocator, sizeof(char *) * cnt);
            THROW_IF(out == NULL, -1);
            buf = allocator_mem_alloc(allocator, strlen(attrib) + 1);
            THROW_IF(buf == NULL, -1);
            strcpy(buf, attrib);
            str_split(buf, "/", out, &cnt);

            dbg_str(DBG_WARNNING, "set class name:%s", out[cnt - 2]);
            target_name = out[cnt - 2]; //class name
            attrib = out[cnt - 1]; //real attrib name
        } else {
            target_name = obj->target_name;
        }

        entry  = object_get_entry_of_class(target_name, attrib);
        THROW_IF(entry == NULL, -1);

        switch(entry->type) {
            case ENTRY_TYPE_INT8_T:
            case ENTRY_TYPE_UINT8_T:
            case ENTRY_TYPE_INT16_T:
            case ENTRY_TYPE_UINT16_T:
            case ENTRY_TYPE_INT32_T:
            case ENTRY_TYPE_UINT32_T:
            case ENTRY_TYPE_INT64_T:
            case ENTRY_TYPE_UINT64_T:
            case ENTRY_TYPE_FLOAT_T:
            case ENTRY_TYPE_SN32:
            case ENTRY_TYPE_STRING:
            case ENTRY_TYPE_VECTOR:
            case ENTRY_TYPE_NORMAL_POINTER:
            case ENTRY_TYPE_FUNC_POINTER:
            case ENTRY_TYPE_VFUNC_POINTER:
            case ENTRY_TYPE_IFUNC_POINTER:
            case ENTRY_TYPE_OBJ_POINTER:
                {
                    addr = (base + entry->offset);
                    break;
                }
            default:
                dbg_str(DBG_DETAIL, "get %s, not support %s item",
                        obj->target_name,
                        attrib);
                addr = NULL;
                break;
        }
    } CATCH (ret) {
        addr = NULL;
    } FINALLY {
        allocator_mem_free(allocator, out);
        allocator_mem_free(allocator, buf);
    }
    return addr;
}

static int __to_json__(void *obj, char *type_name, cjson_t *object) 
{
    class_deamon_t *deamon;
    class_info_entry_t *entry;
    void *(*get)(void *obj, char *attrib) = NULL;
    int i;
    cjson_t *item;
    void *value;
    Obj *o = (Obj *)obj;
    int ret;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *) 
                 class_deamon_search_class(deamon, (char *)type_name);

        get = __object_get_func_of_class_recursively(entry, (char *)"get");
        THROW_IF(get == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            THROW_IF(entry[i].type > ENTRY_TYPE_MAX_TYPE, -1);
            if (entry[i].type == ENTRY_TYPE_OBJ) {
                CONTINUE_IF(strcmp(entry[i].type_name, "Obj") == 0);
                item = cjson_create_object();
                cjson_add_item_to_object(object, entry[i].type_name, item);
                dbg_str(DBG_DETAIL, "%s to json", entry[i].type_name);
                __to_json__(obj, entry[i].type_name, item);
            } else {
                CONTINUE_IF(g_obj_to_json_policy[entry[i].type].policy == NULL);
                strcpy(o->target_name, type_name);

                value = get(obj, entry[i].value_name);
                CONTINUE_IF(value == NULL);
                EXEC(g_obj_to_json_policy[entry[i].type].policy(object, entry[i].value_name, value));
            }
        }   
    } CATCH (ret) {
    }

    return ret;
}

static char *__to_json(Obj *obj) 
{
    String *json = (String *)obj->json;
    cjson_t *root;
    char *out;
    int len;

    if (json == NULL) {
        json = object_new(obj->allocator, "String", NULL);
        obj->json = json;
    } else {
        json->reset(json);
    }

    root = cjson_create_object();

    dbg_str(DBG_DETAIL, "%s to json", obj->name);

    __to_json__(obj, obj->name, root);

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static char *__reset(Obj *obj) 
{
    return 1;
}

static int __assign(Obj *obj, char *value)
{
    return 1;
}

static int 
__override_virtual_funcs__(Obj *obj,
                           char *cur_class_name,
                           char *func_name, void *value)
{
    class_deamon_t *deamon;
    void *class_info;
    class_info_entry_t * entry_of_parent_class;
    int (*set)(void *obj, char *attrib, void *value);
    Obj *o = (Obj *)obj;
    int ret = 1;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        THROW_IF(deamon == NULL, -1);

        class_info = class_deamon_search_class(deamon, (char *)cur_class_name);
        THROW_IF(class_info == NULL, -1);

        set = __object_get_func_of_class_recursively(class_info, "set");
        THROW_IF(set == NULL, -1);

        entry_of_parent_class = __object_get_entry_of_parent_class(class_info);
        if (entry_of_parent_class != NULL) {
            __override_virtual_funcs__(obj, entry_of_parent_class->type_name, func_name, value); 
        }

        strcpy(o->target_name, cur_class_name);
        set(obj, func_name, value); 
    } CATCH (ret) {
    }

    return ret;
}

static int __override_virtual_funcs(Obj *obj, char *func_name, void *value)
{
    return __override_virtual_funcs__(obj, obj->name, func_name, value); 
}

static class_info_entry_t obj_class_info[] = {
    [0] = {ENTRY_TYPE_NORMAL_POINTER, "allocator_t", "allocator", NULL, sizeof(void *), offset_of_class(Obj, allocator)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *), offset_of_class(Obj, construct)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *), offset_of_class(Obj, deconstruct)}, 
    [3] = {ENTRY_TYPE_VFUNC_POINTER, "", "set", __set, sizeof(void *), offset_of_class(Obj, set)}, 
    [4] = {ENTRY_TYPE_VFUNC_POINTER, "", "get", __get, sizeof(void *), offset_of_class(Obj, get)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "to_json", __to_json, sizeof(void *), offset_of_class(Obj, to_json)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "reset", __reset, sizeof(void *), offset_of_class(Obj, reset)}, 
    [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "assign", __assign, sizeof(void *), offset_of_class(Obj, assign)}, 
    [8] = {ENTRY_TYPE_VFUNC_POINTER, "", "override_virtual_funcs", __override_virtual_funcs, sizeof(void *), offset_of_class(Obj, override_virtual_funcs)}, 
    [9] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Obj", obj_class_info);
