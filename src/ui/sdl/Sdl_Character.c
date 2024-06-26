/**
 * @file character_sdl.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-04
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Sdl_Character.h>
#include <libobject/ui/Sdl_Window.h>

static int __construct(Character *character, char *init_str)
{
    dbg_str(OBJ_DETAIL, "character construct, character addr:%p", character);

    return 0;
}

static int __deconstrcut(Character *character)
{
    Sdl_Character *c = (Sdl_Character *)character;
    dbg_str(OBJ_DETAIL, "character deconstruct, character addr:%p", character);

    if (c->surface != NULL){
        SDL_FreeSurface(c->surface);
    }
    if (c->texture != NULL) {
        SDL_DestroyTexture(c->texture);
    }

    return 0;
}

static int __set(Character *character, char *attrib, void *value)
{
    Sdl_Character *c = (Sdl_Character *)character;

    if (strcmp(attrib, "set") == 0) {
        c->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        c->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        c->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        c->deconstruct = value;
    } else if (strcmp(attrib, "load_character") == 0) {
        c->load_character = value;
    } else {
        dbg_str(OBJ_WARN, "character set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(Character *character, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARN, "character get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __load_character(Character *character, void *render)
{
}

static class_info_entry_t character_class_info[] = {
    Init_Obj___Entry(0 , Character, character),
    Init_Nfunc_Entry(1 , Sdl_Character, construct, __construct),
    Init_Nfunc_Entry(2 , Sdl_Character, deconstruct, __deconstrcut),
    Init_Nfunc_Entry(3 , Sdl_Character, load_character, __load_character),
    Init_End___Entry(4 , Sdl_Character),
};
REGISTER_CLASS(Sdl_Character, character_class_info);

int sdl_character()
{
    Window *window;
    Render *r;
    Character *c1;
    Character *c2;
    Character *c3;
    Font *font;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str;
    char buf[2048];

    set_str = gen_window_setting_str();

    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);
    r       = window->render;

    /*
     *object_dump(window, "Sdl_Window", buf, 2048);
     *dbg_str(DBG_DETAIL, "Window dump: %s", buf);
     */
    window->load_resources(window);
    window->update_window(window);

    c1 = r->load_character(r, 'a', window->font, 0, 0, 0, 0xff);
    c2 = r->load_character(r, 'b', window->font, 0, 0, 0, 0xff);
    c3 = r->load_character(r, 'c', window->font, 0, 0, 0, 0xff);
    r->write_character(r, 0,  0, c1);
    r->write_character(r, 5, 0, c2);
    r->write_character(r, 10, 0, c3);

    r->present(r);

    pause();
    r->unload_character(r, c1);
    r->unload_character(r, c2);
    r->unload_character(r, c3);

    object_destroy(window);

    /*
     *free(set_str);
     */

}
REGISTER_TEST_CMD(sdl_character);


