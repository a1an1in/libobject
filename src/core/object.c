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

void * __object_get_func_of_class(char *class_name,
                                  char *func_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    int i;

    dbg_str(OBJ_DETAIL, "object_get_func_of_class");
    deamon = class_deamon_get_global_class_deamon();
    entry = (class_info_entry_t *) class_deamon_search_class(deamon, 
                                                             class_name);
    if (entry == NULL) return NULL;

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (strcmp((char *)entry[i].value_name, func_name) == 0) {
            dbg_str(OBJ_DETAIL, "found func of class");
            return entry[i].value;
        }
        dbg_str(OBJ_DETAIL, "value_name:%s func_name:%s", entry[i].value_name, func_name);
    }   
    dbg_str(OBJ_DETAIL, "not found func of class");

    return NULL;
}

void * 
__object_get_normal_func_of_class(void *class_info_addr, 
                                  char *func_pointer_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;

    if (class_info_addr == 0) {
        /*
         *dbg_str(OBJ_WARNNING, "object_get_func:%s, class_info_addr is nil",
         *        func_pointer_name);
         */
        return NULL;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (strcmp((char *)entry[i].value_name, func_pointer_name) == 0 && 
            entry[i].type == ENTRY_TYPE_FUNC_POINTER) {
            return entry[i].value;
        }
    }   

    return 0;
}

void * 
__object_get_func_of_class_recursively(void *class_info_addr,
                                       char *func_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    char *parent_class_name = NULL;
    class_info_entry_t * entry_of_parent_class;
    class_deamon_t *deamon;
    int i;

    if (class_info_addr == 0) {
        return NULL;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (strcmp((char *)entry[i].value_name, func_name) == 0 && 
            entry[i].value != NULL) {
            return entry[i].value;
        }
    }   

    if (entry[0].type == ENTRY_TYPE_OBJ) {
        parent_class_name = entry[0].type_name;
        deamon = class_deamon_get_global_class_deamon();
        entry_of_parent_class  = (class_info_entry_t *)
                                 class_deamon_search_class(deamon,
                                                           (char *)parent_class_name);

        return __object_get_func_of_class_recursively(entry_of_parent_class,
                                                      func_name);
    }

    return NULL;
}


class_info_entry_t *
__object_get_entry_of_parent_class(void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;

    if (class_info_addr == 0) {
        /*
         *dbg_str(OBJ_WARNNING, "object_get_entry_of_parent_class:%s, class_info_addr is nil",
         *        func_pointer_name);
         */
        return NULL;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_OBJ) {
            return &entry[i];
        }
    }   

    return NULL;
}

class_info_entry_t *
__object_get_entry_of_class(void *class_info, char *entry_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info;
    int i;

    if (class_info == 0) {
        return NULL;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (strcmp(entry[i].value_name, entry_name) == 0) {
            return &entry[i];
        }
    }   

    return NULL;
}

int 
__object_get_class_size(void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i, size = 0;

    if (class_info_addr == 0) {
        return size;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
    }   

    if (entry[i].type == ENTRY_TYPE_END) {
        size = entry[i].value_len;
    }

    return size;
}

void *
__object_find_reimplement_func(char *method_name, 
                               char *start_type_name, 
                               char *end_type_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    char *super_class_name = NULL;
    int i;

    if (strcmp(start_type_name, end_type_name) == 0) return NULL;
    if (start_type_name == NULL) {
        dbg_str(OBJ_WARNNING, "__object_find_reimplement_func, start addr is NULL");
        return NULL;
    }

    deamon = class_deamon_get_global_class_deamon();
    entry  = (class_info_entry_t *)class_deamon_search_class(deamon, 
                                                             (char *)start_type_name);
    if (entry[0].type == ENTRY_TYPE_OBJ) {
        super_class_name = entry[0].type_name;
    }
    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if ((entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
                entry[i].type == ENTRY_TYPE_VFUNC_POINTER) && 
            strcmp(entry[i].value_name, method_name) == 0)
        {
            if (entry[i].value == NULL) {
                break;
            } else {
                return entry[i].value;
            }
        }

    }   

    return __object_find_reimplement_func(method_name, 
                                          super_class_name, 
                                          end_type_name);
}

int __object_init_funcs(void *obj, void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;
    int (*set)(void *obj, char *attrib, void *value);

    set = __object_get_func_of_class_recursively(class_info_addr, "set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING, "obj_init_func_pointer, set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
            entry[i].type == ENTRY_TYPE_VFUNC_POINTER)
        {
            dbg_str(OBJ_DETAIL, "value_name %s, value %p", 
                    entry[i].value_name, 
                    entry[i].value);

            if (entry[i].value != NULL)
                set(obj, (char *)entry[i].value_name, entry[i].value);
        }
    }   

    return 0;
}


int 
__object_inherit_funcs(void *obj, void *class_info)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info;
    int (*set)(void *obj, char *attrib, void *value);
    int i;
    void *method;
    char *current_class_name;

    set = __object_get_func_of_class_recursively(class_info, "set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING, "obj_init_func_pointer, set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].value == NULL && 
            (entry[i].type == ENTRY_TYPE_IFUNC_POINTER || 
             entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
             entry[i].type == ENTRY_TYPE_VFUNC_POINTER)) 
        {
            method = __object_get_func_of_class_recursively(class_info,
                                                  entry[i].value_name);
            if (method != NULL)
                set(obj, (char *)entry[i].value_name, method);
        }
    }   

    return 0;
}


int __object_override_virtual_funcs(void *obj, 
                                   char *cur_type_name, 
                                   char *type_name)
{
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    int i;
    int (*set)(void *obj, char *attrib, void *value);
    void *reimplement_func;

    if (strcmp(cur_type_name, type_name) == 0) return 0;

    deamon = class_deamon_get_global_class_deamon();
    entry  = (class_info_entry_t *)
             class_deamon_search_class(deamon, (char *)cur_type_name);

    set    = __object_get_func_of_class_recursively(entry, "set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING, "obj_init_func_pointer, set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_VFUNC_POINTER){
            reimplement_func = __object_find_reimplement_func(entry[i].value_name, 
                                                              type_name, 
                                                              cur_type_name);
            if (reimplement_func != NULL)
                set(obj, (char *)entry[i].value_name, reimplement_func);
        }
    }   

    return 0;
}

void * object_new(allocator_t *allocator,
                  char *type, char *config)
{
    Obj *o;
    class_info_entry_t *entry;
    class_deamon_t *deamon;
    char *init_data;
    int size = 0, ret, assign_flag = 0;

    if (type == NULL) return NULL;
    if (allocator == NULL) {
        allocator = allocator_get_default_alloc();
    }

    deamon = class_deamon_get_global_class_deamon();
    entry  = (class_info_entry_t *)
             class_deamon_search_class(deamon, type);

    size   = __object_get_class_size(entry);
    if (size == 0) return NULL;

    o = (Obj *)allocator_mem_alloc(allocator, size);
    if (o == NULL) {
        dbg_str(OBJ_ERROR, "alloc mem failed");
        return NULL;
    } else {
        memset(o, 0, size);
        o->allocator = allocator;
        strcpy(o->name, type);
    }

    if (config != NULL && (strcmp(type, "String") == 0 ||
                           strcmp(type, "Vector") == 0)) 
    {
        init_data = config;
        config = NULL;
        assign_flag = 1;
    }

    ret = object_set(o, type, config);
    if (ret < 0) {
        dbg_str(OBJ_ERROR, "object set failed");
        goto err_object_set;
    }

    ret = object_init(o, type);
    if (ret < 0) {
        dbg_str(OBJ_ERROR, "object init failed");
        goto err_object_init;
    }

    if (assign_flag) {
        o->assign(o, init_data);
    }

    goto end;

err_object_init:
err_object_set:
    allocator_mem_free(allocator, o);
end:

    return o;
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
                super_class_set = __object_get_func_of_class_recursively(class_info_addr, "set");
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
                } else if (c->type & OBJECT_STRING) {
                    dbg_str(OBJ_DETAIL, "set string: %s value \"%s\"", c->string, c->valuestring);
                    set(obj, c->string, c->valuestring);
                } else if (c->type & OBJECT_ARRAY) {
                    char *out;
                    out = cjson_print(c);
                    /*
                     *dbg_str(OBJ_DETAIL, "%s array json:%s",c->string, out);
                     */
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

    if (set_str == NULL) {
        root = cjson_parse(set_str);
    } else if (strlen(set_str) > 0) {
#define MAX_OBJECT_INIT_DATA_LEN 1024 * 5
        char init_data[MAX_OBJECT_INIT_DATA_LEN];

        if (strlen(type_name) + strlen(set_str) + 3 >= MAX_OBJECT_INIT_DATA_LEN) {
            dbg_str(DBG_ERROR, "set string is too long, you may change MAX_OBJECT_INIT_DATA_LEN's default value");
            return -1;
        }
        snprintf(init_data, MAX_OBJECT_INIT_DATA_LEN, "{\"%s\":%s}", type_name, set_str);
        dbg_str(OBJ_DETAIL, "%s", init_data);

        root = cjson_parse(init_data);
#undef MAX_OBJECT_INIT_DATA_LEN
    } else {
        root = cjson_parse(set_str);
    }

    __object_set(obj, type_name, root, NULL);
    cjson_delete(root);

    return 0;
}

int __object_init(void *obj, char *cur_type_name, char *type_name) 
{
    class_deamon_t *deamon;
    void *class_info;
    class_info_entry_t * entry_of_parent_class; //class info entry of parent class
    int (*construct)(void *obj, char *init_str);
    Obj *o = (Obj *)obj;

    /*
     *dbg_str(OBJ_WARNNING, "current obj type name =%s", cur_type_name);
     */

    deamon = class_deamon_get_global_class_deamon();
    if (deamon == NULL) {
        return -1;
    }

    class_info = class_deamon_search_class(deamon, (char *)cur_type_name);
    if (class_info == NULL) {
        return -1;
    }

    construct = __object_get_normal_func_of_class(class_info, "construct");
    entry_of_parent_class = __object_get_entry_of_parent_class(class_info);

    dbg_str(OBJ_DETAIL, "obj_class addr:%p", class_info);

    if (entry_of_parent_class != NULL) {
        /*
         *dbg_str(OBJ_DETAIL, "init subclass");
         */
        __object_init(obj, entry_of_parent_class->type_name, type_name);
    } else {
        dbg_str(OBJ_DETAIL, "obj has not subclass");
    }

    strcpy(o->target_name, cur_type_name);

    __object_init_funcs(obj, class_info);

    if (entry_of_parent_class != NULL) {
        __object_inherit_funcs(obj, class_info);
    }

    __object_override_virtual_funcs(obj, cur_type_name, type_name);


    dbg_str(OBJ_DETAIL, "obj addr:%p", obj);
    if (construct != NULL)
        construct(obj, NULL);
    else{
        /*
         *dbg_str(OBJ_WARNNING, "%s construct is NULL", cur_type_name);
         */
    }

    return 0;
}

int object_init(void *obj, char *type_name) 
{
    return __object_init(obj, type_name, type_name);
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

    get = __object_get_func_of_class_recursively(entry, (char *)"get");

    if (get == NULL) {
        dbg_str(OBJ_WARNNING, "get func pointer is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_OBJ){
            item = cjson_create_object();
            cjson_add_item_to_object(object, entry[i].type_name, item);
            __object_dump(obj, entry[i].type_name, item);
        } else if (entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
                   entry[i].type == ENTRY_TYPE_VFUNC_POINTER || 
                   entry[i].type == ENTRY_TYPE_IFUNC_POINTER) 
        {
        } else {
            strcpy(o->target_name, type_name);

            dbg_str(OBJ_WARNNING, "get:%p, __get:%p", get, o->get);
            value = get(obj, entry[i].value_name);
            /*
             *if (value == NULL) continue;
             */
            name = entry[i].value_name;
            if (entry[i].type == ENTRY_TYPE_INT8_T || 
                entry[i].type == ENTRY_TYPE_UINT8_T)
            {
                cjson_add_number_to_object(object, name, *((char *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT16_T || 
                       entry[i].type == ENTRY_TYPE_UINT16_T)
            {
                cjson_add_number_to_object(object, name, *((short *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT32_T ||
                       entry[i].type == ENTRY_TYPE_UINT32_T) 
            {
                cjson_add_number_to_object(object, name, *((int *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT64_T || 
                       entry[i].type == ENTRY_TYPE_UINT64_T)
            {
            } else if (entry[i].type == ENTRY_TYPE_FLOAT_T) {
                cjson_add_number_to_object(object, name, *((float *)value));
            } else if (entry[i].type == ENTRY_TYPE_STRING) {
                String *s = *(String **)value;
                if (s != NULL)
                    cjson_add_string_to_object(object, name, s->value);
            /*
             *} else if (entry[i].type == ENTRY_TYPE_NORMAL_POINTER ||
             *           entry[i].type == ENTRY_TYPE_OBJ_POINTER) 
             *{
             *    unsigned long long d = (unsigned long long) value;
             *    cjson_add_number_to_object(object, name, d);
             */
            } else {
                dbg_str(OBJ_WARNNING, "type error, please check, type name :%s, entry name :%s, type =%d", 
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
    deconstruct = __object_get_normal_func_of_class(class_info, "deconstruct");
    entry_of_parent_class = __object_get_entry_of_parent_class(class_info);

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
    Object_Cache *cache = o->cache;
    List *list = NULL;
    int ret = 0;
    
    if (o->cache) {
        Map *map = cache->class_map;

        ret = map->search(map, o->name, (void **)&list);
        if (ret != 1) {
            dbg_str(OBJ_ERROR, "release object %s to cache, but not found class in the cache, cache addr:%p, class count=%d",
                    o->name, o->cache, map->count(map));
            ret = -1;
            goto end;
        }

        dbg_str(OBJ_DETAIL, "obj destroy, add obj %s to cache", o->name);

        o->reset(o);
        list->add(list, o);
    } else {
        __object_destroy(o, o->name);
    }

end:
    return ret;
}

static int test_object_new() 
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *parent;
    char *test = "abcdefg";
    int ret;

    parent = object_new(allocator, "String", NULL);
    parent->assign(parent, test);  

    if (strcmp(parent->get_cstr(parent), test) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(parent);

    return ret;
}
REGISTER_TEST_FUNC(test_object_new);

