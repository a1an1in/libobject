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
#include <miscellany/buffer.h>

static int __construct(Component *component,char *init_str)
{
    dbg_str(DBG_DETAIL,"component construct, component addr:%p",component);

    component->mouse_entered_flag = 0;

    return 0;
}

static int __deconstrcut(Component *component)
{
    dbg_str(DBG_DETAIL,"component deconstruct,component addr:%p",component);

    return 0;
}

static int __move(Component *component)
{
    dbg_str(DBG_DETAIL,"component move");
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
    } else if (strcmp(attrib, "key_text_pressed") == 0) {
        component->key_text_pressed = value;
    } else if (strcmp(attrib, "key_backspace_pressed") == 0) {
        component->key_backspace_pressed = value;
    } else if (strcmp(attrib, "key_up_pressed") == 0) {
        component->key_up_pressed = value;
    } else if (strcmp(attrib, "key_down_pressed") == 0) {
        component->key_down_pressed = value;
    } else if (strcmp(attrib, "key_left_pressed") == 0) {
        component->key_left_pressed = value;
    } else if (strcmp(attrib, "key_right_pressed") == 0) {
        component->key_right_pressed = value;
    } else if (strcmp(attrib, "key_pageup_pressed") == 0) {
        component->key_pageup_pressed = value;
    } else if (strcmp(attrib, "key_pagedown_pressed") == 0) {
        component->key_pagedown_pressed = value;
    } else if (strcmp(attrib, "key_onelineup_pressed") == 0) {
        component->key_onelineup_pressed = value;
    } else if (strcmp(attrib, "key_onelinedown_pressed") == 0) {
        component->key_onelinedown_pressed = value;
    } else if (strcmp(attrib, "mouse_pressed") == 0) {
        component->mouse_pressed = value;
    } else if (strcmp(attrib, "mouse_moved") == 0) {
        component->mouse_moved = value;
    } else if (strcmp(attrib, "mouse_entered") == 0) {
        component->mouse_entered = value;
    } else if (strcmp(attrib, "mouse_exited") == 0) {
        component->mouse_exited = value;
    } else if (strcmp(attrib, "mouse_wheel_moved") == 0) {
        component->mouse_wheel_moved = value;
    } else if (strcmp(attrib, "window_moved") == 0) {
        component->window_moved = value;
    } else if (strcmp(attrib, "window_resized") == 0) {
        component->window_resized = value;
    } else if (strcmp(attrib, "is_mouse_over_component") == 0) {
        component->is_mouse_over_component = value;
    } 
    /*inherit methods*/
    else if (strcmp(attrib, "add_event_listener") == 0) {
        component->add_event_listener = value;
    } else if (strcmp(attrib, "add_event_listener_cb") == 0) {
        component->add_event_listener_cb = value;
    }
    /*attribs*/
    else if (strcmp(attrib, "name") == 0) {
        strncpy(component->name,value,strlen(value));
    } else {
        dbg_str(DBG_DETAIL,"component set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Component *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING,"component get, \"%s\" getting attrib is not supported",attrib);
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
    if (strcmp(name, "do_mouse_pressed") == 0) {
        component->listener.do_mouse_pressed = value;
    } else {
        dbg_str(DBG_WARNNING,"add_event_listener_cb %s, not support now", name);
    }
}

static void 
load_subcomponent_resources_foreach_cb(Iterator *iter,
                                       void *arg) 
{
    Component *component;
    void *window = (void *)arg;
    uint8_t *addr;

    addr = (uint8_t *)iter->get_vpointer(iter);

    component = (Component *)buffer_to_addr(addr);
    if (component->load_resources)
        component->load_resources(component, window);
}

static int __load_resources(Component *component,void *window)
{
    Container *container = (Container *)component;

    dbg_str(DBG_DETAIL,"%s load resources",component->name);
    container->for_each_component(container, 
                                  load_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void 
unload_subcomponent_resources_foreach_cb(Iterator *iter, void *arg) 
{
    Component *component;
    void *window = (void *)arg;
    uint8_t *addr;

    addr = (uint8_t *)iter->get_vpointer(iter);

    component = (Component *)buffer_to_addr(addr);
    if (component->unload_resources)
        component->unload_resources(component, window);
}

static int __unload_resources(Component *component,void *window)
{
    Container *container = (Container *)component;

    dbg_str(DBG_DETAIL,"%s unload resources",component->name);
    container->for_each_component(container,
                                  unload_subcomponent_resources_foreach_cb, 
                                  window);

    return 0;
}

static void draw_subcomponent_foreach_cb(Iterator *iter, void *arg) 
{
    Component *component;
    uint8_t *addr;
    Graph *g = (Graph *)arg;

    addr      = (uint8_t *)iter->get_vpointer(iter);
    component = (Component *)buffer_to_addr(addr);

    if (component->draw) component->draw(component, g);
}

static int __draw(Component *component, void *graph)
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    dbg_str(DBG_DETAIL,"%s draw", ((Obj *)component)->name);

    container->for_each_component(container, draw_subcomponent_foreach_cb, g);
}

static void __key_text_pressed(Component *component,char c, void *graph)
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    dbg_str(DBG_DETAIL,"EVENT: text input %c", c);
    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }

    if (cur->key_text_pressed) cur->key_text_pressed(cur,c, g);
}

static void __key_up_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_up_pressed) cur->key_up_pressed(cur, g); 

}

static void __key_down_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_down_pressed) cur->key_down_pressed(cur, g); 
}

static void __key_left_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_left_pressed) cur->key_left_pressed(cur, g); 
}

static void __key_right_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_right_pressed) cur->key_right_pressed(cur, g); 
}

static void __key_pageup_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_pageup_pressed) cur->key_pageup_pressed(cur, g); 
}

static void __key_pagedown_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_pagedown_pressed) cur->key_pagedown_pressed(cur, g); 
}

static void __key_onelineup_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_onelineup_pressed) cur->key_onelineup_pressed(cur, g); 
}

static void __key_onelinedown_pressed(Component *component, void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_onelinedown_pressed) cur->key_onelinedown_pressed(cur, g); 
}

static void __key_backspace_pressed(Component *component,void *graph) 
{
    Container *container = (Container *)component;
    Graph *g             = (Graph *)graph;
    Component *cur;

    cur = container->search_component(container,"text_area");
    if (cur == NULL) {
        dbg_str(DBG_WARNNING,"not found component :%s","text_field");
        return;
    }
    if (cur->key_backspace_pressed) cur->key_backspace_pressed(cur, g); 
}

static void 
process_subcomponent_mouse_pressed_foreach_cb(Iterator *iter,
                                              void *arg) 
{
    Graph *g       = (Graph *)arg;
    __Event *event = (__Event *)arg;
	Subject *s;
    Component *component;
    uint8_t *addr;

    addr      = (uint8_t *)iter->get_vpointer(iter);
    component = (Component *)buffer_to_addr(addr);
	s         = (Subject *)component;

    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }

    if (component->mouse_pressed){
        component->mouse_pressed(component, event, event->window);
    } 
}

static void __mouse_pressed(Component *component,void *event, void *window) 
{
    Container *container = (Container *)component;
    Window *w            = (Window *)window;
    Graph *g             = w->graph;
    __Event *e           = (__Event *)event;
    Component *cur;

    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }

    /*
     *dbg_str(DBG_DETAIL,
     *        "%s process mouse_pressed event: Mouse button %d pressed at %d,"
     *        "%d with click count %d in window %d",
     *        component->name, e->button, e->x, e->y, e->clicks, e->windowid); 
     */

    container->for_each_component(container, 
                                  process_subcomponent_mouse_pressed_foreach_cb,
                                  event);
}

int  __is_mouse_over_component(Component *component,void *event)
{
	Subject *s = (Subject *)component;
    __Event *e = (__Event *)event;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d,%d (%d,%d) in window %d",
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */
    if (    e->x >= s->x && e->x <= s->x + s->width &&
            e->y >= s->y && e->y <= s->y + s->height) {
        return 1;
    } else {
        return 0;
    }
    /*
     *dbg_str(DBG_DETAIL,"component name:%s, e->x=%d e->y=%d, s->x=%d, s->y=%d, s->width=%d, s->height=%d",
     *        component->name,e->x,e->y, s->x, s->y, s->width, s->height);
     */

    return 1;

}

static void 
process_subcomponent_mouse_moved_foreach_cb(Iterator *iter, void *arg) 
{
    Graph *g       = (Graph *)arg;
    __Event *event = (__Event *)arg;
    Component *component;
    uint8_t *addr;

    addr = (uint8_t *)iter->get_vpointer(iter);
    component = (Component *)buffer_to_addr(addr);

    if (component->is_mouse_over_component(component, event) == 0) {
        if (component->mouse_entered_flag == 1) {
            if (component->mouse_exited){
                component->mouse_exited(component, event, event->window);
            }
            component->mouse_entered_flag = 0;
        }
        return;
    }

    if (component->mouse_entered_flag == 0) {
        if (component->mouse_entered){
            component->mouse_entered(component, event, event->window);
        }
        component->mouse_entered_flag = 1;
    }

    if (component->mouse_moved) {
        component->mouse_moved(component, event, event->window);
    }
}


static void __mouse_moved(Component *component,void *event, void *window) 
{
    Container *container = (Container *)component;
    Window *w            = (Window *)window;
    Graph *g             = w->graph;
    __Event *e           = (__Event *)event;
    Component *cur;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d,%d (%d,%d) in window %d",
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */
    if (component->is_mouse_over_component(component, event) == 0) {
        return;
    }
    container->for_each_component(container, 
                                  process_subcomponent_mouse_moved_foreach_cb,
                                  event);

}

static void __mouse_wheel_moved(Component *component,void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL,
            "EVENT: Mouse: wheel scrolled %d in x and %d in y (direction: %d) in window %d", 
            e->x, e->y, e->direction, e->windowid);
}

static void __window_moved(Component *component,void *event, void *window) 
{
    dbg_str(DBG_DETAIL,"window moved");
}

static void __window_resized(Component *component,void *event, void *window) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL,"EVENT: Window %d resized to %dx%d",
            e->data1, e->data2, e->windowid);
}


static class_info_entry_t component_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Container","container",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","move",__move,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","draw",__draw,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","load_resources",__load_resources,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","unload_resources",__unload_resources,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","key_text_pressed",__key_text_pressed,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","key_backspace_pressed",__key_backspace_pressed,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","key_up_pressed",__key_up_pressed,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","key_down_pressed",__key_down_pressed,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","key_left_pressed",__key_left_pressed,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","key_right_pressed",__key_right_pressed,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","key_pageup_pressed",__key_pageup_pressed,sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER,"","key_pagedown_pressed",__key_pagedown_pressed,sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER,"","key_onelineup_pressed",__key_onelineup_pressed,sizeof(void *)},
    [18] = {ENTRY_TYPE_VFUNC_POINTER,"","key_onelinedown_pressed",__key_onelinedown_pressed,sizeof(void *)},
    [19] = {ENTRY_TYPE_VFUNC_POINTER,"","mouse_pressed",__mouse_pressed,sizeof(void *)},
    [20] = {ENTRY_TYPE_VFUNC_POINTER,"","mouse_moved",__mouse_moved,sizeof(void *)},
    [21] = {ENTRY_TYPE_VFUNC_POINTER,"","mouse_entered",NULL,sizeof(void *)},
    [22] = {ENTRY_TYPE_VFUNC_POINTER,"","mouse_exited",NULL,sizeof(void *)},
    [23] = {ENTRY_TYPE_VFUNC_POINTER,"","mouse_wheel_moved",__mouse_wheel_moved,sizeof(void *)},
    [24] = {ENTRY_TYPE_VFUNC_POINTER,"","window_moved",__window_moved,sizeof(void *)},
    [25] = {ENTRY_TYPE_VFUNC_POINTER,"","window_resized",__window_resized,sizeof(void *)},
    [26] = {ENTRY_TYPE_VFUNC_POINTER,"","is_mouse_over_component",__is_mouse_over_component,sizeof(void *)},
    [27] = {ENTRY_TYPE_FUNC_POINTER,"","add_event_listener",__add_event_listener,sizeof(void *)},
    [28] = {ENTRY_TYPE_FUNC_POINTER,"","add_event_listener_cb",__add_event_listener_cb,sizeof(void *)},
    [29] = {ENTRY_TYPE_STRING,"char","name",NULL,0},
    [30] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Component",component_class_info);

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
     *subject = OBJECT_ALLOC(allocator,Component);
     *object_set(subject, "Component", set_str);
     *dbg_str(DBG_DETAIL,"x=%d y=%d width=%d height=%d",subject->x,subject->y,subject->width,subject->height);
     */

    subject = OBJECT_NEW(allocator, Component,set_str);

    /*
     *dbg_str(DBG_DETAIL,"x=%d y=%d width=%d height=%d",subject->x,subject->y,subject->width,subject->height);
     *dbg_str(DBG_DETAIL,"component nane=%s",((Component *)subject)->name);
     *subject->move(subject);
     */

    object_dump(subject, "Component", buf, 2048);
    dbg_str(DBG_DETAIL,"Component dump: %s",buf);

    free(set_str);

}


