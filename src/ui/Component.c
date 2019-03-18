/**
 * @file component.c
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
#include <libobject/ui/component.h>
#include <libobject/ui/window.h>
#include <libobject/ui/event.h>

static int __construct(Component *component, char *init_str)
{
    dbg_str(DBG_DETAIL, "component construct, component addr:%p", component);

    component->on_mouse_entered_flag = 0;
    component->render = NULL;

    return 0;
}

static int __deconstrcut(Component *component)
{
    dbg_str(DBG_DETAIL, "component deconstruct, component addr:%p", component);

    return 0;
}

static int __move(Component *component)
{
    dbg_str(DBG_DETAIL, "component move");
}

static int __set(Component *component, char *attrib, void *value)
{
    /*normal methods*/
    if (strcmp(attrib, "set") == 0) {
        component->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        component->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        component->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        component->deconstruct = value;
    }
    /*virtual methods*/
    else if (strcmp(attrib, "move") == 0) {
        component->move = value;
    } else if (strcmp(attrib, "draw") == 0) {
        component->draw = value;
    } else if (strcmp(attrib, "load_resources") == 0) {
        component->load_resources = value;
    } else if (strcmp(attrib, "unload_resources") == 0) {
        component->unload_resources = value;
    } else if (strcmp(attrib, "on_key_text_pressed") == 0) {
        component->on_key_text_pressed = value;
    } else if (strcmp(attrib, "on_key_backspace_pressed") == 0) {
        component->on_key_backspace_pressed = value;
    } else if (strcmp(attrib, "on_key_up_pressed") == 0) {
        component->on_key_up_pressed = value;
    } else if (strcmp(attrib, "on_key_down_pressed") == 0) {
        component->on_key_down_pressed = value;
    } else if (strcmp(attrib, "on_key_left_pressed") == 0) {
        component->on_key_left_pressed = value;
    } else if (strcmp(attrib, "on_key_right_pressed") == 0) {
        component->on_key_right_pressed = value;
    } else if (strcmp(attrib, "on_key_pageup_pressed") == 0) {
        component->on_key_pageup_pressed = value;
    } else if (strcmp(attrib, "on_key_pagedown_pressed") == 0) {
        component->on_key_pagedown_pressed = value;
    } else if (strcmp(attrib, "on_key_onelineup_pressed") == 0) {
        component->on_key_onelineup_pressed = value;
    } else if (strcmp(attrib, "on_key_onelinedown_pressed") == 0) {
        component->on_key_onelinedown_pressed = value;
    } else if (strcmp(attrib, "on_key_esc_pressed") == 0) {
        component->on_key_esc_pressed = value;
    } else if (strcmp(attrib, "on_mouse_pressed") == 0) {
        component->on_mouse_pressed = value;
    } else if (strcmp(attrib, "on_mouse_moved") == 0) {
        component->on_mouse_moved = value;
    } else if (strcmp(attrib, "on_mouse_entered") == 0) {
        component->on_mouse_entered = value;
    } else if (strcmp(attrib, "on_mouse_exited") == 0) {
        component->on_mouse_exited = value;
    } else if (strcmp(attrib, "on_mouse_wheel_moved") == 0) {
        component->on_mouse_wheel_moved = value;
    } else if (strcmp(attrib, "on_window_moved") == 0) {
        component->on_window_moved = value;
    } else if (strcmp(attrib, "on_window_resized") == 0) {
        component->on_window_resized = value;
    } else if (strcmp(attrib, "is_mouse_over_component") == 0) {
        component->is_mouse_over_component = value;
    } else if (strcmp(attrib, "set_name") == 0) {
        component->set_name = value;
    } 
    /*inherit methods*/
    else if (strcmp(attrib, "add_event_listener") == 0) {
        component->add_event_listener = value;
    } else if (strcmp(attrib, "add_event_listener_cb") == 0) {
        component->add_event_listener_cb = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        strncpy(component->name, value, strlen(value));
    } else {
        dbg_str(DBG_DETAIL, "component set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Component *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING, "component get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

int __add_event_listener(Component *component, event_listener_t *listener)
{
    /*
     *component->listener = listener;
     */
    memcpy(&component->listener, listener, sizeof(event_listener_t));
}

int __add_event_listener_cb(Component *component, char *name, void *value)
{
    if (strcmp(name, "do_on_mouse_pressed") == 0) {
        component->listener.do_on_mouse_pressed = value;
    } else {
        dbg_str(DBG_WARNNING, "add_event_listener_cb %s, not support now", name);
    }
}

static void 
load_subcomponent_resources_foreach_cb(void *key, void *element, 
                                       void *arg) 
{
    Component *component;
    void *window = (void *)arg;

    component = (Component *)element;

    if (component->load_resources)
        component->load_resources(component, window);
}

static int __load_resources(Component *component, void *window)
{
    Container *container = (Container *)component;

    dbg_str(DBG_DETAIL, "%s load resources", component->name);
    container->for_each_component(container, 
                                  load_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void 
unload_subcomponent_resources_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    void *window = (void *)arg;
    uint8_t *addr;

    component = (Component *)element;

    if (component->unload_resources)
        component->unload_resources(component, window);
}

static int __unload_resources(Component *component, void *window)
{
    Container *container = (Container *)component;

    dbg_str(DBG_DETAIL, "%s unload resources", component->name);
    container->for_each_component(container, 
                                  unload_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void draw_subcomponent_foreach_cb(void *key, void *element, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Render *g = (Render *)arg;

    component = (Component *)element;

    if (component->draw) component->draw(component, g);
}

static int __draw(Component *component, void *render)
{
    Container *container = (Container *)component;
    Render *g            = (Render *)render;
    dbg_str(DBG_DETAIL, "%s draw", ((Obj *)component)->name);

    if (component->render != render) {
        component->render = render;
    }
    container->for_each_component(container, draw_subcomponent_foreach_cb, g);
}

static void __on_key_text_pressed(Component *component, char c, void *render)
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    dbg_str(DBG_DETAIL, "EVENT: text input %c", c);
    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }

    if (cur->on_key_text_pressed) cur->on_key_text_pressed(cur, c, g);
}

static void __on_key_digit_pressed(Component *component, int d, void *render)
{
}

static void __on_key_up_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_up_pressed) cur->on_key_up_pressed(cur, g); 

}

static void __on_key_down_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_down_pressed) cur->on_key_down_pressed(cur, g); 
}

static void __on_key_left_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_left_pressed) cur->on_key_left_pressed(cur, g); 
}

static void __on_key_right_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_right_pressed) cur->on_key_right_pressed(cur, g); 
}

static void __on_key_pageup_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_pageup_pressed) cur->on_key_pageup_pressed(cur, g); 
}

static void __on_key_pagedown_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_pagedown_pressed) cur->on_key_pagedown_pressed(cur, g); 
}

static void __on_key_onelineup_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_onelineup_pressed) cur->on_key_onelineup_pressed(cur, g); 
}

static void __on_key_onelinedown_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_onelinedown_pressed) cur->on_key_onelinedown_pressed(cur, g); 
}

static void __on_key_backspace_pressed(Component *component, void *render) 
{
    Container *container = (Container *)component;
    Render *g             = (Render *)render;
    Component *cur;

    cur = container->search_component(container, "text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING, "not found component :%s", "text_field");
        return;
    }
    if (cur->on_key_backspace_pressed) cur->on_key_backspace_pressed(cur, g); 
}

static void 
process_subcomponent_on_mouse_pressed_foreach_cb(void *key, void *element, 
                                              void *arg) 
{
    Render *g       = (Render *)arg;
    __Event *event = (__Event *)arg;
	Subject *s;
    Component *component;
    uint8_t *addr;

    component = (Component *)element;
	s         = (Subject *)component;

    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }

    if (component->on_mouse_pressed){
        component->on_mouse_pressed(component, event, event->window);
    } 
}

static void __on_mouse_pressed(Component *component, void *event, void *window) 
{
    Container *container = (Container *)component;
    Window *w            = (Window *)window;
    Render *g             = w->render;
    __Event *e           = (__Event *)event;
    Component *cur;

    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }

    /*
     *dbg_str(DBG_DETAIL, 
     *        "%s process on_mouse_pressed event: Mouse button %d pressed at %d, "
     *        "%d with click count %d in window %d", 
     *        component->name, e->button, e->x, e->y, e->clicks, e->windowid); 
     */

    container->for_each_component(container, 
                                  process_subcomponent_on_mouse_pressed_foreach_cb, 
                                  event);
}

int  __is_mouse_over_component(Component *component, void *event)
{
	Subject *s = (Subject *)component;
    __Event *e = (__Event *)event;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d, %d (%d, %d) in window %d", 
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */
    if (    e->x >= s->x && e->x <= s->x + s->width &&
            e->y >= s->y && e->y <= s->y + s->height) {
        return 1;
    } else {
        return 0;
    }
    /*
     *dbg_str(DBG_DETAIL, "component name:%s, e->x=%d e->y=%d, s->x=%d, s->y=%d, s->width=%d, s->height=%d", 
     *        component->name, e->x, e->y, s->x, s->y, s->width, s->height);
     */

    return 1;

}

static void 
process_subcomponent_on_mouse_moved_foreach_cb(void *key, void *element, void *arg) 
{
    Render *g       = (Render *)arg;
    __Event *event = (__Event *)arg;
    Component *component;
    uint8_t *addr;

    component = (Component *)element;

    if (component->is_mouse_over_component(component, event) == 0) {
        if (component->on_mouse_entered_flag == 1) {
            if (component->on_mouse_exited){
                component->on_mouse_exited(component, event, event->window);
            }
            component->on_mouse_entered_flag = 0;
        }
        return;
    }

    if (component->on_mouse_entered_flag == 0) {
        if (component->on_mouse_entered){
            component->on_mouse_entered(component, event, event->window);
        }
        component->on_mouse_entered_flag = 1;
    }

    if (component->on_mouse_moved) {
        component->on_mouse_moved(component, event, event->window);
    }
}


static void __on_mouse_moved(Component *component, void *event, void *window) 
{
    Container *container = (Container *)component;
    Window *w            = (Window *)window;
    Render *g             = w->render;
    __Event *e           = (__Event *)event;
    Component *cur;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d, %d (%d, %d) in window %d", 
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */
    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }
    container->for_each_component(container, 
                                  process_subcomponent_on_mouse_moved_foreach_cb, 
                                  event);

}

static void __on_mouse_wheel_moved(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, 
            "EVENT: Mouse: wheel scrolled %d in x and %d in y (direction: %d) in window %d", 
            e->x, e->y, e->direction, e->windowid);
}

static void __on_window_moved(Component *component, void *event, void *window) 
{
    dbg_str(DBG_DETAIL, "window moved");
}

static void __on_window_resized(Component *component, void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, "EVENT: Window %d resized to %dx%d", 
            e->data1, e->data2, e->windowid);
}

static void __set_name(Component *component,void *name)
{
    strncpy(component->name, name, strlen(name));
}

static class_info_entry_t component_class_info[] = {
    Init_Obj___Entry(0 , Container, container),
    Init_Nfunc_Entry(1 , Component, construct, __construct),
    Init_Nfunc_Entry(2 , Component, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Component, set, NULL),
    Init_Vfunc_Entry(4 , Component, get, NULL),
    Init_Vfunc_Entry(5 , Component, move, __move),
    Init_Vfunc_Entry(6 , Component, draw, __draw),
    Init_Vfunc_Entry(7 , Component, load_resources, __load_resources),
    Init_Vfunc_Entry(8 , Component, unload_resources, __unload_resources),
    Init_Vfunc_Entry(9 , Component, on_key_text_pressed, __on_key_text_pressed),
    Init_Vfunc_Entry(10, Component, on_key_backspace_pressed, __on_key_backspace_pressed),
    Init_Vfunc_Entry(11, Component, on_key_up_pressed, __on_key_up_pressed),
    Init_Vfunc_Entry(12, Component, on_key_down_pressed, __on_key_down_pressed),
    Init_Vfunc_Entry(13, Component, on_key_left_pressed, __on_key_left_pressed),
    Init_Vfunc_Entry(14, Component, on_key_right_pressed, __on_key_right_pressed),
    Init_Vfunc_Entry(15, Component, on_key_pageup_pressed, __on_key_pageup_pressed),
    Init_Vfunc_Entry(16, Component, on_key_pagedown_pressed, __on_key_pagedown_pressed),
    Init_Vfunc_Entry(17, Component, on_key_onelineup_pressed, __on_key_onelinedown_pressed),
    Init_Vfunc_Entry(18, Component, on_key_onelinedown_pressed, __on_key_right_pressed),
    Init_Vfunc_Entry(19, Component, on_key_esc_pressed, NULL),
    Init_Vfunc_Entry(20, Component, on_mouse_pressed, __on_mouse_pressed),
    Init_Vfunc_Entry(21, Component, on_mouse_moved, __on_mouse_moved),
    Init_Vfunc_Entry(22, Component, on_mouse_entered, NULL),
    Init_Vfunc_Entry(23, Component, on_mouse_exited, NULL),
    Init_Vfunc_Entry(24, Component, on_mouse_wheel_moved, __on_mouse_wheel_moved),
    Init_Vfunc_Entry(25, Component, on_window_moved, __on_window_moved),
    Init_Vfunc_Entry(26, Component, on_window_resized, __on_window_resized),
    Init_Vfunc_Entry(27, Component, is_mouse_over_component, __is_mouse_over_component),
    Init_Vfunc_Entry(28, Component, add_event_listener, __add_event_listener),
    Init_Vfunc_Entry(29, Component, add_event_listener_cb, __add_event_listener_cb),
    Init_Vfunc_Entry(30, Component, set_name, __set_name),
    Init_Str___Entry(31, Component, name, NULL),
    Init_End___Entry(32),
};
REGISTER_CLASS("Component", component_class_info);

void test_ui_component()
{
    Subject *subject;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Component", e = cjson_create_object());{
            cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                cjson_add_number_to_object(s, "x", 1);
                cjson_add_number_to_object(s, "y", 25);
                cjson_add_number_to_object(s, "width", 5);
                cjson_add_number_to_object(s, "height", 125);
            }
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    /*
     *subject = OBJECT_ALLOC(allocator, Component);
     *object_set(subject, "Component", set_str);
     *dbg_str(DBG_DETAIL, "x=%d y=%d width=%d height=%d", subject->x, subject->y, subject->width, subject->height);
     */

    subject = OBJECT_NEW(allocator, Component, set_str);

    /*
     *dbg_str(DBG_DETAIL, "x=%d y=%d width=%d height=%d", subject->x, subject->y, subject->width, subject->height);
     *dbg_str(DBG_DETAIL, "component nane=%s", ((Component *)subject)->name);
     *subject->move(subject);
     */

    object_dump(subject, "Component", buf, 2048);
    dbg_str(DBG_DETAIL, "Component dump: %s", buf);

    free(set_str);

}


