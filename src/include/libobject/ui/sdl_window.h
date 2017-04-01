#ifndef __SDL_WINDOW_H__
#define __SDL_WINDOW_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/component.h>
#include <libobject/ui/graph.h>
#include <libobject/ui/image.h>
#include <libobject/ui/window.h>
#include <SDL2/SDL.h>

typedef struct sdl_window_s Sdl_Window;

struct sdl_window_s{
	Window window;

	int (*construct)(Window *window,char *init_str);
	int (*deconstruct)(Window *window);
	int (*set)(Window *window, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
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

	SDL_Window* sdl_window;
};

char *gen_window_setting_str();
#endif
