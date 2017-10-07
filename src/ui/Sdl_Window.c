/**
 * @file window_sdl.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-04
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
#include <unistd.h>
#include <libobject/ui/sdl_window.h>
#include <libobject/ui/sdl_graph.h>
#include <libobject/ui/sdl_image.h>
#include <libobject/ui/sdl_font.h>
#include <libobject/ui/sdl_event.h>
#include <libobject/ui/sdl_timer.h>

static int __set(Window *window, char *attrib, void *value)
{
    Sdl_Window *w = (Sdl_Window *)window;
    if (strcmp(attrib, "set") == 0) {
        w->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        w->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        w->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        w->deconstruct = value;
    } 
    else if (strcmp(attrib, "move") == 0) {
    } else if (strcmp(attrib, "create_font") == 0) {
        w->create_font = value;
    } else if (strcmp(attrib, "destroy_font") == 0) {
        w->destroy_font = value;
    } else if (strcmp(attrib, "create_graph") == 0) {
        w->create_graph = value;
    } else if (strcmp(attrib, "destroy_graph") == 0) {
        w->destroy_graph = value;
    } else if (strcmp(attrib, "create_event") == 0) {
        w->create_event = value;
    } else if (strcmp(attrib, "destroy_event") == 0) {
        w->destroy_event = value;
    } else if (strcmp(attrib, "create_background") == 0) {
        w->create_background = value;
    } else if (strcmp(attrib, "destroy_background") == 0) {
        w->destroy_background = value;
    } else if (strcmp(attrib, "init_window") == 0) {
        w->init_window = value;
    } else if (strcmp(attrib, "open_window") == 0) {
        w->open_window = value;
    } else if (strcmp(attrib, "close_window") == 0) {
        w->close_window = value;
    } else if (strcmp(attrib, "create_timer") == 0) {
        w->create_timer = value;
    } else if (strcmp(attrib, "remove_timer") == 0) {
        w->remove_timer = value;
    } else if (strcmp(attrib, "destroy_timer") == 0) {
        w->destroy_timer = value;
    }
    else {
        dbg_str(SDL_INTERFACE_DETAIL,"sdl window set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Window *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else {
        dbg_str(DBG_WARNNING,"sdl window get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void *__create_graph(Window *window, char *graph_type)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window create_graph");

    window->graph = (Graph *)OBJECT_NEW(allocator, Sdl_Graph,NULL);
}

static int __destroy_graph(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl window destroy_graph");
    object_destroy(window->graph);
}

static void *__create_font(Window *window,char *font_name)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window create_font");

    window->font = OBJECT_NEW(allocator, Sdl_Font,"");
    window->font->load_font(window->font);
}

static int __destroy_font(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl window destroy_font");
    window->font->load_font(window->font);
    object_destroy(window->font);
}

static void *__create_event(Window *window)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window create_event");

    window->event = OBJECT_NEW(allocator, Sdl_Event,"");
}

static int __destroy_event(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl window destroy_event");
    object_destroy(window->event);
}

static void *__create_background(Window *window, char *pic_path)
{
    allocator_t *allocator = ((Obj *)window)->allocator;
    Graph *g = window->graph;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window draw_background");

    window->background = g->render_load_image(g, pic_path);
}

static int __destroy_background(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL,"sdl window destroy_background");
    object_destroy(window->background);
}

static int __init_window(Window *window)
{
    int ret;
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(SDL_INTERFACE_DETAIL,"Sdl_Graph init window");
    dbg_str(SDL_INTERFACE_DETAIL,"srceen width=%d, height=%d",window->screen_width,window->screen_height);

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) {
        dbg_str(DBG_ERROR,"SDL could not initialize! SDL_Error: %s", SDL_GetError() );
        ret = -1;
    } else {
        //Create window
        w->sdl_window = SDL_CreateWindow("libcutils demo", 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED,
                                         window->screen_width, 
                                         window->screen_height,
                                         SDL_WINDOW_SHOWN);
        if ( w->sdl_window == NULL ) {
            dbg_str(DBG_ERROR,"Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            ret = -1;
        } else {
            Sdl_Graph *g = (Sdl_Graph *)window->graph;
            g->screen_surface = SDL_GetWindowSurface(g->window);
            ret = 1;
        }
    }

    return ret;
}

static int __open_window(Window *window)
{
    Graph *g = window->graph;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window open_window start");

    if (g == NULL) {
        dbg_str(DBG_ERROR,"window graph is NULL, please check");
        return -1;
    } else {
        /*
         *background->load_image(background);
         *g->draw_image(g,background);
         */
    }

    window->init_window(window);

    g->set_window(g, window);
    g->render_set_font(window->graph, window->font);

    g->render_create(g);
    g->render_set_color(g,0xff,0xff,0xff,0xff);
    g->render_clear(g);
    g->render_present(g);

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window open_window end");
    /*
     *g->update_window(g);
     */
}

static int __close_window(Window *window)
{
    Sdl_Graph *g = (Sdl_Graph *)window->graph;
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(SDL_INTERFACE_DETAIL,"sdl window close_window");

    //release screen surface
    if (g->screen_surface)
        SDL_FreeSurface( g->screen_surface );

    //destroy render
    g->render_destroy((Graph *)g);
    
    //Destroy window
    SDL_DestroyWindow(w->sdl_window);

    //Quit SDL subsystems
    SDL_Quit();
}

static void *__create_timer(Window *window)
{
    allocator_t *allocator = ((Obj *)window)->allocator;
    Sdl_Timer *timer;

    timer = OBJECT_NEW(allocator, Sdl_Timer,"");

    return timer;
}

static int __remove_timer(Window *window, Sdl_Timer *timer)
{
    SDL_RemoveTimer(timer->timer_id);
}

static int __destroy_timer(Window *window, void *timer)
{
    object_destroy(timer);
}

static class_info_entry_t sdl_window_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Window","window",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","create_graph",__create_graph,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","destroy_graph",__destroy_graph,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","create_font",__create_font,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","destroy_font",__destroy_font,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","create_event",__create_event,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","destroy_event",__destroy_event,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","create_background",__create_background,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","destroy_background",__destroy_background,sizeof(void *)},
    [11] = {ENTRY_TYPE_FUNC_POINTER,"","init_window",__init_window,sizeof(void *)},
    [12] = {ENTRY_TYPE_FUNC_POINTER,"","open_window",__open_window,sizeof(void *)},
    [13] = {ENTRY_TYPE_FUNC_POINTER,"","close_window",__close_window,sizeof(void *)},
    [14] = {ENTRY_TYPE_FUNC_POINTER,"","create_timer",__create_timer,sizeof(void *)},
    [15] = {ENTRY_TYPE_FUNC_POINTER,"","remove_timer",__remove_timer,sizeof(void *)},
    [16] = {ENTRY_TYPE_FUNC_POINTER,"","destroy_timer",__destroy_timer,sizeof(void *)},
    [17] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Sdl_Window",sdl_window_class_info);

char *gen_window_setting_str()
{
    cjson_t *root,*w, *c, *e, *s;
    char *set_str;

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Window", w = cjson_create_object());{
            cjson_add_item_to_object(root, "Component", c = cjson_create_object());{
                cjson_add_item_to_object(root, "Container", e = cjson_create_object());{
                    cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                        cjson_add_number_to_object(s, "x", 0);
                        cjson_add_number_to_object(s, "y", 0);
                        cjson_add_number_to_object(s, "width", 600);
                        cjson_add_number_to_object(s, "height", 600);
                        cjson_add_number_to_object(s, "x_speed", 1.2);
                        cjson_add_number_to_object(s, "y_speed", 2.3);
                    }
                    cjson_add_string_to_object(e, "name", "Container");
                    cjson_add_number_to_object(e, "map_type", 1);
                }
                cjson_add_string_to_object(c, "name", "Window");
            }
            cjson_add_string_to_object(w, "name", "Window");
            cjson_add_number_to_object(w, "screen_width", 600);
            cjson_add_number_to_object(w, "screen_height", 600);
        }
    }
    set_str = cjson_print(root);

    return set_str;
}

void test_ui_sdl_window()
{
    Window *window;
    Graph *g;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];

    set_str = gen_window_setting_str();

    dbg_str(DBG_SUC, "test_ui_sdl_window begin alloc count =%d",allocator->alloc_count);
    window  = OBJECT_NEW(allocator, Sdl_Window,set_str);
    g       = window->graph;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(SDL_INTERFACE_DETAIL,"Window dump: %s",buf);

    dbg_str(SDL_INTERFACE_DETAIL,"render draw test");
    g->render_draw_image(g,0,0,window->background);
    g->render_present(g);

    /*
     *sleep(2);
     */
    g->render_clear(g);
    g->render_set_color(g,0xff,0x0,0xff,0xff);
    g->render_draw_line(g,20,0,50,50);
    g->render_set_color(g,0xff,0x0,0x0,0xff);
    g->render_draw_rect(g,20,20,100,100);
    /*
     *g->render_fill_rect(g,20,20,100,100);
     */
    g->render_present(g);
    /*
     *sleep(5);
     */

    object_destroy(window);
    dbg_str(DBG_SUC, "test_ui_sdl_window end alloc count =%d",allocator->alloc_count);

    free(set_str);

}


