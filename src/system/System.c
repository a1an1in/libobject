/**
 * @file system.c
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
#include <libobject/system/system.h>

static int __construct(System *system,char *init_str)
{
    dbg_str(OBJ_DETAIL,"system construct, system addr:%p",system);

    return 0;
}

static int __deconstrcut(System *system)
{
    dbg_str(OBJ_DETAIL,"system deconstruct,system addr:%p",system);

    return 0;
}

static int __set(System *system, char *attrib, void *value)
{

    if (strcmp(attrib, "set") == 0) {
        system->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        system->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        system->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        system->deconstruct = value;
    } else {
        dbg_str(OBJ_DETAIL,"system set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(System *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "value") == 0) {
        return obj->value;
    } else {
        dbg_str(OBJ_WARNNING,"system get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t system_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_STRING,"char *","name",NULL,0},
    [6 ] = {ENTRY_TYPE_STRING,"char *","value",NULL,0},
    [7 ] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("System",system_class_info);

void test_obj_system()
{
#define MAX_BUFFER_LEN 1024
    System *system;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int alloc_count_be, alloc_count_end;

    dbg_str(DBG_DETAIL,"test_obj_system");
    alloc_count_be = allocator->alloc_count;

    char config[MAX_BUFFER_LEN] = {0};

    object_config(config, MAX_BUFFER_LEN, "/System", OBJECT_STRING, "name", "alan") ;
    system  = OBJECT_NEW(allocator, System,config);

    object_dump(system, "System", buf, 2048);
    dbg_str(DBG_DETAIL,"System dump: %s",buf);

    object_destroy(system);

    alloc_count_end = allocator->alloc_count;
    if (alloc_count_be != alloc_count_end) {
        dbg_str(DBG_WARNNING, "there's mem leak in test_obj_system test");
    }

#undef MAX_BUFFER_LEN
}
