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
#include <libdbg/debug.h>
#include <libobject/object.h>

int str_split(char *str, char *delim, char **out, int *cnt) 
{
    int index = 0;
    char *ptr = NULL;

    while((*(out + index) = strtok_r(str, delim, &ptr)) != NULL) {  
        str = NULL;  
        /*
         *printf("addr:%p, %s\n",out[index], out[index]);  
         */
        index++;
    }  

    return *cnt = index;
}

int compute_slash_count(char *path)
{
    int i, len = strlen(path), cnt = 0;

    for (i = 0; i < len; i++) {
        if (path[i] == '/') {
            cnt++;
        }
    }

    return cnt;
}

int object_config(char *config, int len, char *path, int type, char *name, void *value) 
{
    allocator_t *allocator = allocator_get_default_alloc();
    cjson_t *root, *object, *item;
    char *buf;  
    char **out;  
    char *p;
    int cnt, j, ret = 0;

    buf = allocator_mem_alloc(allocator, strlen(path));
    if (buf == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        return -1;
    }
    strcpy(buf, path);

    cnt = compute_slash_count(path);
    out = allocator_mem_alloc(allocator, sizeof(char *) * cnt);
    if (out == NULL) {
        dbg_str(OBJ_WARNNING, "oss set alloc err");
        allocator_mem_free(allocator, buf);
        return -1;
    }

    str_split(buf, "/", out, &cnt);

    if (strlen(config) != 0) {
        object = cjson_parse(config);
    } else {
        object = cjson_create_object();
    }

    root = object;

    for (j = 0; j < cnt; j++)  {     
        item = cjson_get_object_item(object, out[j]);
        if (item != NULL) {
            object = item;
        } else {
            item = cjson_create_object();
            cjson_add_item_to_object(object, out[j],item);
            object = item;
        }
    } 

    switch(type) {
        case OBJECT_FALSE:
            break;
        case OBJECT_NUMBER:
            cjson_add_number_to_object(item, name, *((int *)value));
            break;
        case OBJECT_STRING:
            cjson_add_string_to_object(item, name, (char *)value);
            break;
        default:
            break;
    }

    p = cjson_print(root);

    if (strlen(p) > len) {
        dbg_str(OBJ_WARNNING,"config buffer is too small");
        ret = -1;
        goto err;
    } else {
        strcpy(config, p);
    }

err:
end:
    allocator_mem_free(allocator, buf);
    allocator_mem_free(allocator, out);

    free(p);
    cjson_delete(root);

    return ret;
}

void * object_get_func_pointer(void *class_info_addr, char *func_pointer_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (    strcmp((char *)entry[i].value_name, func_pointer_name) == 0 && 
                entry[i].type == ENTRY_TYPE_FUNC_POINTER) {
            return entry[i].value;
        }
    }   

    return 0;
}

class_info_entry_t *
object_get_entry_of_parent_class(void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if ( entry[i].type == ENTRY_TYPE_OBJ) {
            return &entry[i];
        }
    }   

    return NULL;
}

int object_init_func_pointer(void *obj,void *class_info_addr)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info_addr;
    int i;
    int (*set)(void *obj, char *attrib, void *value);

    set = object_get_func_pointer(class_info_addr,"set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING,"obj_init_func_pointer,set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (    entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
                entry[i].type == ENTRY_TYPE_VFUNC_POINTER)
        {
            /*
             *dbg_str(OBJ_DETAIL,"value_name %s, value %p",
             *        entry[i].value_name,
             *        entry[i].value);
             */

            if (entry[i].value != NULL)
                set(obj, (char *)entry[i].value_name, entry[i].value);
        }
    }   

    return 0;
}

void *
object_find_method_to_inherit(char *method_name,
                              void *class_name)
{
    class_info_entry_t *entry;
    object_deamon_t *deamon;
    char *parent_class_name = NULL;
    class_info_entry_t * entry_of_parent_class; //class info entry of parent class
    int i;

    if (class_name == NULL) return NULL;

    deamon = object_deamon_get_global_object_deamon();
    entry  = (class_info_entry_t *)object_deamon_search_class(deamon,
                                                              (char *)class_name);
    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (    (entry[i].type == ENTRY_TYPE_FUNC_POINTER ||
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

    if (entry[0].type == ENTRY_TYPE_OBJ) {
        parent_class_name = entry[0].type_name;
    }

    return object_find_method_to_inherit(method_name,
                                            parent_class_name);
}

int object_inherit_methods(void *obj,void *class_info,void *parent_class_name)
{
    class_info_entry_t *entry = (class_info_entry_t *)class_info;
    int (*set)(void *obj, char *attrib, void *value);
    int i;
    void *method;

    if (parent_class_name == NULL) return 0; 

    set = object_get_func_pointer(class_info,"set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING,"obj_init_func_pointer,set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_IFUNC_POINTER) {
            method = object_find_method_to_inherit(entry[i].value_name,
                                                   parent_class_name);
            if (method != NULL)
                set(obj, (char *)entry[i].value_name, method);
        }
    }   
}

void *
object_find_reimplement_func_pointer(char *method_name,
                                     char *start_type_name,
                                     char *end_type_name)
{
    class_info_entry_t *entry;
    object_deamon_t *deamon;
    char *subclass_name = NULL;
    int i;

    if (strcmp(start_type_name,end_type_name) == 0) return NULL;
    if (start_type_name == NULL) {
        dbg_str(OBJ_WARNNING,"object_find_reimplement_func_pointer, start addr is NULL");
        return NULL;
    }

    deamon = object_deamon_get_global_object_deamon();
    entry  = (class_info_entry_t *)object_deamon_search_class(deamon,
                                                              (char *)start_type_name);
    if (entry[0].type == ENTRY_TYPE_OBJ) {
        subclass_name = entry[0].type_name;
    }
    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (    entry[i].type == ENTRY_TYPE_FUNC_POINTER &&
                strcmp(entry[i].value_name, method_name) == 0)
        {
            if (entry[i].value == NULL) {
                break;
            } else {
                return entry[i].value;
            }
        }

    }   

    return object_find_reimplement_func_pointer(method_name,
                                                subclass_name,
                                                end_type_name);
}

int object_cover_vitual_func_pointer(void *obj,
                                     char *cur_type_name,
                                     char *type_name)
{
    class_info_entry_t *entry;
    object_deamon_t *deamon;
    int i;
    int (*set)(void *obj, char *attrib, void *value);
    void *reimplement_func;

    if (strcmp(cur_type_name,type_name) == 0) return 0;

    deamon = object_deamon_get_global_object_deamon();
    entry  = (class_info_entry_t *)
             object_deamon_search_class(deamon, (char *)cur_type_name);

    set    = object_get_func_pointer(entry,"set");
    if (set == NULL) {
        dbg_str(OBJ_WARNNING,"obj_init_func_pointer,set func is NULL");
        return -1;
    }

    for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
        if (entry[i].type == ENTRY_TYPE_VFUNC_POINTER){
            reimplement_func = object_find_reimplement_func_pointer(entry[i].value_name,
                                                                    type_name,
                                                                    cur_type_name);
            if (reimplement_func != NULL)
                set(obj, (char *)entry[i].value_name, reimplement_func);
        }
    }   

    return 0;
}

static int __object_set(void *obj,
                        cjson_t *c,
                        int (*set)(void *obj, char *attrib, void *value)) 
{
    object_deamon_t *deamon;
    void *class_info_addr;
    cjson_t *next;
    cjson_t *object;
    int (*sub_set)(void *obj, char *attrib, void *value); 

    while (c) {
        if (c->type & CJSON_OBJECT) {
            object = c;
            if (object->string) {
                dbg_str(OBJ_DETAIL,"object name:%s",object->string);
                deamon          = object_deamon_get_global_object_deamon();
                class_info_addr = object_deamon_search_class(deamon,object->string);
                sub_set         = object_get_func_pointer(class_info_addr,"set");
            }

            if (c->child) {
                __object_set(obj,c->child, sub_set);
            }
        } else {
            if (set) {
                /*
                 *dbg_str(OBJ_DETAIL,"object name %s,set %s",object->string, c->string);
                 */
                if (c->type & CJSON_NUMBER) {
                    set(obj,c->string,&(c->valueint));
                    /*
                     *set(obj,c->string,&(c->valuedouble));
                     */
                } else if (c->type & OBJECT_STRING) {
                    set(obj,c->string,c->valuestring);
                }
            }
        }

        c = c->next;
    }

}
int object_set(void *obj, char *type_name, char *set_str) 
{
    cjson_t *root;

    dbg_str(OBJ_DETAIL,"%s",set_str);

    root = cjson_parse(set_str);

    /*
     *set_str = cjson_print(root);
     *printf("%s",set_str);
     */

    __object_set(obj, root,NULL);
    cjson_delete(root);

    return 0;
}

int __object_dump(void *obj, char *type_name, cjson_t *object) 
{
    object_deamon_t *deamon;
    class_info_entry_t *entry;
    void *(*get)(void *obj, char *attrib);
    int len;
    int i;
    cjson_t *item;
    void *value;
    char *name;

    deamon = object_deamon_get_global_object_deamon();
    entry  = (class_info_entry_t *)object_deamon_search_class(deamon,(char *)type_name);
    get    = object_get_func_pointer(entry,(char *)"get");
    if (get == NULL) {
        dbg_str(OBJ_WARNNING,"get func pointer is NULL");
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
            value = get(obj,entry[i].value_name);
            /*
             *if (value == NULL) continue;
             */
            name = entry[i].value_name;
            if (entry[i].type == ENTRY_TYPE_INT8_T || entry[i].type == ENTRY_TYPE_UINT8_T){
                cjson_add_number_to_object(object, name, *((char *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT16_T || entry[i].type == ENTRY_TYPE_UINT16_T) {
                cjson_add_number_to_object(object, name, *((short *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT32_T || entry[i].type == ENTRY_TYPE_UINT32_T) {
                cjson_add_number_to_object(object, name, *((int *)value));
            } else if (entry[i].type == ENTRY_TYPE_INT64_T || entry[i].type == ENTRY_TYPE_UINT64_T) {
            } else if (entry[i].type == ENTRY_TYPE_FLOAT_T) {
                cjson_add_number_to_object(object, name, *((float *)value));
            } else if (entry[i].type == ENTRY_TYPE_STRING) {
                cjson_add_string_to_object(object, name, (char *)value);
            } else if (entry[i].type == ENTRY_TYPE_NORMAL_POINTER ||
                      entry[i].type == ENTRY_TYPE_FUNC_POINTER || 
                      entry[i].type == ENTRY_TYPE_VFUNC_POINTER ||
                      entry[i].type == ENTRY_TYPE_IFUNC_POINTER ||
                      entry[i].type == ENTRY_TYPE_OBJ_POINTER) 
            {
                unsigned long long d = (unsigned long long) value;
                cjson_add_number_to_object(object, name,d);
            } else {
                dbg_str(OBJ_WARNNING,"type error,please check,type name :%s,entry name :%s,type =%d",
                        type_name, entry[i].type_name, entry[i].type);
            }
        }
    }   
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
    strncpy(buf,out,len);

    strncpy(buf,out,max_len);
    cjson_delete(root);
    free(out);
}

int __object_init(void *obj, char *cur_type_name, char *type_name) 
{
    object_deamon_t *deamon;
    void *class_info;
    class_info_entry_t * entry_of_parent_class; //class info entry of parent class
    int (*construct)(void *obj,char *init_str);

    dbg_str(OBJ_DETAIL,"current obj type name =%s",cur_type_name);

    deamon                = object_deamon_get_global_object_deamon();
    class_info            = object_deamon_search_class(deamon,(char *)cur_type_name);
    construct             = object_get_func_pointer(class_info,"construct");
    entry_of_parent_class = object_get_entry_of_parent_class(class_info);

    dbg_str(OBJ_DETAIL,"obj_class addr:%p",class_info);

    if (entry_of_parent_class != NULL) {
        /*
         *dbg_str(OBJ_DETAIL,"init subclass");
         */
        __object_init(obj, entry_of_parent_class->type_name, type_name);
    } else {
        dbg_str(OBJ_DETAIL,"obj has not subclass");
    }

    object_init_func_pointer(obj,class_info);
    object_cover_vitual_func_pointer(obj, cur_type_name, type_name);

    if (entry_of_parent_class != NULL) {
        object_inherit_methods(obj,class_info,entry_of_parent_class->type_name);
    }

    dbg_str(OBJ_DETAIL,"obj addr:%p",obj);
    if (construct != NULL)
        construct(obj,NULL);
    else{
        /*
         *dbg_str(OBJ_WARNNING,"%s construct is NULL",cur_type_name);
         */
    }

    return 0;
}

int object_init(void *obj, char *type_name) 
{
    __object_init(obj, type_name, type_name);
}

int __object_destroy(void *obj, char *type_name) 
{
    object_deamon_t *deamon;
    void *class_info, *parent_class_info;
    class_info_entry_t * entry_of_parent_class;
    int (*deconstruct)(void *obj);

    deamon                = object_deamon_get_global_object_deamon();
    class_info            = object_deamon_search_class(deamon,(char *)type_name);
    deconstruct           = object_get_func_pointer(class_info,"deconstruct");
    entry_of_parent_class = object_get_entry_of_parent_class(class_info);

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
    __object_destroy(obj, ((Obj *)obj)->name);
    return 0;
}

int test_object_config()
{
    char buf[1024] = {0};
    int width = 128;

    object_config(buf, 1024, "/root/home/worksapce/linux", OBJECT_STRING, "name", "alan") ;
    object_config(buf, 1024, "/root/home/worksapce/linux", OBJECT_NUMBER, "width", &width) ;
    dbg_str(DBG_DETAIL,"\n%s",buf);

    return 0;
}
