/**
 * @file Vector.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-08
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this vector of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this vector of conditions and the following disclaimer in the
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
#include <libobject/basic_types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/core/String.h>
#include <libobject/core/try.h>
#include <libobject/core/policy.h>
#include <libobject/argument/Command.h>

static int __construct(Vector *vector, char *init_str)
{
    allocator_t *allocator = vector->obj.allocator;
    cjson_t *c, *bak;
    int value_type = -1;
    String *s;
    char *out;
    Obj *o;

    dbg_str(OBJ_DETAIL, "vector construct, vector addr:%p", vector);

    vector->vector = vector_create(allocator, 0);
    if (vector->value_size == 0) {
        vector->value_size = sizeof(void *);
    }
    if (vector->capacity == 0) {
        vector->capacity = 10;
    }
    vector_init(vector->vector, vector->value_size, vector->capacity);

    if (vector->init_data == NULL) {
        return 0;
    }

    dbg_str(DBG_DETAIL, "vector init data:%s", STR2A(vector->init_data));
    c = cjson_parse(vector->init_data->get_cstr(vector->init_data));
    bak = c;
    if (c->type & OBJECT_ARRAY) {
        c = c->child;
        dbg_str(DBG_DETAIL, "array name:%s", c->string);
    }

    while (c) {
        if (c->type & CJSON_NUMBER) {
            value_type = ENTRY_TYPE_INT32_T;
            vector->add(vector, c->valueint);
        } else if (c->type & OBJECT_STRING) {
            value_type = VALUE_TYPE_STRING;
            dbg_str(DBG_DETAIL, "vector element:%s", c->valuestring);
            s = object_new(allocator, "String", NULL);
            s->assign(s, c->valuestring);
            vector->add(vector, s);
        } else if (c->type & OBJECT_ARRAY) {
            dbg_str(DBG_DETAIL, "vector element, not supported now!");
        } else if (c->type & CJSON_OBJECT) {
            value_type = VALUE_TYPE_OBJ_POINTER;
            char *class_name = STR2A(vector->class_name);
            out = cjson_print(c);
            o = object_new(allocator, class_name, out);
            vector->add(vector, o);

            dbg_str(DBG_DETAIL, "o: %s", o->to_json(o));
            free(out);
        } else {
            dbg_str(DBG_DETAIL, "vector element, not supported now!");
        }

        c = c->next;
    }

    if (value_type != -1)
        vector->set(vector, "/Vector/value_type", &value_type);

    cjson_delete(bak);

    return 1;
}

static int __deconstrcut(Vector *vector)
{
    dbg_str(OBJ_DETAIL, "vector deconstruct, vector addr:%p", vector);

    if (vector->init_data != NULL) {
        object_destroy(vector->init_data);
    }

    if (vector->class_name != NULL) {
        object_destroy(vector->class_name);
    }

    vector->reset(vector);

    vector_destroy(vector->vector);

    return 1;
}

static int __reconstruct(Vector *vector)
{
    allocator_t *allocator = vector->obj.allocator;

    vector_destroy(vector->vector);
    vector->construct(vector, NULL);

    return 1;
}

static int __add(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __add_back(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __add_at(Vector *vector, int index, void *value)
{
    return vector_add_at(vector->vector, index, value);
}

static int __remove(Vector *vector, int index, void **value)
{
    return vector_remove(vector->vector, index, value);
}

static int __remove_back(Vector *vector, void **value)
{
    return vector_remove_back(vector->vector, value);
}

static int __peek_at(Vector *vector, int index, void **value)
{
    return vector_peek_at(vector->vector, index, value);
}

static int 
__for_each(Vector *vector, int (*func)(int index, void *element))
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        vector->peek_at(vector, index, (void **)&element);
        func(index++, element);
    }
}

static void __free_vector_elements(Vector *vector)
{   
    vector->reset(vector);
}

static uint32_t __count(Vector * vector)
{
    uint32_t count = 0;

    sync_trylock(&vector->vector->vector_lock, NULL);
    count = vector->vector->count;
    sync_unlock(&vector->vector->vector_lock);

    return count;
} 

static int  __empty(Vector * vector) 
{
    return vector->count(vector) == 0 ? 1 : 0;
}

static int __reset(Vector *vector)
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        if (vector->trustee_flag != 1) {
            break;
        }

        vector->peek_at(vector, index++, (void **)&element);
        if (element == NULL) {
            continue;
        }

        if (vector->value_type == VALUE_TYPE_OBJ_POINTER) {
            object_destroy(element);
        } else if (vector->value_type  == VALUE_TYPE_STRING) {
            object_destroy(element);
        } else if (vector->value_type  == VALUE_TYPE_ALLOC_POINTER) {
            allocator_mem_free(vector->obj.allocator, element);
        } else if (vector->value_type  == VALUE_TYPE_UNKNOWN_POINTER) {
            dbg_str(DBG_WARNNING, "not support reset unkown pointer");
        } else {
        }
        element = NULL;
    }

    memset((void *)v->vector_head, 0, v->capacity * (v->step));
    v->count = 0;
    vector_pos_init(&v->end, 0, v);
}

static char *__to_json(Obj *obj)
{
    Vector *vector = (Vector *)obj;
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    cjson_t *root;
    String *json = (String *)obj->json;
    void *element = NULL;
    char *out;
    int index = 0;
    int ret;

    if (json == NULL) {
        json = object_new(obj->allocator, "String", NULL);
        obj->json = json;
    } else {
        json->reset(json);
    }

    root = cjson_create_array();

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        vector->peek_at(vector, index++, (void **)&element);
        
        if (vector->value_type > ENTRY_TYPE_MAX_TYPE) {
            return NULL;
        }
        if (g_vector_to_json_policy[vector->value_type].policy == NULL) {
            continue;
        }
        ret = g_vector_to_json_policy[vector->value_type].policy(root, element);
        if (ret < 0) {
            return NULL;
        }
    }

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static int __assign(Vector *vector, char *value)
{
    allocator_t *allocator = vector->obj.allocator;
    cjson_t *c, *bak = NULL;
    Obj *o;
    char *out;
    String *s, **sp2;
    char object_name[20] = {0};
    char *json;
    int ret = 1;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    TRY {
        if (value[0] != '[' && value[0] == '(') {
            out  = strchr(value, ')');
            THROW_IF(out == NULL, -1);
            strncpy(object_name, value + 1, out - value - 1);
            vector->set(vector, "/Vector/class_name", object_name);
            json = strchr(value, ':');
            THROW_IF(out == NULL, -1);
            json += 1;
        } else if (value[0] == '[') {
            json = value;
        } else {
            THROW(-1);
        }

        bak = c = cjson_parse(json);
        if (c->type & OBJECT_ARRAY) {
            c = c->child;
            dbg_str(DBG_DETAIL, "array name:%s", c->string);
        }

        while (c) {
            if (c->type & CJSON_NUMBER) {
                value_type = ENTRY_TYPE_INT32_T;
                vector->add(vector, c->valueint);
            } else if (c->type & OBJECT_STRING) {
                dbg_str(DBG_DETAIL, "vector element:%s", c->valuestring);
                value_type = VALUE_TYPE_STRING;
                s = object_new(allocator, "String", NULL);
                s->assign(s, c->valuestring);
                vector->add(vector, s);
            } else if (c->type & OBJECT_ARRAY) {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            } else if (c->type & CJSON_OBJECT) {
                value_type = VALUE_TYPE_OBJ_POINTER;
                out = cjson_print(c);

                sp2 = vector->get(vector, "/Vector/class_name");
                THROW_IF(sp2 == NULL || *sp2 == NULL, -1);
                o = object_new(allocator, STR2A(*sp2), out);
                vector->add(vector, o);

                dbg_str(DBG_DETAIL, "o: %s", o->to_json(o));
                free(out);
            } else {
                dbg_str(DBG_DETAIL, "vector assign, not supported %d now!", c->type);
            }

            c = c->next;
        }

        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
        vector->set(vector, "/Vector/value_type", &value_type);

        cjson_delete(bak);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "vector assign error, assign value:%s", value);
    }

    return ret;
}

static int __search(Vector *vector, int (*cmp)(void *element, void *key), void *key) 
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element = NULL;
    int ret = 1, index = 0;

    TRY {
        for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
             !vector_pos_equal(&pos, &v->end);
             pos = next, vector_pos_next(&pos, &next)) 
        {
            vector->peek_at(vector, index++, (void **)&element);
            CONTINUE_IF(element == NULL);
            ret = cmp(element, key);
            THROW_IF(ret == 1, index - 1);
        }
        THROW(-1);
    } CATCH(ret) {
    }

    return ret;
}

static int __get_end_index(Vector *vector)
{
    vector_t *v      = vector->vector;
    uint32_t end_pos = v->end.vector_pos;

    return end_pos;
}

static class_info_entry_t vector_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Vector, construct, __construct),
    Init_Nfunc_Entry(2 , Vector, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Vector, set, NULL),
    Init_Vfunc_Entry(4 , Vector, get, NULL),
    Init_Vfunc_Entry(5 , Vector, add, __add),
    Init_Vfunc_Entry(6 , Vector, add_at, __add_at),
    Init_Vfunc_Entry(7 , Vector, add_back, __add_back),
    Init_Vfunc_Entry(8 , Vector, remove, __remove),
    Init_Vfunc_Entry(9 , Vector, remove_back, __remove_back),
    Init_Vfunc_Entry(10, Vector, peek_at, __peek_at),
    Init_Vfunc_Entry(11, Vector, for_each, __for_each),
    Init_Vfunc_Entry(12, Vector, free_vector_elements, __free_vector_elements),
    Init_Vfunc_Entry(13, Vector, reset, __reset),
    Init_Vfunc_Entry(14, Vector, count, __count),
    Init_Vfunc_Entry(15, Vector, empty, __empty),
    Init_Vfunc_Entry(16, Vector, to_json, __to_json),
    Init_Vfunc_Entry(17, Vector, assign, __assign),
    Init_Vfunc_Entry(18, Vector, search, __search),
    Init_Vfunc_Entry(19, Vector, get_end_index, __get_end_index),
    Init_U32___Entry(20, Vector, value_size, NULL),
    Init_U8____Entry(21, Vector, value_type, NULL),
    Init_U32___Entry(22, Vector, capacity, NULL),
    Init_Str___Entry(23, Vector, init_data, NULL),
    Init_Str___Entry(24, Vector, class_name, NULL),
    Init_U8____Entry(25, Vector, trustee_flag, 0),
    Init_End___Entry(26, Vector),
};
REGISTER_CLASS("Vector", vector_class_info);
