/**
 * @file window.c
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
#include <libobject/ui/window.h>
#include <libobject/core/utils/miscellany/buffer.h>

static int __construct(Window *window, char *init_str)
{
    dbg_str(DBG_IMPORTANT, "window construct, window addr:%p", window);

    window->create_font(window, NULL);
    window->create_render(window, NULL);
    window->create_event(window);
    window->open_window(window);

    window->create_background(window, "./bin/hello_world.bmp");//must create after openning window

    return 0;
}

static int __deconstrcut(Window *window)
{
    dbg_str(DBG_IMPORTANT, "window deconstruct, window addr:%p", window);

    window->destroy_background(window);
    window->destroy_event(window);
    window->destroy_font(window);
    window->destroy_render(window);
    window->close_window(window);

    return 0;
}

static int __set(Window *window, char *attrib, void *value)
{
    /*normal methods*/
    if (strcmp(attrib, "set") == 0) {
        window->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        window->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        window->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        window->deconstruct = value;
    } else if (strcmp(attrib, "update_window") == 0) {
        window->update_window = value;
    } 
    /*vitual methods*/
    else if (strcmp(attrib, "load_resources") == 0) {
        window->load_resources = value;
    } else if (strcmp(attrib, "unload_resources") == 0) { 
        window->unload_resources = value;
    } else if (strcmp(attrib, "move") == 0) { 
        window->move = value;
    } else if (strcmp(attrib, "create_font") == 0) {
        window->create_font = value;
    } else if (strcmp(attrib, "destroy_font") == 0) {
        window->destroy_font = value;
    } else if (strcmp(attrib, "create_render") == 0) {
        window->create_render = value;
    } else if (strcmp(attrib, "destroy_render") == 0) {
        window->destroy_render = value;
    } else if (strcmp(attrib, "create_event") == 0) {
        window->create_event = value;
    } else if (strcmp(attrib, "destroy_event") == 0) {
        window->destroy_event = value;
    } else if (strcmp(attrib, "create_background") == 0) {
        window->create_background = value;
    } else if (strcmp(attrib, "destroy_background") == 0) {
        window->destroy_background = value;
    } else if (strcmp(attrib, "init_window") == 0) {
        window->init_window = value;
    } else if (strcmp(attrib, "open_window") == 0) {
        window->open_window = value;
    } else if (strcmp(attrib, "close_window") == 0) {
        window->close_window = value;
    } else if (strcmp(attrib, "create_timer") == 0) {
        window->create_timer = value;
    } else if (strcmp(attrib, "remove_timer") == 0) {
        window->remove_timer = value;
    } else if (strcmp(attrib, "destroy_timer") == 0) {
        window->destroy_timer = value;
    } else if (strcmp(attrib, "poll_event") == 0) {
        window->poll_event = value;
    } else if (strcmp(attrib, "on_window_moved") == 0) {
        window->on_window_moved = value;
    } else if (strcmp(attrib, "on_window_resized") == 0) {
        window->on_window_resized = value;
    } else if (strcmp(attrib, "on_key_text_pressed") == 0) {
        window->on_key_text_pressed = value;
    } else if (strcmp(attrib, "on_key_esc_pressed") == 0) {
        window->on_key_esc_pressed = value;
    } else if (strcmp(attrib, "set_window_title") == 0) {
        window->set_window_title = value;
    } else if (strcmp(attrib, "set_window_icon") == 0) {
        window->set_window_icon = value;
    } else if (strcmp(attrib, "set_window_size") == 0) {
        window->set_window_size = value;
    } else if (strcmp(attrib, "set_window_position") == 0) {
        window->set_window_position = value;
    } else if (strcmp(attrib, "full_screen") == 0) {
        window->full_screen = value;
    } else if (strcmp(attrib, "maximize_window") == 0) {
        window->maximize_window = value;
    } else if (strcmp(attrib, "minimize_window") == 0) {
        window->minimize_window = value;
    } else if (strcmp(attrib, "restore_window") == 0) {
        window->restore_window = value;
    }
    /*inherited methods*/
    else if (strcmp(attrib, "add_component") == 0) {
        window->add_component = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) { 
        strncpy(window->name, value, strlen(value));
    } else if (strcmp(attrib, "render_type") == 0) {
        window->render_type = *((uint8_t *)value);
    } else if (strcmp(attrib, "screen_width") == 0) {
        window->screen_width = *((uint32_t *)value);
    } else if (strcmp(attrib, "screen_height") == 0) {
        window->screen_height = *((uint32_t *)value);
    } else if (strcmp(attrib, "render") == 0) {
        window->render = value;
    } else {
        dbg_str(DBG_DETAIL, "window set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Window *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "render_type") == 0) {
        return &obj->render_type;
    } else if (strcmp(attrib, "render") == 0) {
        return obj->render;
    } else if (strcmp(attrib, "screen_width") == 0) {
        return &obj->screen_width;
    } else if (strcmp(attrib, "screen_height") == 0) {
        return &obj->screen_height;
    } else {
        dbg_str(DBG_WARNNING, "window get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __update_window(Window *window)
{
    Render *r = window->render;
    Component *component = (Component *)window;

    dbg_str(DBG_DETAIL, "window update_window");

    component->draw(component, r);
    r->present(r);

}

static void
load_subcomponent_resources_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    Window *window = (Window *)arg;

    dbg_str(DBG_DETAIL, "window load_subcomponent_resources_foreach_cb");

    component = (Component *)element;
    if (component->load_resources)
        component->load_resources(component, window);
}

static int __load_resources(Window *window)
{
    Container *container = (Container *)window;

    dbg_str(DBG_IMPORTANT, "window load_resources");

    window->font->load_ascii_character(window->font, window->render);

    container->for_each_component(container, 
                                  load_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void unload_subcomponent_resources_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    Window *window = (Window *)arg;

    dbg_str(DBG_DETAIL, "window unload_subcomponent_resources_foreach_cb");

    /*
     *window->font->unload_ascii_character(window->font, window->render);
     */

    component = (Component *)element;
    if (component->unload_resources)
        component->unload_resources(component, window);
}

static int __unload_resources(Window *window)
{
    Container *container = (Container *)window;

    dbg_str(DBG_IMPORTANT, "window unload_resources");

    container->for_each_component(container, 
                                  unload_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void __on_window_moved(Window *window, void *event, void *w) 
{
    dbg_str(DBG_DETAIL, "window moved");
}

static void __on_window_resized(Window *window, void *event, void *w) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, "Event: windowid %d, resized to %d x %d",
            e->windowid, e->data1, e->data2);
}

static void __on_key_text_pressed(Window *window, char c, void *render)
{
    Render *g             = (Render *)render;

    dbg_str(DBG_DETAIL, "on_text_key_pressed: %c", c);

    switch (c) {
        case '1':
            window->maximize_window(window);
            break;
        case '2':
            window->minimize_window(window);
            break;
        case '3':
            window->restore_window(window);
            break;
        case '4':
            window->full_screen(window);
            break;
        case '5':
            break;
        case '6':
            break;
        case '7':
            break;
        case '8':
            break;
        case '9':
            break;
        case '0':
            break;
        default:
            break;

    }
}

static void __on_key_esc_pressed(Window *window, void *render)
{
    dbg_str(DBG_DETAIL, "on_key_esc_pressed");
}

static class_info_entry_t window_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Component", "component", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "load_resources", __load_resources, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "unload_resources", __unload_resources, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "update_window", __update_window, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "move", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_font", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_font", NULL, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_render", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_render", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_event", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_event", NULL, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_background", NULL, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_background", NULL, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "init_window", NULL, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "open_window", NULL, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "close_window", NULL, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_timer", NULL, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove_timer", NULL, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_timer", NULL, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_IFUNC_POINTER, "", "add_component", NULL, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_VFUNC_POINTER, "", "poll_event", NULL, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_window_moved", NULL, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_window_resized", NULL, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_text_pressed", __on_key_text_pressed, sizeof(void *)}, 
    [28] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_esc_pressed", __on_key_esc_pressed, sizeof(void *)}, 
    [29] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_window_title", NULL, sizeof(void *)}, 
    [30] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_window_icon", NULL, sizeof(void *)}, 
    [31] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_window_size", NULL, sizeof(void *)}, 
    [32] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_window_position", NULL, sizeof(void *)}, 
    [33] = {ENTRY_TYPE_VFUNC_POINTER, "", "full_screen", NULL, sizeof(void *)}, 
    [34] = {ENTRY_TYPE_VFUNC_POINTER, "", "maximize_window", NULL, sizeof(void *)}, 
    [35] = {ENTRY_TYPE_VFUNC_POINTER, "", "minimize_window", NULL, sizeof(void *)}, 
    [36] = {ENTRY_TYPE_VFUNC_POINTER, "", "restore_window", NULL, sizeof(void *)}, 
    [37] = {ENTRY_TYPE_STRING, "char", "name", NULL, 0}, 
    [38] = {ENTRY_TYPE_UINT8_T, "uint8_t", "render_type", NULL, 0}, 
    [39] = {ENTRY_TYPE_UINT32_T, "", "screen_width", NULL, sizeof(short)}, 
    [40] = {ENTRY_TYPE_UINT32_T, "", "screen_height", NULL, sizeof(short)}, 
    [41] = {ENTRY_TYPE_NORMAL_POINTER, "Render", "render", NULL, sizeof(float)}, 
    [42] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Window", window_class_info);

