#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/component.h>
#include <libobject/ui/label.h>

typedef struct button_s Button;

struct button_s{
	Component component;

	int (*construct)(Button *button,char *init_str);
	int (*deconstruct)(Button *button);
	int (*set)(Button *button, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    
    /*inherit methods*/
    int (*add_event_listener)(Component *component, event_listener_t *listener);
    int (*add_event_listener_cb)(Component *component, char *name, void *value);

	/*virtual methods reimplement*/
	int (*move)(Button *button);
	int (*draw)(Component *component, void *graph);

    void (*mouse_pressed)(Component *component,void *event, void *window);
    void (*mouse_released)(Component *component,void *event, void *window);
    void (*mouse_entered)(Component *component,void *event, void *window);
    void (*mouse_exited)(Component *component,void *event, void *window);
    void (*mouse_moved)(Component *component,void *event, void *window);
    void (*mouse_dragged)(Component *component,void *event, void *window);
};

#endif
