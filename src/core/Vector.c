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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/core/String.h>
#include <libobject/argument/Command.h>

static int __construct(Vector *vector, char *init_str)
{
    allocator_t *allocator = vector->obj.allocator;

    dbg_str(OBJ_DETAIL, "vector construct, vector addr:%p", vector);

    vector->vector = vector_create(allocator, 0);
    if (vector->value_size == 0) {
        vector->value_size = sizeof(void *);
    }
    if (vector->capacity == 0) {
        vector->capacity = 10;
    }
    vector_init(vector->vector, vector->value_size, vector->capacity);

    if (vector->init_data != NULL) {
        cjson_t *c;

        dbg_str(DBG_DETAIL, "vector init data:%s", 
                vector->init_data->get_cstr(vector->init_data));

        c = cjson_parse(vector->init_data->get_cstr(vector->init_data));
        if (c->type & OBJECT_ARRAY) {
            c = c->child;
            dbg_str(DBG_DETAIL, "array name:%s", c->string);
        }

        while (c) {
            if (c->type & CJSON_NUMBER) {
                vector->add(vector, c->valueint);
            } else if (c->type & OBJECT_STRING) {
                String *s;
                dbg_str(DBG_DETAIL, "vector element:%s", c->valuestring);
                s = object_new(allocator, "String", NULL);
                s->assign(s, c->valuestring);
                vector->add(vector, s);
            } else if (c->type & OBJECT_ARRAY) {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            } else if (c->type & CJSON_OBJECT) {
                char *out;
                Obj *o;
                char *class_name = vector->class_name->get_cstr(vector->class_name);
                char init_str[1024];
                out = cjson_print(c);
                sprintf(init_str, "{\"%s\":%s}", class_name, out);
                dbg_str(DBG_DETAIL, "init_str:%s", init_str);
                o = object_new(allocator, class_name, init_str);
                vector->add(vector, o);

                dbg_str(DBG_DETAIL, "o: %s", o->to_json(o));
                free(out);
            } else {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            }

            c = c->next;
        }

        cjson_delete(c);
    }

    return 0;
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

    vector->clear(vector);

    vector_destroy(vector->vector);

    return 0;
}

static int __reconstruct(Vector *vector)
{
    allocator_t *allocator = vector->obj.allocator;

    dbg_str(DBG_WARNNING, "vector reconstruct, vector addr:%p", vector);

    vector_destroy(vector->vector);

    vector->construct(vector, NULL);

    return 0;
}

static int __add(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __add_back(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __remove(Vector *vector, int index, void **value)
{
    return vector_remove(vector->vector, index, value);
}

static int __remove_back(Vector *vector, void **value)
{
    return vector_remove_back(vector->vector, value);
}

static int __add_at(Vector *vector, int index, void *value)
{
   return vector_add_at(vector->vector, index, value);
}

static int __peek_at(Vector *vector, int index, void **value)
{
    return vector_peek_at(vector->vector, index, value);
}

static void 
__for_each(Vector *vector, void (*func)(int index, void *element))
{
	vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

	for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
			!vector_pos_equal(&pos, &v->end);
			pos = next, vector_pos_next(&pos, &next))
	{
        vector->peek_at(vector, index, (void **)&element);
		func(index++, element);
	}
}

static void __free_vector_elements(Vector *vector)
{   
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    for(vector_begin(v, &pos), vector_pos_next(&pos, &next);
        !vector_pos_equal(&pos, &v->end);
        pos = next, vector_pos_next(&pos, &next))
    {
        vector->peek_at(vector, index++, (void **)&element);
        if(element != NULL){
            object_destroy(element);
        }
    }
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

static void __clear(Vector *vector)
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    if (vector->trustee_flag == 1) {
        for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
                !vector_pos_equal(&pos, &v->end);
                pos = next, vector_pos_next(&pos, &next))
        {
            vector->peek_at(vector, index++, (void **)&element);

            if (    vector->value_type == VALUE_TYPE_OBJ_POINTER && 
                    element != NULL) 
            {
                object_destroy(element);
            } else if (vector->value_type  == VALUE_TYPE_STRING &&
                       element != NULL)
            {
                object_destroy(element);
            } else if (vector->value_type  == VALUE_TYPE_ALLOC_POINTER &&
                       element != NULL)
            {
                allocator_mem_free(vector->obj.allocator, element);
            } else if (vector->value_type  == VALUE_TYPE_UNKNOWN_POINTER &&
                       element != NULL)
            {
                dbg_str(DBG_WARNNING, "not support clear unkown pointer");
            } else {
            }
            element = NULL;
        }
    }

    vector->vector->count = 0;
    vector_pos_init(&v->end, 0, v);
}

static char *__to_json(Obj *obj)
{
    Vector *vector = (Vector *)obj;
	vector_pos_t pos, next;
    vector_t *v = vector->vector;
    cjson_t *root, *item = NULL;
    String *json = (String *)obj->json;
    void *element = NULL;
    char *out;
    int index = 0;

    if (json == NULL) {
        json = object_new(obj->allocator, "String", NULL);
        obj->json = json;
    } else {
        json->clear(json);
    }

    root = cjson_create_array();

	for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
			!vector_pos_equal(&pos, &v->end);
			pos = next, vector_pos_next(&pos, &next))
	{
        vector->peek_at(vector, index++, (void **)&element);

        switch(vector->value_type) {
            case VALUE_TYPE_INT8_T: {
                    int8_t num = (int8_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT8_T: {
                    uint8_t num = (uint8_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT16_T: {
                    int16_t num = (int16_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT16_T: {
                    uint16_t num = (uint16_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT32_T: {
                    int32_t num = (int32_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT32_T: {
                    uint32_t num = (uint32_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT64_T: {
                    int64_t num = (int64_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT64_T: {
                    uint64_t num = (uint64_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_FLOAT_T: {
                    float *num = (float *)element;
                    item = cjson_create_number(*num);
                    break;
                }
            case VALUE_TYPE_STRING: {
                    String *s = (String *)element;
                    item = cjson_create_string(s->get_cstr(s));
                    break;
                }
            case VALUE_TYPE_OBJ_POINTER: {
                    Obj *o = (Obj *)element;
                    item = cjson_parse(o->to_json(o));
                    break;
                }
            case VALUE_TYPE_NORMAL_POINTER:
                dbg_str(DBG_DETAIL, "value type:%d not supported now!!",
                        vector->value_type);
                break;
            default:
                break;
        }

        if (item != NULL)
            cjson_add_item_to_array(root, item);
        item = NULL;
	}

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static class_info_entry_t vector_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Vector, construct, __construct),
    Init_Nfunc_Entry(2 , Vector, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Vector, reconstruct, __reconstruct),
    Init_Vfunc_Entry(4 , Vector, set, NULL),
    Init_Vfunc_Entry(5 , Vector, get, NULL),
    Init_Vfunc_Entry(6 , Vector, add, __add),
    Init_Vfunc_Entry(7 , Vector, add_at, __add_at),
    Init_Vfunc_Entry(8 , Vector, add_back, __add_back),
    Init_Vfunc_Entry(9 , Vector, remove, __remove),
    Init_Vfunc_Entry(10, Vector, remove_back, __remove_back),
    Init_Vfunc_Entry(11, Vector, peek_at, __peek_at),
    Init_Vfunc_Entry(12, Vector, for_each, __for_each),
    Init_Vfunc_Entry(13, Vector, free_vector_elements, __free_vector_elements),
    Init_Vfunc_Entry(14, Vector, clear, __clear),
    Init_Vfunc_Entry(15, Vector, count, __count),
    Init_Vfunc_Entry(16, Vector, empty, __empty),
    Init_Vfunc_Entry(17, Vector, to_json, __to_json),
    Init_U32___Entry(18, Vector, value_size, NULL),
    Init_U8____Entry(19, Vector, value_type, NULL),
    Init_U32___Entry(20, Vector, capacity, NULL),
    Init_Str___Entry(21, Vector, init_data, NULL),
    Init_Str___Entry(22, Vector, class_name, NULL),
    Init_U8____Entry(23, Vector, trustee_flag, 0),
    Init_End___Entry(24, Vector),
};
REGISTER_CLASS("Vector", vector_class_info);
