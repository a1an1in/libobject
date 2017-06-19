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

static int __construct(Obj *obj,char *init_str)
{
    dbg_str(OBJ_DETAIL,"obj construct, obj addr:%p",obj);

    return 0;
}

static int __deconstrcut(Obj *obj)
{
    dbg_str(OBJ_DETAIL,"obj deconstruct,obj addr:%p",obj);
    allocator_mem_free(obj->allocator,obj);

    return 0;
}

static int __set(Obj *obj, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        obj->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        obj->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        obj->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        obj->deconstruct = value;
    } else if (strcmp(attrib, "allocator") == 0) {
        obj->allocator = value;
        dbg_str(OBJ_DETAIL,"allocator addr:%p",obj->allocator);
    } else {
        dbg_str(OBJ_WARNNING,"obj set, \"%s\" setting is not supported",attrib);
    }

    return 0;
}

static void *__get(Obj *obj, char *attrib)
{
    if (strcmp(attrib, "allocator") == 0) {
        return obj->allocator;
    } else if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(OBJ_WARNNING,"obj get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t obj_class_info[] = {
    [0] = {ENTRY_TYPE_NORMAL_POINTER,"allocator_t","allocator",NULL,sizeof(void *)},
    [1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5] = {ENTRY_TYPE_STRING,"","name",NULL,0},
    [6] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Obj",obj_class_info);

void test_obj()
{
    Obj *obj;
    char buf[2048];
    allocator_t *allocator = allocator_get_default_alloc();

    obj = OBJECT_NEW(allocator, Obj,"");

    object_dump(obj, "Obj", buf, 2048);
}


