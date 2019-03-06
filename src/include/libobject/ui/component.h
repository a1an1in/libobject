#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/container.h>
#include <libobject/ui/render.h>

typedef struct component_s Component;

typedef struct event_listener_s{
    void (*do_on_mouse_pressed)(Component *component,void *event, void *window);
    void (*do_on_mouse_released)(Component *component,void *event, void *window);
    void (*do_on_mouse_entered)(Component *component,void *event, void *window);
    void (*do_on_mouse_exited)(Component *component,void *event, void *window);
    void (*do_on_mouse_moved)(Component *component,void *event, void *window);
    void (*do_on_mouse_dragged)(Component *component,void *event, void *window);
    void (*do_on_mouse_wheel_moved)(Component *component,void *event, void *window);
}event_listener_t;

struct component_s{
	Container container;

    /*normal methods*/
	int (*construct)(Component *component,char *init_str);
	int (*deconstruct)(Component *component);
	int (*set)(Component *component, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add_event_listener)(Component *component, event_listener_t *listener);
    int (*add_event_listener_cb)(Component *component, char *name, void *value);

	/*virtual methods*/
	int (*move)(Component *component);
	int (*draw)(Component *component, void *graph);
	int (*load_resources)(Component *component, void *window);
	int (*unload_resources)(Component *component, void *window);

	int (*on_key_text_pressed)(Component *component,char c, void *graph);
	int (*on_key_backspace_pressed)(Component *component,void *graph);
	int (*on_key_up_pressed)(Component *component,void *graph);
	int (*on_key_down_pressed)(Component *component,void *graph);
	int (*on_key_left_pressed)(Component *component,void *graph);
	int (*on_key_right_pressed)(Component *component,void *graph);
	int (*on_key_pageup_pressed)(Component *component,void *graph);
	int (*on_key_pagedown_pressed)(Component *component,void *graph);
	int (*on_key_onelineup_pressed)(Component *component,void *graph);
	int (*on_key_onelinedown_pressed)(Component *component,void *graph);
	int (*on_key_esc_pressed)(Component *component,void *graph);
    
    void (*on_mouse_pressed)(Component *component,void *event, void *window);
    void (*on_mouse_released)(Component *component,void *event, void *window);
    void (*on_mouse_entered)(Component *component,void *event, void *window);
    void (*on_mouse_exited)(Component *component,void *event, void *window);
    void (*on_mouse_moved)(Component *component,void *event, void *window);
    void (*on_mouse_dragged)(Component *component,void *event, void *window);
    void (*on_mouse_wheel_moved)(Component *component,void *event, void *window);
    int  (*is_mouse_over_component)(Component *component,void *event);

    void (*on_window_moved)(Component *component,void *event, void *window);
    void (*on_window_resized)(Component *component,void *event, void *window);
    void (*set_name)(Component *component,void *name);


#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    unsigned char on_mouse_entered_flag;
    event_listener_t listener;
};

#endif
