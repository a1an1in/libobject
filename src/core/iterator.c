/**
 * @file iter.c
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
#include <libobject/iterator.h>

static int __construct(Iterator *iter,char *init_str)
{
    dbg_str(OBJ_DETAIL,"iter construct, iter addr:%p",iter);

    return 0;
}

static int __deconstrcut(Iterator *iter)
{
    dbg_str(OBJ_DETAIL,"iter deconstruct,iter addr:%p",iter);

    return 0;
}

static int __set(Iterator *iter, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        iter->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        iter->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        iter->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        iter->deconstruct = value;
    } else if (strcmp(attrib, "next") == 0) {
        iter->next = value;
    } else if (strcmp(attrib, "prev") == 0) {
        iter->prev = value;
    } else if (strcmp(attrib, "equal") == 0) {
        iter->equal = value;
    } else if (strcmp(attrib, "get_vpointer") == 0) {
        iter->get_vpointer = value;
    } else if (strcmp(attrib, "get_kpointer") == 0) {
        iter->get_kpointer = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        iter->destroy = value;
    } else if (strcmp(attrib, "name") == 0) {
        strncpy(iter->name,value,strlen(value));
    } else {
        dbg_str(OBJ_DETAIL,"iter set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Iterator *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(OBJ_WARNNING,"iter get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static Iterator *__next(Iterator *it)
{
    dbg_str(OBJ_DETAIL,"Iterator next");
}

static Iterator *__prev(Iterator *it)
{
    dbg_str(OBJ_DETAIL,"Iterator prev");
}

static int __equal(Iterator *it1,Iterator *it2)
{
    dbg_str(OBJ_DETAIL,"Iterator equal");
}

static void *__get_vpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL,"Iterator get_vpointer");
}

static void *__get_kpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL,"Iterator get_kpointer");
}

static class_info_entry_t iter_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","next",__next,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","prev",__prev,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","equal",__equal,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_vpointer",__get_vpointer,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_kpointer",__get_vpointer,sizeof(void *)},
    [10] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Iterator",iter_class_info);

void test_obj_iter()
{
    Iterator *iter, *next,*prev;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str = NULL;
    cjson_t *root, *e, *s;
    char buf[2048];

    iter = OBJECT_NEW(allocator, Iterator,set_str);
    /*
     *next = OBJECT_NEW(allocator, Iterator,set_str);
     *prev = OBJECT_NEW(allocator, Iterator,set_str);
     */

    iter->next(iter);
    iter->prev(iter);
}


