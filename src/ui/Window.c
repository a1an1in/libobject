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
#include <libobject/ui/Window.h>
#include <libobject/core/utils/miscellany/buffer.h>

static int __construct(Window *window, char *init_str)
{
    dbg_str(DBG_INFO, "window construct, window addr:%p", window);

    window->create_font(window, NULL);
    window->create_render(window, NULL);
    window->create_event(window);
    window->open_window(window);

    /*
     *window->create_background(window, "./bin/hello_world.bmp");//must create after openning window
     */

    return 0;
}

static int __deconstrcut(Window *window)
{
    dbg_str(DBG_INFO, "window deconstruct, window addr:%p", window);

    if (window->name != NULL) {
        object_destroy(window->name);
    }
    /*
     *window->destroy_background(window);
     */
    window->destroy_event(window);
    window->destroy_font(window);
    window->destroy_render(window);
    window->close_window(window);

    return 0;
}

static int __update_window(Window *window)
{
    Render *r = window->render;
    Component *component = (Component *)window;

    dbg_str(DBG_DETAIL, "window update_window");

    /*
     *r->set_color(r, 0xff, 0xff, 0xff, 0xff);
     */
    r->set_color(r, 0, 0, 0, 0);
    r->clear(r);
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

    dbg_str(DBG_INFO, "window load_resources");

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

    dbg_str(DBG_INFO, "window unload_resources");

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
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Window, construct, __construct),
    Init_Nfunc_Entry(2 , Window, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Window, set, NULL),
    Init_Vfunc_Entry(4 , Window, get, NULL),
    Init_Vfunc_Entry(5 , Window, load_resources, __load_resources),
    Init_Vfunc_Entry(6 , Window, unload_resources, __unload_resources),
    Init_Vfunc_Entry(7 , Window, update_window, __update_window),
    Init_Vfunc_Entry(8 , Window, move, NULL),
    Init_Vfunc_Entry(9 , Window, create_font, NULL),
    Init_Vfunc_Entry(10, Window, destroy_font, NULL),
    Init_Vfunc_Entry(11, Window, create_render, NULL),
    Init_Vfunc_Entry(12, Window, destroy_render, NULL),
    Init_Vfunc_Entry(13, Window, create_event, NULL),
    Init_Vfunc_Entry(14, Window, destroy_event, NULL),
    Init_Vfunc_Entry(15, Window, create_background, NULL),
    Init_Vfunc_Entry(16, Window, destroy_background, NULL),
    Init_Vfunc_Entry(17, Window, init_window, NULL),
    Init_Vfunc_Entry(18, Window, open_window, NULL),
    Init_Vfunc_Entry(19, Window, close_window, NULL),
    Init_Vfunc_Entry(20, Window, create_timer, NULL),
    Init_Vfunc_Entry(21, Window, remove_timer, NULL),
    Init_Vfunc_Entry(22, Window, destroy_timer, NULL),
    Init_Vfunc_Entry(23, Window, add_component, NULL),
    Init_Vfunc_Entry(24, Window, poll_event, NULL),
    Init_Vfunc_Entry(25, Window, on_window_moved, NULL),
    Init_Vfunc_Entry(26, Window, on_window_resized, NULL),
    Init_Vfunc_Entry(27, Window, on_key_text_pressed, __on_key_text_pressed),
    Init_Vfunc_Entry(28, Window, on_key_esc_pressed, __on_key_esc_pressed),
    Init_Vfunc_Entry(29, Window, set_window_title, NULL),
    Init_Vfunc_Entry(30, Window, set_window_icon, NULL),
    Init_Vfunc_Entry(31, Window, set_window_size, NULL),
    Init_Vfunc_Entry(32, Window, set_window_position, NULL),
    Init_Vfunc_Entry(33, Window, full_screen, NULL),
    Init_Vfunc_Entry(34, Window, maximize_window, NULL),
    Init_Vfunc_Entry(35, Window, minimize_window, NULL),
    Init_Vfunc_Entry(36, Window, restore_window, NULL),
    Init_Str___Entry(37, Window, name, NULL),
    Init_U8____Entry(38, Window, render_type, NULL),
    Init_U32___Entry(39, Window, screen_width, NULL),
    Init_U32___Entry(40, Window, screen_height, NULL),
    Init_Point_Entry(41, Window, render, NULL),
    Init_End___Entry(42, Window),
};
REGISTER_CLASS("Window", window_class_info);

