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

static int __set(Render *render, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        render->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        render->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        render->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        render->deconstruct = value;
    }
    else if (strcmp(attrib, "move") == 0) {
        render->move = value;
    } else if (strcmp(attrib, "update_window") == 0) {
        render->update_window = value;
    } else if (strcmp(attrib, "set_window") == 0) {
        render->set_window = value;
    } else if (strcmp(attrib, "draw_image0") == 0) {
        render->draw_image0 = value;
    } else if (strcmp(attrib, "create") == 0) {
        render->create = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        render->destroy = value;
    } else if (strcmp(attrib, "set_color") == 0) {
        render->set_color = value;
    } else if (strcmp(attrib, "set_font") == 0) {
        render->set_font = value;
    } else if (strcmp(attrib, "clear") == 0) {
        render->clear = value;
    } else if (strcmp(attrib, "draw_line") == 0) {
        render->draw_line = value;
    } else if (strcmp(attrib, "draw_rect") == 0) {
        render->draw_rect = value;
    } else if (strcmp(attrib, "fill_rect") == 0) {
        render->fill_rect = value;
    } else if (strcmp(attrib, "draw_image") == 0) {
        render->draw_image = value;
    } else if (strcmp(attrib, "draw_texture") == 0) {
        render->draw_texture = value;
    } else if (strcmp(attrib, "load_image") == 0) {
        render->load_image = value;
    } else if (strcmp(attrib, "load_text") == 0) {
        render->load_text = value;
    } else if (strcmp(attrib, "unload_text") == 0) {
        render->unload_text = value;
    } else if (strcmp(attrib, "write_text") == 0) {
        render->write_text = value;
    } else if (strcmp(attrib, "load_character") == 0) {
        render->load_character = value;
    } else if (strcmp(attrib, "unload_character") == 0) {
        render->unload_character = value;
    } else if (strcmp(attrib, "write_character") == 0) {
        render->write_character = value;
    } else if (strcmp(attrib, "present") == 0) {
        render->present = value;
    } else if (strcmp(attrib, "create_yuvtexture") == 0) {
        render->create_yuvtexture = value;
    } else if (strcmp(attrib, "destroy_texture") == 0) {
        render->destroy_texture = value;
    }
    else if (strcmp(attrib, "name") == 0) {
        strncpy(render->name, value, strlen(value));
    } else {
        dbg_str(DBG_DETAIL, "render set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Render *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING, "render get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "update_window", NULL, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_window", NULL, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_image0", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "create", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_color", NULL, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_font", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "clear", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_line", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_rect", NULL, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "fill_rect", NULL, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_image", NULL, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_texture", NULL, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "load_image", NULL, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "load_text", NULL, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "unload_text", NULL, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "write_text", NULL, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "load_character", NULL, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_VFUNC_POINTER, "", "unload_character", NULL, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_VFUNC_POINTER, "", "write_character", NULL, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_VFUNC_POINTER, "", "present", NULL, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_yuvtexture", NULL, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_texture", NULL, sizeof(void *)}, 
    [28] = {ENTRY_TYPE_STRING, "char", "name", NULL, 0}, 
    [29] = {ENTRY_TYPE_END}, 
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


