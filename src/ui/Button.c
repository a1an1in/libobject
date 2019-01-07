/**
 * @file button.c
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
#include <libobject/ui/button.h>
#include <libobject/ui/border_layout.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/label.h>
#include <libobject/core/utils/miscellany/buffer.h>

static int __construct(Button *button, char *init_str)
{
    Container *container = (Container *)button;
    Subject *subject     = (Subject *)button;
    Label *label;
    char buf[2048];

	dbg_str(DBG_IMPORTANT, "button construct, button addr:%p", button);

    gen_label_setting_str(subject->x, subject->y, subject->width, subject->height, 
                          "label", (void *)buf);
    label   = OBJECT_NEW(((Obj *)button)->allocator, Label, buf);

    container->add_component((Container *)button, NULL, label);

	return 0;
}

static int __deconstrcut(Button *button)
{
	dbg_str(DBG_IMPORTANT, "button deconstruct, button addr:%p", button);

	return 0;
}

static int __set(Button *button, char *attrib, void *value)
{
	if (strcmp(attrib, "set") == 0) {
		button->set = value;
    } else if (strcmp(attrib, "get") == 0) {
		button->get = value;
	} else if (strcmp(attrib, "construct") == 0) {
		button->construct = value;
	} else if (strcmp(attrib, "deconstruct") == 0) {
		button->deconstruct = value;
	} 
    else if (strcmp(attrib, "move") == 0) {
		button->move = value;
    } else if (strcmp(attrib, "draw") == 0) {
        button->draw = value;
    } else if (strcmp(attrib, "on_mouse_pressed") == 0) {
        button->on_mouse_pressed = value;
    } else if (strcmp(attrib, "on_mouse_released") == 0) {
        button->on_mouse_released = value;
    } else if (strcmp(attrib, "on_mouse_entered") == 0) {
        button->on_mouse_entered = value;
    } else if (strcmp(attrib, "on_mouse_exited") == 0) {
        button->on_mouse_exited = value;
    } else if (strcmp(attrib, "on_mouse_moved") == 0) {
        button->on_mouse_moved = value;
    } 
    else if (strcmp(attrib, "add_event_listener") == 0) {
        button->add_event_listener = value;
    } else if (strcmp(attrib, "add_event_listener_cb") == 0) {
        button->add_event_listener_cb = value;
	} 
    else {
		dbg_str(DBG_DETAIL, "button set, not support %s setting", attrib);
	}

	return 0;
}

static void *__get(Button *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING, "button get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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

static int __draw(Component *component, void *render)
{
    Container *container = (Container *)component;
    Render *r             = (Render *)render;
    Button *button       = (Button *)component;
    Subject *s           = (Subject *)component;

    dbg_str(DBG_DETAIL, "%s draw", ((Obj *)component)->name);

    r->set_color(r, 0, 0, 0, 0xff);
    r->draw_rect(r, s->x, s->y, s->width, s->height);
    container->for_each_component(container, draw_subcomponent_foreach_cb, r);
}

static void __on_mouse_pressed(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, 
            "EVENT: mouse pressed: %s process on_mouse_pressed event: Mouse button %d pressed at %d, "
            "%d with click count %d in window %d", 
            component->name, e->button, e->x, e->y, e->clicks, e->windowid); 

    component->listener.do_on_mouse_pressed(component, event, window);
}

static void __on_mouse_released(Component *component, void *event, void *window) 
{

}

static void __on_mouse_moved(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d, %d (%d, %d) in window %d", 
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */

}

static void __on_mouse_entered(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, "EVENT: Mouse entered %s: moved to %d, %d (%d, %d) in window %d", 
            component->name, e->x, e->y, e->xrel, e->yrel, e->windowid);
}

static void __on_mouse_exited(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, "EVENT: Mouse exited %s: moved to %d, %d (%d, %d) in window %d", 
            component->name, e->x, e->y, e->xrel, e->yrel, e->windowid);
}

static class_info_entry_t button_class_info[] = {
	[0 ] = {ENTRY_TYPE_OBJ, "Component", "component", NULL, sizeof(void *)}, 
	[1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
	[2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
	[3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
	[4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
	[5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "move", NULL, sizeof(void *)}, 
	[6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "draw", __draw, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_mouse_pressed", __on_mouse_pressed, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_mouse_released", __on_mouse_released, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_FUNC_POINTER, "", "on_mouse_entered", __on_mouse_entered, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_FUNC_POINTER, "", "on_mouse_exited", __on_mouse_exited, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_FUNC_POINTER, "", "on_mouse_moved", __on_mouse_moved, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_IFUNC_POINTER, "", "add_event_listener", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_IFUNC_POINTER, "", "add_event_listener_cb", NULL, sizeof(void *)}, 
	[14] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("Button", button_class_info);

static void *new_button(allocator_t *allocator, int x, int y, int width, int height, char *name)
{
#define MAX_BUFFER_LEN 1024
    Subject *subject;
    char buf[MAX_BUFFER_LEN] = {0};

    object_config(buf, MAX_BUFFER_LEN, "/Subject", OBJECT_NUMBER, "x", &x);
    object_config(buf, MAX_BUFFER_LEN, "/Subject", OBJECT_NUMBER, "y", &y);
    object_config(buf, MAX_BUFFER_LEN, "/Subject", OBJECT_NUMBER, "width", &width);
    object_config(buf, MAX_BUFFER_LEN, "/Subject", OBJECT_NUMBER, "height", &height);
    object_config(buf, MAX_BUFFER_LEN, "/Component", OBJECT_STRING, "name", name) ;

    dbg_str(DBG_DETAIL, "\n%s", buf);

    subject = OBJECT_NEW(allocator, Button, buf);

    return subject;
#undef MAX_BUFFER_LEN
}

static void __do_on_mouse_pressed(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, "%s do mouse pressed", component->name); 
}

static event_listener_t button_listener = {
    .do_on_mouse_pressed = __do_on_mouse_pressed, 
};

static int button()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Window *window;
    Border_Layout *layout;
    Button *button;
    char *set_str;
    char buf[2048];
    Subject *s;

    set_str = gen_window_setting_str();
    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);

    /*
     *object_dump(window, "Sdl_Window", buf, 2048);
     *dbg_str(DBG_DETAIL, "Window dump: %s", buf);
     */

    layout = new_border_layout(allocator, 0, 0, 0, 0, "border layout");

    button = new_button(allocator, 0, 0, 100, 50, "button02");
#if 0
    button->add_event_listener((Component *)button, &button_listener);
#else
    button->add_event_listener_cb((Component *)button, "do_on_mouse_pressed", __do_on_mouse_pressed);
#endif
    layout->add_component((Container *)layout, "North", NULL);
    layout->add_component((Container *)layout, "West", NULL);
    layout->add_component((Container *)layout, "Center", button);
    layout->add_component((Container *)layout, "East", NULL);
    layout->add_component((Container *)layout, "South", NULL);

    window->add_component((Container *)window, NULL, layout);
    window->load_resources(window);
    window->update_window(window);
    window->event->poll_event(window->event, window);

    window->unload_resources(window);
    object_destroy(window);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(button);

