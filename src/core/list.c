/**
 * @file list.c
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
#include <libobject/list.h>

static int __construct(List *list,char *init_str)
{
    dbg_str(OBJ_DETAIL,"list construct, list addr:%p",list);

    return 0;
}

static int __deconstrcut(List *list)
{
    dbg_str(OBJ_DETAIL,"list deconstruct,list addr:%p",list);

    return 0;
}

static int __set(List *list, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        list->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        list->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        list->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        list->deconstruct = value;

    } else if (strcmp(attrib, "push_back") == 0) {
        list->push_back = value;
    } else if (strcmp(attrib, "insert_after") == 0) {
        list->insert_after = value;
    } else if (strcmp(attrib, "del") == 0) {
        list->del = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        list->for_each = value;
    } else if (strcmp(attrib, "for_each_arg2") == 0) {
        list->for_each_arg2 = value;
    } else if (strcmp(attrib, "begin") == 0) {
        list->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        list->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        list->destroy = value;

    } else if (strcmp(attrib, "name") == 0) {
        strncpy(list->name,value,strlen(value));
    } else if (strcmp(attrib, "value_size") == 0) {
        list->value_size = *(int *)value;

    } else {
        dbg_str(OBJ_DETAIL,"list set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(List *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "value_size") == 0) {
        return &obj->value_size;
    } else if (strcmp(attrib, "for_each") == 0) {
        return obj->for_each;
    } else {
        dbg_str(OBJ_WARNNING,"list get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __push_back(List *list,void *value)
{
    dbg_str(OBJ_DETAIL,"List insert");
}


static int __del(List *list,Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"List del");
}

static void __for_each(List *list,void (*func)(Iterator *iter))
{
    Iterator *cur, *end;

    dbg_str(OBJ_IMPORTANT,"List for_each");
    cur = list->begin(list);
    end = list->end(list);

    for (; !end->equal(end,cur); cur->next(cur)) {
        func(cur);
    }

    object_destroy(cur);
    object_destroy(end);
}

static void __for_each_arg2(List *list,void (*func)(Iterator *iter,void *arg),void *arg)
{
    Iterator *cur, *end;

    dbg_str(DBG_IMPORTANT,"List for_each arg2");
    cur = list->begin(list);
    end = list->end(list);

    for (; !end->equal(end,cur); cur->next(cur)) {
        func(cur, arg);
    }

    object_destroy(cur);
    object_destroy(end);
}

static Iterator *__begin(List *list)
{
    dbg_str(OBJ_DETAIL,"List begin");
}

static Iterator *__end(List *list)
{
    dbg_str(OBJ_DETAIL,"List end");
}

static class_info_entry_t list_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","push_back",__push_back,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","insert_after",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del",__del,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each",__for_each,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each_arg2",__for_each_arg2,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","begin",__begin,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","end",__end,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","destroy",NULL,sizeof(void *)},
    [13] = {ENTRY_TYPE_UINT32_T,"","value_size",NULL,sizeof(short)},
    [14] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("List",list_class_info);

void test_obj_list()
{
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "List", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    list = OBJECT_NEW(allocator, List,set_str);

    object_dump(list, "List", buf, 2048);
    dbg_str(OBJ_DETAIL,"List dump: %s",buf);

    free(set_str);

}


