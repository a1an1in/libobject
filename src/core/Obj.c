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
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>
#include <libobject/core/Number.h>
#include <libobject/core/utils/dbg/debug.h>

int __set_int8_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int8_t t = (int8_t)(*((int *)value));
    int8_t *addr = (int8_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_uint8_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint8_t t = (uint8_t)(*((int *)value));
    uint8_t *addr = (uint8_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_int16_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int16_t t = (int16_t)(*((int *)value));
    int16_t *addr = (int16_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_uint16_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint16_t t = (uint16_t)(*((int *)value));
    uint16_t *addr = (uint16_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_int32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int32_t t = (int32_t)(*((int *)value));
    int32_t *addr = (int32_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_uint32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint32_t t = (uint32_t)(*((int *)value));
    uint32_t *addr = (uint32_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __set_float_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    double t1 = *((double *)value);
    float t2 = (double)t1;
    float *addr = (float *)(base + entry->offset);
    *addr = t2;

    return 1;
}

int __set_double_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    float *addr = (float *)(base + entry->offset);
    *addr = *((double *)value);

    return 1;
}

int __set_sn32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    Number **addr = (Number **)(base + entry->offset);

    if (*addr == NULL) {
        *addr = object_new(allocator, "Number", NULL);
        (*addr)->set_type((*addr), NUMBER_TYPE_SIGNED_INT);
    }

    (*addr)->set_value((*addr), value);

    return 1;
}

int __set_string_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    String **addr = (String **)(base + entry->offset);

    if (*addr != NULL)
        (*addr)->assign((*addr), (char *)value);
    else {
        *addr = object_new(allocator, "String", NULL);
        (*addr)->assign((*addr), (char *)value);
    }

    return 1;
}

int __set_vector_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    Vector **addr = (Vector **)(base + entry->offset);
    Vector *v;
    uint8_t trustee_flag = 1;
    char *p = (char *)value;
    int ret = 1;

    TRY {
        if (*addr != NULL) {
            v = *addr;
        } else {
            v = object_new(allocator, "Vector", NULL);
        }
        THROW_IF(p[0] != '[', -1);
        THROW_IF(p[1] == ']', 0);

        v->set(v, "/Vector/init_data", value);
        v->set(v, "/Vector/class_name", entry->type_name);
        v->set(v, "/Vector/trustee_flag", &trustee_flag);
        v->reconstruct(v);
        *addr = v;
        Vector **addr = (Vector **)(base + entry->offset);
    } CATCH (ret) {
    }

    return ret;
}

int __set_pointer_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    void **addr = (void **)(base + entry->offset);
    *addr = value;

    return 1;
}

static struct obj_set_policy_s {
    int (*policy)(Obj *obj, class_info_entry_t *entry, void *value);
} g_obj_set_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]         = {.policy = __set_int8_policy},
    [ENTRY_TYPE_UINT8_T]        = {.policy = __set_uint8_policy},
    [ENTRY_TYPE_INT16_T]        = {.policy = __set_int16_policy},
    [ENTRY_TYPE_UINT16_T]       = {.policy = __set_uint16_policy},
    [ENTRY_TYPE_INT32_T]        = {.policy = __set_int32_policy},
    [ENTRY_TYPE_UINT32_T]       = {.policy = __set_uint32_policy},
    [ENTRY_TYPE_FLOAT_T]        = {.policy = __set_float_policy},
    [ENTRY_TYPE_DOUBLE_T]       = {.policy = __set_double_policy},
    [ENTRY_TYPE_SN32]           = {.policy = __set_sn32_policy},
    [ENTRY_TYPE_STRING]         = {.policy = __set_string_policy},
    [ENTRY_TYPE_VECTOR]         = {.policy = __set_vector_policy},
    [ENTRY_TYPE_NORMAL_POINTER] = {.policy = __set_pointer_policy},
    [ENTRY_TYPE_FUNC_POINTER]   = {.policy = __set_pointer_policy},
    [ENTRY_TYPE_VFUNC_POINTER]  = {.policy = __set_pointer_policy},
    [ENTRY_TYPE_IFUNC_POINTER]  = {.policy = __set_pointer_policy},
    [ENTRY_TYPE_OBJ_POINTER]    = {.policy = __set_pointer_policy},
};

static int __to_json_int8_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int8_t *)value));

    return 1;
}

static int __to_json_uint8_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint8_t *)value));

    return 1;
}

static int __to_json_int16_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int16_t *)value));

    return 1;
}

static int __to_json_uint16_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint16_t *)value));

    return 1;
}

static int __to_json_int32_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int32_t *)value));

    return 1;
}

static int __to_json_uint32_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint32_t *)value));

    return 1;
}

static int __to_json_float_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((float *)value));

    return 1;
}

static int __to_json_string_policy(cjson_t *json, char *name, void *value)
{
    String *s = *(String **)value;
    if (s != NULL)
        cjson_add_string_to_object(json, name, s->value);

    return 1;
}

static int __to_json_sn32_policy(cjson_t *json, char *name, void *value)
{
    double d;
    Number *number = *(Number **)value;

    if (number == NULL) {
        dbg_str(DBG_WARNNING, "Number to json, but point is null, offset:%p", value);
        return -1;
    }
    d = number->get_signed_int_value(number);
    cjson_add_number_to_object(json, name, d);

    return 1;
}

static int __to_json_vector_policy(cjson_t *json, char *name, void *value)
{
    Vector *v = *((Vector **)value);
    cjson_t *item;

    if (v != NULL) {
        dbg_str(DBG_DETAIL, "Vector json: %s", v->to_json(v));
        item = cjson_parse(v->to_json(v));
        cjson_add_item_to_object(json, name, item);
    } else {
        dbg_str(DBG_WARNNING, "Vector to json, but content is null, offset:%p", value);
    }

    return 1;
}

static int __to_json_object_pointer_policy(cjson_t *json, char *name, void *value)
{
    Obj *o = *(Obj **)value;
    cjson_t *item;

    if (o != NULL) {
        item = cjson_parse(o->to_json(o));
        cjson_add_item_to_object(json, o->name, item);
    }

    return 1;
}

static struct obj_to_json_policy_s {
    int (*policy)(cjson_t *json, char *name, void *value);
} g_obj_to_json_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]      = {.policy = __to_json_int8_policy},
    [ENTRY_TYPE_UINT8_T]     = {.policy = __to_json_uint8_policy},
    [ENTRY_TYPE_INT16_T]     = {.policy = __to_json_int16_policy},
    [ENTRY_TYPE_UINT16_T]    = {.policy = __to_json_uint16_policy},
    [ENTRY_TYPE_INT32_T]     = {.policy = __to_json_int32_policy},
    [ENTRY_TYPE_UINT32_T]    = {.policy = __to_json_uint32_policy},
    [ENTRY_TYPE_FLOAT_T]     = {.policy = __to_json_float_policy},
    [ENTRY_TYPE_STRING]      = {.policy = __to_json_string_policy},
    [ENTRY_TYPE_SN32]        = {.policy = __to_json_sn32_policy},
    [ENTRY_TYPE_VECTOR]      = {.policy = __to_json_vector_policy},
    [ENTRY_TYPE_OBJ_POINTER] = {.policy = __to_json_object_pointer_policy},
};

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
