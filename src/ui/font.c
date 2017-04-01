/**
 * @file font.c
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
#include <libobject/ui/font.h>

static int __construct(Font *font,char *init_str)
{
    dbg_str(OBJ_DETAIL,"font construct, font addr:%p",font);
    font->path = (String *)OBJECT_NEW(((Obj *)font)->allocator, String,NULL);
    if (font->path == NULL) {
        dbg_str(DBG_ERROR,"construct font error");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Font *font)
{
    dbg_str(OBJ_DETAIL,"font deconstruct,font addr:%p",font);
    object_destroy(font->path);

    return 0;
}

static int __set(Font *font, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        font->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        font->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        font->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        font->deconstruct = value;
    }
    else if (strcmp(attrib, "load_font") == 0) {
        font->load_font = value;
    } else if (strcmp(attrib, "unload_font") == 0) {
        font->unload_font = value;
    } else if (strcmp(attrib, "load_ascii_character") == 0) {
        font->load_ascii_character = value;
    } else if (strcmp(attrib, "unload_ascii_character") == 0) {
        font->unload_ascii_character = value;
    } else if (strcmp(attrib, "get_character_width") == 0) {
        font->get_character_width = value;
    } else if (strcmp(attrib, "get_character_height") == 0) {
        font->get_character_height = value;
    }
    else {
        dbg_str(OBJ_WARNNING,"font set,  \"%s\" setting is not support",attrib);
    }

    return 0;
}

static void * __get(Font *font, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING,"font get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t font_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","load_font",NULL,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","unload_font",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","load_ascii_character",NULL,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","unload_ascii_character",NULL,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_character_width",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","get_character_height",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_NORMAL_POINTER,"","String",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Font",font_class_info);

void test_obj_font()
{
    Font *font;
    allocator_t *allocator = allocator_get_default_alloc();

    font = OBJECT_NEW(allocator, Font,"");
    font->path->assign(font->path,"hello world");
}


