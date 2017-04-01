#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/component.h>
#include <libobject/ui/graph.h>
#include <libobject/ui/image.h>
#include <libobject/ui/font.h>
#include <libobject/ui/event.h>

typedef struct window_s Window;

struct window_s{
	Component component;

	int (*construct)(Window *window,char *init_str);
	int (*deconstruct)(Window *window);
	int (*set)(Window *window, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*update_window)(Window *window);

	/*virtual methods reimplement*/
	int (*load_resources)(Window *window);
	int (*unload_resources)(Window *window);
	int (*move)(Window *window);
    void *(*create_font)(Window *window, char *font_name);
    int (*destroy_font)(Window *window);
    void *(*create_graph)(Window *window, char *graph_type);
    int (*destroy_graph)(Window *window);
    void *(*create_event)(Window *window);
    int (*destroy_event)(Window *window);
    void *(*create_background)(Window *window, char *pic_path);
    int (*destroy_background)(Window *window);
    int (*init_window)(Window *window);
    int (*open_window)(Window *window);
    int (*close_window)(Window *window);
    void *(*create_timer)(Window *window);
    int (*remove_timer)(Window *window, void *timer);
    int (*destroy_timer)(Window *window, void *timer);

    /*inherit methods*/
    int (*add_component)(Container *obj, void *pos, void *component);

    /*attribs*/
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
	uint8_t graph_type;

	Graph *graph;
	Font *font;
	Image *background;
    __Event *event;

	int screen_width;
	int screen_height;
};

#endif
