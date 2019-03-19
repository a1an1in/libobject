/**
 * @file panel.c
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
#include <libobject/ui/border_layout.h>
#include <libobject/ui/panel.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/config.h>

static int __construct(Panel *panel, char *init_str)
{
    dbg_str(DBG_IMPORTANT, "panel construct, panel addr:%p", panel);


    return 0;
}

static int __deconstrcut(Panel *panel)
{
    dbg_str(DBG_IMPORTANT, "panel deconstruct, panel addr:%p", panel);

    return 0;
}

static void draw_subcomponent_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Render *r = (Render *)arg;

    addr      = (uint8_t *)element;
    component = (Component *)buffer_to_addr(addr);

    if (component->draw) component->draw(component, r);
}

static int __draw(Component *component, void *graph)
{
    Container *container = (Container *)component;
    Render *r             = (Render *)graph;
    Panel *panel         = (Panel *)component;
    Subject *s           = (Subject *)component;

    dbg_str(DBG_DETAIL, "%s draw", ((Obj *)component)->name);

    r->set_color(r, 0, 0, 0, 0xff);
    r->draw_rect(r, s->x, s->y, s->width, s->height);
    container->for_each_component(container, draw_subcomponent_foreach_cb, r);
}

static class_info_entry_t panel_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Panel, construct, __construct),
    Init_Nfunc_Entry(2 , Panel, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Panel, set, NULL),
    Init_Vfunc_Entry(4 , Panel, get, NULL),
    Init_Vfunc_Entry(5 , Panel, draw, __draw),
    Init_End___Entry(6 ),
};
REGISTER_CLASS("Panel", panel_class_info);

void test_ui_panel()
{
#define MAX_BUFFER_LEN 1024
    allocator_t *allocator = allocator_get_default_alloc();
    Window *window;
    Border_Layout *layout;
    Panel *panel;
    char *set_str;
    char config[MAX_BUFFER_LEN] = {0};
    Subject *s;
    configurator_t * c;
    int x = 0, y = 0, width = 400, height = 400;

    set_str = gen_window_setting_str();
    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);

    c = cfg_alloc(allocator); 
    cfg_config_str(c, "/Component", "name", "layout");
    layout  = OBJECT_NEW(allocator, Border_Layout, c->buf);
    cfg_destroy(c);

    c = cfg_alloc(allocator); 
    cfg_config_num(c, "/Subject", "x", x);
    cfg_config_num(c, "/Subject", "y", y);
    cfg_config_num(c, "/Subject", "width", width);
    cfg_config_num(c, "/Subject", "height", height);
    cfg_config_str(c, "/Component", "name", "panel");
    dbg_str(DBG_DETAIL, "config:%s", c->buf);
    panel   = OBJECT_NEW(allocator, Panel, config);
    cfg_destroy(c);

    layout->add_component((Container *)layout, "North", NULL);
    layout->add_component((Container *)layout, "West", NULL);
    layout->add_component((Container *)layout, "East", NULL);
    layout->add_component((Container *)layout, "South", NULL);
    layout->add_component((Container *)layout, "Center", panel);

    window->add_component((Container *)window, NULL, layout);
    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);

    window->unload_resources(window);
    object_destroy(window);
#undef MAX_BUFFER_LEN
}


