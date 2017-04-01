/**
 * @file string.c
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
#include <libobject/string.h>

static int string_buf_auto_modulate(String *string, int write_len)
{
    if (string->value_max_len == 0) {
        string->value_max_len = 100;
        if (write_len > string->value_max_len) {
            string->value_max_len = write_len;
        }
        string->value = (char *)allocator_mem_alloc(string->obj.allocator,
                                                    string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING,"string assign alloc error");
            return -1;
        }
    } else if ( string->value_max_len > string->value_len &&
               string->value_max_len < string->value_len + write_len)
    {
        char *new_buf;

        string->value_max_len = 2 * string->value_max_len;
        new_buf = (char *)allocator_mem_alloc(string->obj.allocator,
                                              string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING,"string assign alloc error");
            return -1;
        }
        strncpy(new_buf, string->value, string->value_len);

        allocator_mem_free(string->obj.allocator,string->value);
        string->value = new_buf;
    }

    return 0;
}

static int __construct(String *string,char *init_str)
{
    dbg_str(OBJ_DETAIL,"string construct, string addr:%p",string);
    string->value = (char *)allocator_mem_alloc(string->obj.allocator, 256);
    string->value_max_len = 256;
    return 0;
}

static int __deconstrcut(String *string)
{
    dbg_str(OBJ_DETAIL,"string deconstruct,string addr:%p",string);
    if (string->value)
        allocator_mem_free(string->obj.allocator,string->value);

    return 0;
}

static int __set(String *string, char *attrib, void *value)
{

    if (strcmp(attrib, "set") == 0) {
        string->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        string->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        string->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        string->deconstruct = value;
    } else if (strcmp(attrib, "pre_alloc") == 0) {
        string->pre_alloc = value;
    } else if (strcmp(attrib, "assign") == 0) {
        string->assign = value;
    } else if (strcmp(attrib, "append_char") == 0) {
        string->append_char = value;
    } else if (strcmp(attrib, "replace_char") == 0) {
        string->replace_char = value;
    } else if (strcmp(attrib, "at") == 0) {
        string->at = value;
    }
    else if (strcmp(attrib, "name") == 0) {
        strncpy(string->name,value,strlen(value));
    } else {
        dbg_str(OBJ_DETAIL,"string set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(String *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "value") == 0) {
        return obj->value;
    } else {
        dbg_str(OBJ_WARNNING,"string get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static String *__pre_alloc(String *string,uint32_t size)
{
    dbg_str(OBJ_DETAIL,"pre_alloc, size=%d",size);
    string->value         = (char *)allocator_mem_alloc(string->obj.allocator, size);
    string->value_max_len = size;
    memset(string->value, 0, size);
    return string;
}

static String *__assign(String *string,char *s)
{
    int len = strlen(s);
    int ret;

    ret = string_buf_auto_modulate(string, len);
    if (ret < 0) return string;

    memset(string->value,0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len  = len;
    string->value[len] = '\0';

    return string;
}

static String *__append_char(String *string,char c)
{
    int ret;

    ret = string_buf_auto_modulate(string, 1);
    if (ret < 0) {
        dbg_str(DBG_WARNNING,"string buf_auto_modulate have problem,please check");
        return string;
    }

    string->value[string->value_len] = c;
    string->value_len++;
    string->value[string->value_len] = '\0';

    return string;
}

static String *__replace_char(String *string,int index, char c)
{
    string->value[index] = c;

    return string;
}

static char __at(String *string,int index)
{
    return string->value[index];
}

static class_info_entry_t string_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","pre_alloc",__pre_alloc,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","assign",__assign,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","append_char",__append_char,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","replace_char",__replace_char,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","at",__at,sizeof(void *)},
    [10] = {ENTRY_TYPE_STRING,"char *","name",NULL,0},
    [11] = {ENTRY_TYPE_STRING,"char *","value",NULL,0},
    [12] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("String",string_class_info);

void test_obj_string()
{
    String *string;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    dbg_str(DBG_DETAIL,"test_obj_string");
    root = cjson_create_object();{
        cjson_add_item_to_object(root, "String", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    dbg_str(DBG_SUC, "test_obj_string begin alloc count =%d",allocator->alloc_count);
    string = OBJECT_NEW(allocator, String,set_str);

    string->assign(string,"hello world!");
    string->append_char(string,'a');
    string->append_char(string,'b');

    object_dump(string, "String", buf, 2048);
    dbg_str(DBG_DETAIL,"String dump: %s",buf);

    free(set_str);

    object_destroy(string);
    dbg_str(DBG_SUC, "test_obj_string end alloc count =%d",allocator->alloc_count);

}


