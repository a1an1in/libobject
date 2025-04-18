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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/object.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/Object_Cache.h>

void * 
__object_get_private_func_of_class(void *class_info_addr, 
                                   char *func_pointer_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i, ret;

    TRY {
        THROW_IF(class_info_addr == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (strcmp((char *)entry[i].value_name, func_pointer_name) == 0 && 
                       entry[i].type == ENTRY_TYPE_FUNC_POINTER) {
                return entry[i].value;
            }
        }   
    } CATCH (ret) {
        dbg_str(OBJ_ERROR, "class_info_addr:%p func_pointer_name:%s",
                class_info_addr, func_pointer_name);
    }

    return NULL;
}

void * __object_get_lastest_vitual_func(void *class_info_addr,
                                        char *func_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    char *parent_class_name = NULL;
    class_info_entry_t * entry_of_parent_class;
    class_deamon_t *deamon;
    int i, ret;

    TRY {
        THROW_IF(class_info_addr == 0, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (strcmp((char *)entry[i].value_name, func_name) == 0 && entry[i].value != NULL && 
                entry[i].type == ENTRY_TYPE_VFUNC_POINTER) {
                return entry[i].value;
            }
        }   

        if (entry[0].type == ENTRY_TYPE_OBJ) {
            parent_class_name = entry[0].type_name;
            deamon = class_deamon_get_global_class_deamon();
            entry_of_parent_class  = (class_info_entry_t *) 
                                     class_deamon_search_class(deamon, (char *)parent_class_name);

            return __object_get_lastest_vitual_func(entry_of_parent_class, func_name);
        }
    } CATCH (ret) {
        dbg_str(OBJ_ERROR, "object_get_func_of_class_recursively, func_name:%s", func_name);
    }

    return NULL;
}

class_info_entry_t *__object_get_entry_of_parent_class(void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i, ret;

    TRY {
        THROW_IF(class_info_addr == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (entry[i].type == ENTRY_TYPE_OBJ) {
                return &entry[i];
            }
        }   
    } CATCH (ret) {}

    return NULL;
}

int __object_get_class_size(void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i, size = 0, ret;

    TRY {
        THROW_IF(class_info_addr == 0, 0);
        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        }   

        if (entry[i].type == ENTRY_TYPE_END) {
            size = entry[i].value_len;
        }
    } CATCH (ret) {}

    return size;
}

int __object_init_funcs(void *obj, void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i, ret = 1;
    int (*set)(void *obj, char *attrib, void *value);

    TRY {
        set = __object_get_lastest_vitual_func(class_info_addr, "set");
        THROW_IF(set == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
                entry[i].type == ENTRY_TYPE_VFUNC_POINTER) {
                dbg_str(OBJ_DETAIL, "value_name %s, value %p", entry[i].value_name, entry[i].value);
                if (entry[i].value != NULL)
                    set(obj, (char *)entry[i].value_name, entry[i].value);
            }
        }   
    } CATCH (ret) {}

    return ret;
}

int __object_inherit_funcs(void *obj, void *class_info)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info;
    int (*set)(void *obj, char *attrib, void *value);
    int i, ret = 1;
    void *method;
    char *current_class_name;

    TRY {
        set = __object_get_lastest_vitual_func(class_info, "set");
        THROW_IF(set == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (entry[i].value == NULL && entry[i].type == ENTRY_TYPE_VFUNC_POINTER) 
            {
                method = __object_get_lastest_vitual_func(class_info, entry[i].value_name);
                if (method != NULL)
                    set(obj, (char *)entry[i].value_name, method);
            }
        }   
    } CATCH (ret) {}

    return ret;
}

int __object_override_virtual_func(void *obj, 
                                   char *cur_type_name,
                                   char *type_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    int i, ret = 1;
    int (*set)(void *obj, char *attrib, void *value);
    void *reimplement_func;

    TRY {
        THROW_IF(strcmp(cur_type_name, type_name) == 0, 0);

        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *)class_deamon_search_class(deamon, (char *)cur_type_name);

        set = __object_get_lastest_vitual_func(entry, "set");
        THROW_IF(set == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (entry[i].type == ENTRY_TYPE_VFUNC_POINTER){
                reimplement_func = object_get_lastest_virtual_func(type_name, cur_type_name, entry[i].value_name);
                if (reimplement_func != NULL)
                    set(obj, (char *)entry[i].value_name, reimplement_func);
            }
        }   
    } CATCH (ret) {}

    return ret;
}

int __object_init(void *obj, char *cur_type_name, char *type_name) 
{
    class_deamon_t *deamon;
    void *class_info;
    class_info_entry_t * entry_of_parent_class; //class info entry of parent class
    int (*construct)(void *obj, char *init_str);
    Obj *o = (Obj *)obj;
    int ret = 1;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        THROW_IF(deamon == NULL, -1);

        class_info = class_deamon_search_class(deamon, (char *)cur_type_name);
        THROW_IF(class_info == NULL, -1);

        construct = __object_get_private_func_of_class(class_info, "construct");
        entry_of_parent_class = __object_get_entry_of_parent_class(class_info);

        dbg_str(OBJ_DETAIL, "obj_class addr:%p", class_info);
        if (entry_of_parent_class != NULL) {
            EXEC(__object_init(obj, entry_of_parent_class->type_name, type_name));
        }

        strcpy(o->target_name, cur_type_name);
        EXEC(__object_init_funcs(obj, class_info));
        if (entry_of_parent_class != NULL) {
            EXEC(__object_inherit_funcs(obj, class_info));
        }

        EXEC(__object_override_virtual_func(obj, cur_type_name, type_name));

        dbg_str(OBJ_DETAIL, "obj addr:%p", obj);
        /* 先执行父构造， 再执行子构造函数， 因为先从子函数开始找构造函数， 所以递归
         * 执行时要放在递归函数后面执行construct. 析构的过程与构造正好相反。*/
        if (construct != NULL) {
            EXEC(construct(obj, NULL));
        }
    } CATCH (ret) {
        dbg_str(OBJ_ERROR, "__object_init error, obj:%p, cur_type_name:%s, type_name:%s",
                obj, cur_type_name, type_name);
    }

    return ret;
}

int object_init(void *obj, char *type_name) 
{
    return __object_init(obj, type_name, type_name);
}

static int 
__object_set(void *obj, char *type_name,
             cjson_t *c, int (*set)(void *obj, char *attrib, void *value)) 
{
    class_deamon_t *deamon;
    void *class_info_addr;
    cjson_t *next;
    cjson_t *object;
    Obj *o = (Obj *)obj;
    int (*super_class_set)(void *obj, char *attrib, void *value); 
    int cnt = 0;

    while (c) {
        if (c->type & CJSON_OBJECT) {
            object = c;
            if (object->string) {
                dbg_str(OBJ_DETAIL, "set object name:%s", object->string);
                deamon          = class_deamon_get_global_class_deamon();
                class_info_addr = class_deamon_search_class(deamon, object->string);
                super_class_set = __object_get_lastest_vitual_func(class_info_addr, "set");
                dbg_str(OBJ_DETAIL, "set addr:%p", super_class_set);
            }

            if (c->child) {
                __object_set(obj,object->string, c->child, super_class_set);
            }
        } else {
            if (cnt == 0) {
                strcpy(o->target_name, type_name);
            }

            if (set) {
                if (c->type & CJSON_NUMBER) {
                    dbg_str(OBJ_DETAIL, "set number: %s value \"%d\"", c->string, c->valueint);
                    set(obj, c->string, &(c->valueint));
                    /*
                     *set(obj, c->string, &(c->valuedouble));
                     */
                } else if (c->type & CJSON_STRING) {
                    /*
                     * 这里只是识别到json 字符串， 具体到属性类型在set()判断，如果属性以字符串的形式赋值，就会走这个分支。
                     * 数组和数字目前不支持字符串形式赋值.
                     * */
                    dbg_str(OBJ_DETAIL, "set string: %s value \"%s\"", c->string, c->valuestring);
                    set(obj, c->string, c->valuestring);
                } else if (c->type & CJSON_ARRAY) {
                    char *out;
                    out = cjson_print(c);
                    set(obj, c->string, out);
                    free(out);
                }
            }
            cnt++;
        }

        c = c->next;
    }

    return 0;
}

int object_set(void *obj, char *type_name, char *set_str) 
{
    cjson_t *root;
    char init_data[MAX_OBJECT_INIT_DATA_LEN];
    int ret;

    TRY {
        if (set_str == NULL) {
            root = cjson_parse(set_str);
        } else if (strlen(set_str) > 0) {
            THROW_IF(strlen(type_name) + strlen(set_str) + 3 >= MAX_OBJECT_INIT_DATA_LEN, -1);
            snprintf(init_data, MAX_OBJECT_INIT_DATA_LEN, "{\"%s\":%s}", type_name, set_str);
            dbg_str(OBJ_DETAIL, "%s", init_data);

            root = cjson_parse(init_data);
        } else {
            root = cjson_parse(set_str);
        }

        EXEC(__object_set(obj, type_name, root, NULL));
        cjson_delete(root);
    } CATCH (ret) {}

    return ret;
}

void * object_new(allocator_t *allocator, const char *type, char *config)
{
    Obj *o = NULL;
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    char *init_data;
    int size = 0, ret, assign_flag = 0;

    TRY {
        THROW_IF(type == NULL, -1);

        if (allocator == NULL) {
            allocator = allocator_get_default_instance();
        }

        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *)class_deamon_search_class(deamon, (char *)type);

        THROW_IF(entry == NULL, -1);
        THROW_IF((size = __object_get_class_size(entry)) == 0, -1);
        THROW_IF((o = (Obj *)allocator_mem_alloc(allocator, size)) == NULL, -1);

        // allocator_save_upper_nlayer_name(allocator, 7, o);
        memset(o, 0, size);
        o->allocator = allocator;
        strcpy(o->name, type);

        /* 因为String和Vector没有太多配置， 为了简化传入的配置，这里直接把config作为init data， 这样
         * 可以直接传入对应类型的init数据就可以得到初始化的vector了， 但是这里需要注意， 如果json传入的
         * 是对象， vector会把它当作object来处理。 如果你期望当作struct来处理， 只有后面传入了定制struct
         * 方法后才能实现。 因为如果这里你希望当作struct，但是vector更不不知道struct怎么来构造。vector知道
         * object怎么构造是因为init data传入了对象类型，可以根据对象类型名去查询相应的class_info_entry_t，
         * 但是struct没有相应的实现。
         **/
        if (config != NULL && (strcmp(type, "String") == 0 || strcmp(type, "Vector") == 0)) {
            init_data = config;
            config = NULL;
            assign_flag = 1;
        }
        
        /* 需要先配置，再初始化，初始化会依赖前面的配置 */
        EXEC(object_set(o, (char *)type, config));
        EXEC(object_init(o, (char *)type));

        if (assign_flag) {
            o->assign(o, init_data);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "object_new error, type:%s", type);
        allocator_mem_free(allocator, o);
        o = NULL;
    }

    return o;
}

int __object_dump(void *obj, char *type_name, cjson_t *object) 
{
    class_deamon_t *deamon;
    class_info_entry_t *entry;
    void *(*get)(void *obj, char *attrib);
    int len, i;
    cjson_t *item;
    void *value;
    char *name;
    Obj *o = (Obj *)obj;

    deamon = class_deamon_get_global_class_deamon();
    entry  = (class_info_entry_t *)
             class_deamon_search_class(deamon,
                                       (char *)type_name);

    get = __object_get_lastest_vitual_func(entry, (char *)"get");

    if (get == NULL) {
        dbg_str(OBJ_WARN, "get func pointer is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_OBJ){
            item = cjson_create_object();
            cjson_add_item_to_object(object, entry[i].type_name, item);
            __object_dump(obj, entry[i].type_name, item);
        } else if (entry[i].type == ENTRY_TYPE_FUNC_POINTER  || 
                   entry[i].type == ENTRY_TYPE_VFUNC_POINTER) {
        } else {
            strcpy(o->target_name, type_name);
            dbg_str(OBJ_WARN, "get:%p, __get:%p", get, o->get);
            value = get(obj, entry[i].value_name);
            name = entry[i].value_name;
            if (entry[i].type == ENTRY_TYPE_INT8_T || 
                entry[i].type == ENTRY_TYPE_UINT8_T) {
                cjson_add_number_to_object(object, name, *((char *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT16_T || 
                       entry[i].type == ENTRY_TYPE_UINT16_T) {
                cjson_add_number_to_object(object, name, *((short *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT32_T ||
                       entry[i].type == ENTRY_TYPE_UINT32_T) {
                cjson_add_number_to_object(object, name, *((int *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT64_T || 
                       entry[i].type == ENTRY_TYPE_UINT64_T) {
            } else if (entry[i].type == ENTRY_TYPE_FLOAT_T) {
                cjson_add_number_to_object(object, name, *((float *)value));
            } else if (entry[i].type == ENTRY_TYPE_STRING) {
                String *s = *(String **)value;
                if (s != NULL)
                    cjson_add_string_to_object(object, name, s->value);
            /*
             *} else if (entry[i].type == ENTRY_TYPE_NORMAL_POINTER ||
             *           entry[i].type == ENTRY_TYPE_OBJ_POINTER) {
             *    unsigned long long d = (unsigned long long) value;
             *    cjson_add_number_to_object(object, name, d);
             */
            } else {
                dbg_str(OBJ_WARN, "type error, please check, type name :%s, entry name :%s, type =%d", 
                        type_name, entry[i].type_name, entry[i].type);
            }
        }
    }   

    return 0;
}

int object_dump(void *obj, char *type_name, char *buf, int max_len) 
{
    cjson_t *root;
    cjson_t *item;
    char *out;
    int len;

    root = cjson_create_object();
    item = cjson_create_object();
    cjson_add_item_to_object(root, type_name, item);

    __object_dump(obj, type_name, item);

    out = cjson_print(root);
    len = strlen(out);
    len = len > max_len ? max_len: len; 
    strncpy(buf, out, len);

    strncpy(buf, out, max_len);
    cjson_delete(root);
    free(out);
}

int __object_destroy(void *obj, char *type_name) 
{
    class_deamon_t *deamon;
    void *class_info, *parent_class_info;
    class_info_entry_t * entry_of_parent_class;
    int (*deconstruct)(void *obj);

    deamon      = class_deamon_get_global_class_deamon();
    class_info  = class_deamon_search_class(deamon, (char *)type_name);
    deconstruct = __object_get_private_func_of_class(class_info, "deconstruct");
    entry_of_parent_class = __object_get_entry_of_parent_class(class_info);

    /* 先执行子类析构，再执行父类析构 */
    if (deconstruct != NULL) 
        deconstruct(obj);

    if (entry_of_parent_class == NULL) {
        return 0;
    } else {
        __object_destroy(obj, entry_of_parent_class->type_name);
    }

    return 0;
}

int object_destroy(void *obj) 
{
    Obj *o = (Obj *)obj;
    Object_Cache *cache;
    List *list = NULL;
    int ret = 0;
    
    TRY {
        THROW_IF(obj == NULL, 0);

        cache = (Object_Cache *)o->cache;
        if (cache) {
            Map *map = cache->class_map;

            ret = map->search(map, o->name, (void **)&list);
            THROW_IF(ret != 1, -1);
            dbg_str(OBJ_INFO, "object destroy cache, obj name:%s", o->name);
            o->reset(o);
            list->add(list, o);
        } else {
            __object_destroy(o, o->name);
        }
    } CATCH (ret) {}

    return ret;
}

static int 
__object_override_virtual_funcs(void *obj, char *cur_class_name, char *func_name, void *value)
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

        set = __object_get_lastest_vitual_func(class_info, "set");
        THROW_IF(set == NULL, -1);

        entry_of_parent_class = __object_get_entry_of_parent_class(class_info);
        if (entry_of_parent_class != NULL) {
            __object_override_virtual_funcs(obj, entry_of_parent_class->type_name, func_name, value); 
        }

        strcpy(o->target_name, cur_class_name);
        set(obj, func_name, value); 
    } CATCH (ret) {}

    return ret;
}

int object_override(void *obj, char *func_name, void *value)
{
    return __object_override_virtual_funcs(obj, ((Obj *)obj)->name, func_name, value); 
}

void *object_get_lastest_virtual_func(char *start_class_name, char *end_class_name, char *method_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    char *super_class_name = NULL;
    int i, ret = 1;

    TRY {
        THROW_IF(start_class_name == NULL, -1);

        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *)class_deamon_search_class(deamon, 
                 (char *)start_class_name);
        if (entry[0].type == ENTRY_TYPE_OBJ) {
            super_class_name = entry[0].type_name;
        }
        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if ((entry[i].type == ENTRY_TYPE_VFUNC_POINTER) && 
                strcmp(entry[i].value_name, method_name) == 0) {
                if (entry[i].value == NULL) {
                    break;
                } else {
                    return entry[i].value;
                }
            }
        }   

        THROW_IF(strcmp(start_class_name, end_class_name) == 0, 0);

        return object_get_lastest_virtual_func(super_class_name, end_class_name, method_name);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "method_name:%s, start_class_name:%s, end_class_name:%s",
                method_name, start_class_name, end_class_name);
    }

    return NULL;
}

void *object_get_progeny_class_first_private_func(char *sub_class_name, char *root_class_name, char *func_name)
{
    class_info_entry_t *entry = NULL;
    class_deamon_t *deamon;
    void *func = NULL;
    int i, ret;

    TRY {
        THROW_IF(root_class_name == NULL, -1);

        deamon = class_deamon_get_global_class_deamon();
        entry = (class_info_entry_t *)class_deamon_search_class(deamon, sub_class_name);
        if ((strcmp(sub_class_name, root_class_name) == 0)) {
            return NULL;
        }

        if (entry[0].type == ENTRY_TYPE_OBJ) {
            func = object_get_progeny_class_first_private_func(entry[0].type_name, root_class_name, func_name);
            if (func != NULL) {
                return func;
            }
        }

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (strcmp((char *)entry[i].value_name, func_name) == 0 &&
                entry[i].value != NULL &&
                (entry[i].type == ENTRY_TYPE_FUNC_POINTER)) {
                return entry[i].value;
            }
        }  
    } CATCH (ret) {
        dbg_str(OBJ_ERROR, "object_get_progeny_class_first_private_func, func_name:%s", func_name);
    }

    return NULL;
}

class_info_entry_t *__object_get_entry_of_class(void *class_info, char *entry_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info;
    int i, ret;

    TRY {
        THROW_IF(class_info == NULL || entry_name == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (entry[i].value_name == NULL) continue;
            if (strcmp(entry[i].value_name, entry_name) == 0) {
                return &entry[i];
            }
        }   
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "object_get_entry_of_class error, class_info:%p, entry_name:%s",
                class_info, entry_name);
    }

    return NULL;
}

class_info_entry_t *object_get_entry_of_class(char *class_name, char *entry_name)
{
    class_info_entry_t * info, *entry = NULL;
    class_deamon_t *deamon;
    int ret = 1;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        info = (class_info_entry_t *) class_deamon_search_class(deamon, class_name);
        THROW_IF(info == NULL, -1);
        entry = __object_get_entry_of_class(info, entry_name);
        THROW_IF(entry == NULL, NULL);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "class_name:%s, entry_name:%s", class_name, entry_name);
    }

    return entry;
}

void *object_get_member_of_class(char *class_name, char *func_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    int i, ret = 0;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        entry = (class_info_entry_t *) class_deamon_search_class(deamon, 
                class_name);
        SET_CATCH_STR_PARS(class_name, class_name);
        THROW_IF(entry == NULL, NULL);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            if (strcmp((char *)entry[i].value_name, func_name) == 0) {
                dbg_str(OBJ_DETAIL, "found func of class");
                return entry[i].value;
            }
        }
    } CATCH (ret) {
        dbg_str(OBJ_ERROR, "object_get_member_of_class, class_name:%s func_name:%s",
                class_name, func_name);
    }

    return NULL;
}

void * object_get_lastest_vitual_func(void *class_name, char *func_name)
{
    class_deamon_t *deamon;
    class_info_entry_t *entry;
    int ret;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *) 
                 class_deamon_search_class(deamon, (char *)class_name);
        THROW_IF(entry == NULL, -1);

        return __object_get_lastest_vitual_func(entry, func_name);
    } CATCH (ret) {} 

    return NULL;
}