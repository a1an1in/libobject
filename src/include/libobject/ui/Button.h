#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Component.h>
#include <libobject/ui/Label.h>

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

    void (*on_mouse_pressed)(Component *component,void *event, void *window);
    void (*on_mouse_released)(Component *component,void *event, void *window);
    void (*on_mouse_entered)(Component *component,void *event, void *window);
    void (*on_mouse_exited)(Component *component,void *event, void *window);
    void (*on_mouse_moved)(Component *component,void *event, void *window);
    void (*on_mouse_dragged)(Component *component,void *event, void *window);
};

#endif
