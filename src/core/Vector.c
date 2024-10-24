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
#include <libobject/core/String.h>
#include <libobject/argument/Command.h>

/* 
 * 如果value类型如果是struct，则不能使用object_new 直接传入初始值初始化vector，
 * 因为vector没法知道struct具体怎么构造， 如果是对象vector默认当作object类型来
 * 处理。
 */
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
        vector->capacity = 20;
    }
    vector_init(vector->vector, vector->value_size, vector->capacity);

    return 1;
}

static int __deconstrcut(Vector *vector)
{
    dbg_str(OBJ_DETAIL, "vector deconstruct, vector addr:%p", vector);

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

static int __add_vector(Vector *vector, Vector *src)
{
    vector_pos_t pos, next;
    vector_t *v = src->vector;
    void *element = NULL;
    int index = 0, ret = 0;

    TRY {
        THROW_IF(vector == NULL || src == NULL, -1);
        for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
                !vector_pos_equal(&pos, &v->end);
                pos = next, vector_pos_next(&pos, &next), index++) {
            src->peek_at(src, index, (void **)&element);

            if (src->value_type == VALUE_TYPE_OBJ_POINTER || src->value_type  == VALUE_TYPE_STRING_POINTER) {
                CONTINUE_IF(element == NULL);
            }
            EXEC(src->remove(src, index, (void **)&element));
            EXEC(vector->add_back(vector, element));
            element = NULL;
        }
    } CATCH (ret) {
    }

    return ret;
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

    return 0;
}

static int 
__for_each_arg(Vector *vector, int (*func)(int index, void *element, void *arg), void *arg)
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        vector->peek_at(vector, index, (void **)&element);
        func(index++, element, arg);
    }

    return 0;
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
    String **class_name;
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
        } else if (vector->value_type  == VALUE_TYPE_STRING_POINTER) {
            object_destroy(element);
        } else if (vector->value_type  == VALUE_TYPE_ALLOC_POINTER) {
            allocator_mem_free(vector->obj.allocator, element);
        } else if (vector->value_type == VALUE_TYPE_STRUCT_POINTER) {
            class_name = vector->get(vector, "/Vector/class_name");
            if (*class_name != NULL) {
                int (*free_method)(allocator_t *, void *);
                // dbg_str(DBG_DETAIL, "vector strcut adapter class name:%s", STR2A(*class_name));
                free_method = object_get_member_of_class(STR2A(*class_name), "free");
                if (free_method == NULL) return -1;
                free_method(vector->obj.allocator, element);
                continue;
            } else if (vector->value_free_callback != NULL) {
                vector->value_free_callback(vector->obj.allocator, element);   
            } else if (vector->value_free_callback == NULL) {
                allocator_mem_free(vector->obj.allocator, element);
            } else { }
        }  else if (vector->value_type >= VALUE_TYPE_MAX_TYPE) {
            dbg_str(DBG_WARN, "not support reset unkown pointer:%d", vector->value_type);
        }
        element = NULL;
    }

    memset((void *)v->vector_head, 0, v->capacity * (v->step));
    v->count = 0;
    vector_pos_init(&v->end, 0, v);

    return 0;
}

static char *__to_json(Obj *obj)
{
    Vector *vector = (Vector *)obj;
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    int (*policy)(cjson_t *root, void *element);
    cjson_t *root;
    String *json = (String *)obj->json;
    String **class_name;
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

    if (vector->value_type > ENTRY_TYPE_MAX_TYPE) { return NULL; }

    root = cjson_create_array();

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        element = NULL;
        vector->peek_at(vector, index++, (void **)&element);
        
        if ((vector->value_type == VALUE_TYPE_OBJ_POINTER || 
             vector->value_type == VALUE_TYPE_STRING_POINTER) &&
            (element == NULL)) {continue;}

        if (vector->value_type == VALUE_TYPE_STRUCT_POINTER) {
            //因为这个是定制化的、多变的， 没法加入g_vector_to_json_policy。
            class_name = vector->get(vector, "/Vector/class_name");
            if (*class_name != NULL) {
                // dbg_str(DBG_INFO, "vector strcut adapter class name:%s", STR2A(*class_name));
                policy = object_get_member_of_class(STR2A(*class_name), "to_json");
                if (policy == NULL) return -1;
            } else {
                policy = vector->value_to_json_callback;
            }
        } else {
            policy = g_vector_to_json_policy[vector->value_type].policy;
        }
        
        if (policy == NULL) { continue; }
        ret = policy(root, element);
        if (ret < 0) { return NULL; }
    }

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static int __print(Vector *vector)
{
    Obj *obj = (Obj *)vector;
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    int (*policy)(int index, void *element);
    String **class_name;
    void *element = NULL;
    int index = 0;
    int ret;

    TRY {
        THROW_IF(vector->value_type > ENTRY_TYPE_MAX_TYPE, -1);

        for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
            !vector_pos_equal(&pos, &v->end);
            pos = next, vector_pos_next(&pos, &next)) {
            element = NULL;
            vector->peek_at(vector, index++, (void **)&element);
            
            if ((vector->value_type == VALUE_TYPE_OBJ_POINTER || 
                vector->value_type == VALUE_TYPE_STRING_POINTER) &&
                (element == NULL)) {continue;}

            if (vector->value_type == VALUE_TYPE_STRUCT_POINTER) {
                //因为这个是定制化的、多变的， 没法加入g_vector_to_json_policy。
                class_name = vector->get(vector, "/Vector/class_name");
                THROW_IF(class_name == NULL, -1);
                policy = object_get_member_of_class(STR2A(*class_name), "print");
                THROW_IF(policy == NULL, -1);
            } else {
                policy == NULL;
            }
            
            THROW_IF(policy == NULL, -1);
            EXEC(policy(index, element));
        }
    } CATCH (ret) {}

    return ret;
}

/* 
 * 如果new的时候传入了init_data, object会调用这个assign函数，但是如果数据类型是object，
 * 则assign只能默认是VALUE_TYPE_OBJ_POINTERT类型， 因为这时对象刚建好，还没配置value类型。
 * 这个限制导致vector 不支持自定义结构体、ctring在new vector的时候传入初始值。
 * 如果是自定义结构体需要指定value_type，value_new后才可以使用assign。
 */
static int __assign(Vector *vector, char *value)
{
    allocator_t *allocator = vector->obj.allocator;
    int (*new_method)(allocator_t *allocator, cjson_t *c, void **value);
    cjson_t *c, *bak = NULL;
    void *o;
    char *out;
    String *s, **sp2;
    char object_name[20] = {0};
    char *json;
    int ret = 1;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_UNDEFINED;

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
        } else { THROW(-1); }

        bak = c = cjson_parse(json);
        if (c->type & OBJECT_ARRAY) { c = c->child; }

        while (c) {
            if (c->type & CJSON_NUMBER) {
                value_type = ENTRY_TYPE_INT32_T;
                vector->add(vector, c->valueint);
            } else if (c->type & OBJECT_STRING) {
                // dbg_str(DBG_DETAIL, "vector element:%s", c->valuestring);
                value_type = VALUE_TYPE_STRING_POINTER;
                s = object_new(allocator, "String", NULL);
                s->assign(s, c->valuestring);
                vector->add(vector, s);
            } else if (c->type & OBJECT_ARRAY) {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            } else if ((c->type & CJSON_OBJECT) && (vector->value_type != VALUE_TYPE_STRUCT_POINTER)) {
                value_type = VALUE_TYPE_OBJ_POINTER;
                out = cjson_print(c);
                sp2 = vector->get(vector, "/Vector/class_name");
                THROW_IF(sp2 == NULL || *sp2 == NULL, -1);
                o = object_new(allocator, STR2A(*sp2), out);
                THROW_IF(o == NULL, -1);
                EXEC(vector->add(vector, o));
                free(out);
            } else if ((c->type & CJSON_OBJECT) && (vector->value_type == VALUE_TYPE_STRUCT_POINTER)) {
                sp2 = vector->get(vector, "/Vector/class_name");
                value_type = VALUE_TYPE_STRUCT_POINTER;
                if (*sp2 != NULL) {
                    // dbg_str(DBG_DETAIL, "vector strcut adapter class name:%s", STR2A(*sp2));
                    new_method = object_get_member_of_class(STR2A(*sp2), "new");
                    if (new_method == NULL) return -1;
                } else {
                    new_method = vector->value_new_callback;
                }
                THROW_IF(new_method == NULL, -1);
                EXEC(new_method(allocator, c->child, &o));
                vector->add(vector, o);
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

static int __search(Vector *vector, int (*cmp)(void *element, void *key), void *key, void **out, int *index) 
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element = NULL;
    int ret = 1, i = 0;

    TRY {
        for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
             !vector_pos_equal(&pos, &v->end);
             pos = next, vector_pos_next(&pos, &next)) 
        {
            vector->peek_at(vector, i++, (void **)&element);
            CONTINUE_IF(element == NULL);
            THROW_IF((ret = cmp(element, key)) < 0, -1);
            if (ret == 1) {
                *out = element;
                THROW_IF(index == NULL, 1);
                *index = i - 1;
                THROW(1);
            }
        }
        THROW(0);
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

static int __bubble_sort(Vector *vector, int (*cmp)(void *e1, void *e2))
{
    int i, j;
    void *t;
    int len = vector->get_end_index(vector);
    void **array  = vector->vector->vector_head;

    for (i = 0; i < len - 1; i++) {
        for (j = 0; j < len - 1 - i; j++) {
            if (cmp(array[j], array[j + 1])) {
                t = array[j];
                array[j] = array[j + 1];
                array[j + 1] = t;
            }
        }
    }

    return 1;
}

static int __sort(Vector *vector, enum vector_sort_type_e type, int (*cmp)(void *e1, void *e2))
{
    vector_t *v      = vector->vector;
    uint32_t end_pos = v->end.vector_pos;
    int (*sort_methods[VECTOR_SORT_MAX_TYPE])(Vector *vector, int (*cmp)(void *e1, void *e2)) = {
        [VECTOR_SORT_TYPE_BUBBLE] = __bubble_sort,
    };
    int ret;

    TRY {
        THROW_IF(type >= VECTOR_SORT_MAX_TYPE, -1);
        THROW_IF(sort_methods[type] == NULL, -1);
        EXEC(sort_methods[type](vector, cmp)); 
    } CATCH (ret) {
    }

    return ret;
}

static int __reset_from(Vector *vector, int index)
{
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    String **class_name;
    int i = 0, end;

    end = vector->get_end_index(vector);
    if (index >= end) return 0;

    for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
         !vector_pos_equal(&pos, &v->end);
         pos = next, vector_pos_next(&pos, &next)) {
        if (vector->trustee_flag != 1) { break; }
        if (i < index) continue;
        
        vector->remove(vector, i++, (void **)&element);
        if (element == NULL) { continue; }

        if (vector->value_type == VALUE_TYPE_OBJ_POINTER) {
            object_destroy(element);
        } else if (vector->value_type  == VALUE_TYPE_STRING_POINTER) {
            object_destroy(element);
        } else if (vector->value_type  == VALUE_TYPE_ALLOC_POINTER) {
            allocator_mem_free(vector->obj.allocator, element);
        } else if (vector->value_type == VALUE_TYPE_STRUCT_POINTER) {
            class_name = vector->get(vector, "/Vector/class_name");
            if (*class_name != NULL) {
                int (*free_method)(allocator_t *, void *);
                dbg_str(DBG_VIP, "vector strcut adapter class name:%s", STR2A(*class_name));
                free_method = object_get_member_of_class(STR2A(*class_name), "free");
                if (free_method == NULL) return -1;
                free_method(vector->obj.allocator, element);
                continue;
            } else if (vector->value_free_callback != NULL) {
                vector->value_free_callback(vector->obj.allocator, element);   
            } else if (vector->value_free_callback == NULL) {
                allocator_mem_free(vector->obj.allocator, element);
            } else { }
        } else if (vector->value_type >= VALUE_TYPE_MAX_TYPE) {
            dbg_str(DBG_WARN, "not support reset unkown pointer");
        }
        element = NULL;
    }
    vector_pos_init(&v->end, index, v);

    return 0;
}

static int __filter(Vector *vector, int (*condition)(void *element, void *cond), void *cond, Vector *out)
{
    vector_pos_t pos, next;
    vector_t *v = NULL;
    void *element = NULL;
    int ret, index = 0;

    TRY {
        THROW_IF(vector == NULL || out == NULL || cond == NULL, -1);
        v = vector->vector;

        for (vector_begin(v, &pos), vector_pos_next(&pos, &next);
             !vector_pos_equal(&pos, &v->end);
             pos = next, vector_pos_next(&pos, &next), index++) {
            vector->peek_at(vector, index, (void **)&element);

            if (vector->value_type == VALUE_TYPE_OBJ_POINTER || vector->value_type  == VALUE_TYPE_STRING_POINTER) {
                CONTINUE_IF(element == NULL);
            }
            THROW_IF((ret = condition(element, cond)) < 0, -1);
            CONTINUE_IF(ret == 0);
            vector->remove(vector, index, (void **)&element);
            out->add_back(out, element);
            element = NULL;
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __copy(Vector *vector, Vector *out)
{
    char *json;
    String **sp2_1, **sp2_2;
    int ret = 0;

    TRY {
        THROW_IF(vector == NULL || out == NULL, -1);

        if (vector->value_type == VALUE_TYPE_OBJ_POINTER || vector->value_type  == VALUE_TYPE_STRING_POINTER) {
            sp2_1 = vector->get(vector, "class_name");
            sp2_2 = out->get(out, "class_name");
            THROW_IF(sp2_1 == NULL || sp2_2 == NULL || *sp2_1 == NULL || *sp2_2 == NULL, -1);
            THROW_IF(strcmp(STR2A(*sp2_1), STR2A(*sp2_2)) != 0, -1);
        }

        json = vector->to_json(vector);
        out->assign(out, json);
    } CATCH (ret) {
    }

    return ret;
}

/* vector value类型如果是struct类型而且只需释放函数，则可以使用这个函数配置， 否则使用class_name 配置struct adapter。*/
static int __customize(Vector *vector, int value_type, int (*free_callback)(allocator_t *allocator, void *value))
{
    int trustee_flag = 1;

    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/value_free_callback", free_callback);

    return 1;
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
    Init_Vfunc_Entry(12, Vector, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(13, Vector, free_vector_elements, __free_vector_elements),
    Init_Vfunc_Entry(14, Vector, reset, __reset),
    Init_Vfunc_Entry(15, Vector, count, __count),
    Init_Vfunc_Entry(16, Vector, empty, __empty),
    Init_Vfunc_Entry(17, Vector, to_json, __to_json),
    Init_Vfunc_Entry(18, Vector, assign, __assign),
    Init_Vfunc_Entry(19, Vector, search, __search),
    Init_Vfunc_Entry(20, Vector, get_end_index, __get_end_index),
    Init_Vfunc_Entry(21, Vector, sort, __sort),
    Init_Vfunc_Entry(22, Vector, reset_from, __reset_from),
    Init_Vfunc_Entry(23, Vector, filter, __filter),
    Init_Vfunc_Entry(24, Vector, add_vector, __add_vector),
    Init_Vfunc_Entry(25, Vector, copy, __copy),
    Init_Vfunc_Entry(26, Vector, customize, __customize),
    Init_Vfunc_Entry(27, Vector, print, __print),
    Init_U32___Entry(28, Vector, value_size, NULL),
    Init_U8____Entry(29, Vector, value_type, NULL),
    Init_U32___Entry(30, Vector, capacity, NULL),
    Init_Str___Entry(31, Vector, class_name, NULL),
    Init_U8____Entry(32, Vector, trustee_flag, 0),
    Init_Point_Entry(33, Vector, value_free_callback, NULL),
    Init_Point_Entry(34, Vector, value_to_json_callback, NULL),
    Init_Point_Entry(35, Vector, value_new_callback, NULL),
    Init_End___Entry(36, Vector),
};
REGISTER_CLASS(Vector, vector_class_info);



static int __vector_to_json_int8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int8_t num = (int8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint8_t num = (uint8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_int16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int16_t num = (int16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint16_t num = (uint16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_int32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int32_t num = (int32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint32_t num = (uint32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_float_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    float *num = (float *)element;

    item = cjson_create_number(*num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_string_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    String *s = (String *)element;

    item = cjson_create_string(s->get_cstr(s));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_object_pointer_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    Obj *o = (Obj *)element;

    item = cjson_parse(o->to_json(o));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

vector_to_json_policy_t g_vector_to_json_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]         = {.policy = __vector_to_json_int8_policy},
    [ENTRY_TYPE_UINT8_T]        = {.policy = __vector_to_json_uint8_policy},
    [ENTRY_TYPE_INT16_T]        = {.policy = __vector_to_json_int16_policy},
    [ENTRY_TYPE_UINT16_T]       = {.policy = __vector_to_json_uint16_policy},
    [ENTRY_TYPE_INT32_T]        = {.policy = __vector_to_json_int32_policy},
    [ENTRY_TYPE_UINT32_T]       = {.policy = __vector_to_json_uint32_policy},
    [VALUE_TYPE_FLOAT_T]        = {.policy = __vector_to_json_float_policy},
    [VALUE_TYPE_STRING_POINTER] = {.policy = __vector_to_json_string_policy},
    [VALUE_TYPE_OBJ_POINTER]    = {.policy = __vector_to_json_object_pointer_policy},
};