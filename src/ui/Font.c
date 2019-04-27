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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/font.h>

static int __construct(Font *font, char *init_str)
{
    dbg_str(OBJ_DETAIL, "font construct, font addr:%p", font);
    font->path = (String *)OBJECT_NEW(((Obj *)font)->allocator, String, NULL);
    if (font->path == NULL) {
        dbg_str(DBG_ERROR, "construct font error");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Font *font)
{
    dbg_str(OBJ_DETAIL, "font deconstruct, font addr:%p", font);
    object_destroy(font->path);

    return 0;
}

static class_info_entry_t font_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Font, construct, __construct),
    Init_Nfunc_Entry(2 , Font, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Font, set, NULL),
    Init_Vfunc_Entry(4 , Font, get, NULL),
    Init_Vfunc_Entry(5 , Font, load_font, NULL),
    Init_Vfunc_Entry(6 , Font, unload_font, NULL),
    Init_Vfunc_Entry(7 , Font, load_ascii_character, NULL),
    Init_Vfunc_Entry(8 , Font, unload_ascii_character, NULL),
    Init_Vfunc_Entry(9 , Font, get_character_width, NULL),
    Init_Vfunc_Entry(10, Font, get_character_height, NULL),
    Init_End___Entry(11, Font),
};
REGISTER_CLASS("Font", font_class_info);

void test_obj_font()
{
    Font *font;
    allocator_t *allocator = allocator_get_default_alloc();

    font = OBJECT_NEW(allocator, Font, "");
    font->path->assign(font->path, "hello world");
}


