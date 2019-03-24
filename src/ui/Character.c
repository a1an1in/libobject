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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/character.h>

static int __construct(Character *character, char *init_str)
{
    dbg_str(OBJ_DETAIL, "character construct");

    return 0;
}

static int __deconstrcut(Character *character)
{
    dbg_str(OBJ_DETAIL, "character deconstruct");

    return 0;
}

static int __assign(Character *character, uint32_t code)
{
    character->code = code;
}

static class_info_entry_t character_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Character, construct, __construct),
    Init_Nfunc_Entry(2 , Character, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Character, set, NULL),
    Init_Vfunc_Entry(4 , Character, get, NULL),
    Init_Vfunc_Entry(5 , Character, assign, __assign),
    Init_Vfunc_Entry(6 , Character, load_character, NULL),
    Init_U32___Entry(7 , Character, code, NULL),
    Init_End___Entry(8 ),
};
REGISTER_CLASS("Character", character_class_info);

void test_obj_character()
{
    Character *character;
    allocator_t *allocator = allocator_get_default_alloc();

    character = OBJECT_NEW(allocator, Character, "");
}


