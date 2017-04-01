#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/container.h>

typedef struct component_s Component;

typedef struct event_listener_s{
    void (*do_mouse_pressed)(Component *component,void *event, void *window);
    void (*do_mouse_released)(Component *component,void *event, void *window);
    void (*do_mouse_entered)(Component *component,void *event, void *window);
    void (*do_mouse_exited)(Component *component,void *event, void *window);
    void (*do_mouse_moved)(Component *component,void *event, void *window);
    void (*do_mouse_dragged)(Component *component,void *event, void *window);
    void (*do_mouse_wheel_moved)(Component *component,void *event, void *window);
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

	int (*key_text_pressed)(Component *component,char c, void *graph);
	int (*key_backspace_pressed)(Component *component,void *graph);
	int (*key_up_pressed)(Component *component,void *graph);
	int (*key_down_pressed)(Component *component,void *graph);
	int (*key_left_pressed)(Component *component,void *graph);
	int (*key_right_pressed)(Component *component,void *graph);
	int (*key_pageup_pressed)(Component *component,void *graph);
	int (*key_pagedown_pressed)(Component *component,void *graph);
	int (*key_onelineup_pressed)(Component *component,void *graph);
	int (*key_onelinedown_pressed)(Component *component,void *graph);
    
    void (*mouse_pressed)(Component *component,void *event, void *window);
    void (*mouse_released)(Component *component,void *event, void *window);
    void (*mouse_entered)(Component *component,void *event, void *window);
    void (*mouse_exited)(Component *component,void *event, void *window);
    void (*mouse_moved)(Component *component,void *event, void *window);
    void (*mouse_dragged)(Component *component,void *event, void *window);
    void (*mouse_wheel_moved)(Component *component,void *event, void *window);
    int  (*is_mouse_over_component)(Component *component,void *event);

    void (*window_moved)(Component *component,void *event, void *window);
    void (*window_resized)(Component *component,void *event, void *window);


#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    unsigned char mouse_entered_flag;
    event_listener_t listener;

};

#endif
