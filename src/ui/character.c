/**
 * @file character.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-03
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
#include <libobject/ui/character.h>

static int __construct(Character *character,char *init_str)
{
    dbg_str(OBJ_DETAIL,"character construct");

    return 0;
}

static int __deconstrcut(Character *character)
{
    dbg_str(OBJ_DETAIL,"character deconstruct");

    return 0;
}

static int __set(Character *character, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        character->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        character->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        character->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        character->deconstruct = value;
    } else if (strcmp(attrib, "load_character") == 0) {
        character->load_character = value;
    } else if (strcmp(attrib, "assign") == 0) {
        character->assign = value;
    } else if (strcmp(attrib, "load_character") == 0) {
        character->load_character = value;
    } else {
        dbg_str(OBJ_WARNNING,"character set,  \"%s\" setting is not support",attrib);
    }

    return 0;
}

static void * __get(Character *character, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING,"character get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __assign(Character *character, uint32_t code)
{
    character->code = code;
}

static class_info_entry_t character_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5] = {ENTRY_TYPE_FUNC_POINTER,"","assign",__assign,sizeof(void *)},
    [6] = {ENTRY_TYPE_VFUNC_POINTER,"","load_character",NULL,sizeof(void *)},
    [7] = {ENTRY_TYPE_UINT32_T,"","code",NULL,sizeof(void *)},
    [8] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Character",character_class_info);

void test_obj_character()
{
    Character *character;
    allocator_t *allocator = allocator_get_default_alloc();

    character = OBJECT_NEW(allocator, Character,"");
}


