/**
 * @file render.c
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
#include <libobject/ui/render.h>
#include <libobject/ui/component.h>

static int __construct(Render *render, char *init_str)
{
    dbg_str(DBG_DETAIL, "render construct, render addr:%p", render);

    return 0;
}

static int __deconstrcut(Render *render)
{
    dbg_str(DBG_DETAIL, "render deconstruct, render addr:%p", render);

    return 0;
}

static class_info_entry_t class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Render, construct, __construct),
    Init_Nfunc_Entry(2 , Render, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Render, set, NULL),
    Init_Vfunc_Entry(4 , Render, get, NULL),
    Init_Vfunc_Entry(5 , Render, update_window, NULL),
    Init_Vfunc_Entry(6 , Render, set_window, NULL),
    Init_Vfunc_Entry(7 , Render, draw_image0, NULL),
    Init_Vfunc_Entry(8 , Render, create, NULL),
    Init_Vfunc_Entry(9 , Render, destroy, NULL),
    Init_Vfunc_Entry(10, Render, set_color, NULL),
    Init_Vfunc_Entry(11, Render, set_font, NULL),
    Init_Vfunc_Entry(12, Render, clear, NULL),
    Init_Vfunc_Entry(13, Render, draw_line, NULL),
    Init_Vfunc_Entry(14, Render, draw_rect, NULL),
    Init_Vfunc_Entry(15, Render, fill_rect, NULL),
    Init_Vfunc_Entry(16, Render, draw_image, NULL),
    Init_Vfunc_Entry(17, Render, draw_texture, NULL),
    Init_Vfunc_Entry(18, Render, load_text, NULL),
    Init_Vfunc_Entry(19, Render, load_image, NULL),
    Init_Vfunc_Entry(20, Render, unload_text, NULL),
    Init_Vfunc_Entry(21, Render, write_text, NULL),
    Init_Vfunc_Entry(22, Render, load_character, NULL),
    Init_Vfunc_Entry(23, Render, unload_character, NULL),
    Init_Vfunc_Entry(24, Render, write_character, NULL),
    Init_Vfunc_Entry(25, Render, present, NULL),
    Init_Vfunc_Entry(26, Render, create_yuvtexture, NULL),
    Init_Vfunc_Entry(27, Render, destroy_texture, NULL),
    Init_Str___Entry(28, Render, name, NULL),
    Init_End___Entry(29),
};
REGISTER_CLASS("Render", class_info);

void test_ui_render()
{
    Render *render;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Render", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    render = OBJECT_NEW(allocator, Render, set_str);

    object_dump(render, "Render", buf, 2048);
    dbg_str(DBG_DETAIL, "Render dump: %s", buf);

    object_destroy(render);

    free(set_str);

}


