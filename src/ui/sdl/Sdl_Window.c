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
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Sdl_Render.h>
#include <libobject/ui/Sdl_Image.h>
#include <libobject/ui/Sdl_Font.h>
#include <libobject/ui/Sdl_Event.h>
#include <libobject/ui/Sdl_Timer.h>
#include <libobject/core/utils/config.h>

static int __construct(Window *window, char *init_str)
{
    Sdl_Window *w = (Sdl_Window *)window;
    dbg_str(DBG_INFO, "Sdl_window construct");

    w->flags = 0;

    return 0;
}

static void *__create_render(Window *window, char *type)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window create_render");

    window->render = (Render *)OBJECT_NEW(allocator, Sdl_Render, NULL);
}

static int __destroy_render(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl window destroy_render");
    object_destroy(window->render);
}

static void *__create_font(Window *window, char *font_name)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window create_font");

    window->font = OBJECT_NEW(allocator, Sdl_Font, "");
    window->font->load_font(window->font);
}

static int __destroy_font(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl window destroy_font");
    window->font->unload_font(window->font);
    object_destroy(window->font);
}

static void *__create_event(Window *window)
{
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window create_event");

    window->event = OBJECT_NEW(allocator, Sdl_Event, "");
}

static int __destroy_event(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl window destroy_event");
    object_destroy(window->event);
}

static void *__create_background(Window *window, char *pic_path)
{
    allocator_t *allocator = ((Obj *)window)->allocator;
    Render *r = window->render;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window draw_background");

    window->background = r->load_image(r, pic_path);
}

static int __destroy_background(Window *window)
{
    dbg_str(SDL_INTERFACE_DETAIL, "sdl window destroy_background");
    object_destroy(window->background);
}

static int __init_window(Window *window)
{
    int ret;
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(SDL_INTERFACE_DETAIL, "Sdl_Render init window");
    dbg_str(SDL_INTERFACE_DETAIL, "srceen width=%d, height=%d", window->screen_width, window->screen_height);

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) < 0 ) {
        dbg_str(DBG_ERROR, "SDL could not initialize! SDL_Error: %s", SDL_GetError() );
        ret = -1;
    } else {
        //Create window
        w->sdl_window = SDL_CreateWindow("libobject demo", 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         window->screen_width, 
                                         window->screen_height, 
                                         SDL_WINDOW_RESIZABLE);
        if ( w->sdl_window == NULL ) {
            dbg_str(DBG_ERROR, "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            ret = -1;
        } else {
            Sdl_Render *r = (Sdl_Render *)window->render;
            r->screen_surface = SDL_GetWindowSurface(r->window);
            ret = 1;
        }
    }

    return ret;
}

static int __open_window(Window *window)
{
    Render *r = window->render;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window open_window start");

    if (r == NULL) {
        dbg_str(DBG_ERROR, "window render is NULL, please check");
        return -1;
    } else {
        /*
         *background->load_image(background);
         *r->draw_image(r, background);
         */
    }

    window->init_window(window);

    r->set_window(r, window);
    r->set_font(window->render, window->font);

    r->create(r);
    /*
     *r->set_color(r, 0xff, 0xff, 0xff, 0xff);
     */
    r->clear(r);
    r->present(r);

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window open_window end");
    /*
     *r->update_window(r);
     */
}

static int __close_window(Window *window)
{
    Sdl_Render *r = (Sdl_Render *)window->render;
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(SDL_INTERFACE_DETAIL, "sdl window close_window");

    //release screen surface
    if (r->screen_surface)
        SDL_FreeSurface( r->screen_surface );

    //destroy render
    r->destroy((Render *)r);
    
    //Destroy window
    if (w->sdl_window)
        SDL_DestroyWindow(w->sdl_window);

    //Quit SDL subsystems
    SDL_Quit();
}

static void *__create_timer(Window *window)
{
    allocator_t *allocator = ((Obj *)window)->allocator;
    Sdl_Timer *timer;

    timer = OBJECT_NEW(allocator, Sdl_Timer, "");

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

static int __poll_event(Window *window)
{
    __Event *event;

    event = window->event;

    event->poll_event(event, window);

    return 0;
}

static void __set_window_title(Window *window, void *title) 
{
    Sdl_Window *w = (Sdl_Window *)window;
    dbg_str(DBG_DETAIL, "set_window_title");
    SDL_SetWindowTitle(w->sdl_window, title);
}

static void __set_window_icon(Window *window, void *title) 
{
    dbg_str(DBG_DETAIL, "window moved");
}
static void __set_window_size(Window *window, int width, int hight) 
{
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(DBG_DETAIL, "set window size");
    SDL_SetWindowSize(w->sdl_window, width, hight);
}

static void __set_window_position(Window *window, int x, int y) 
{
    dbg_str(DBG_DETAIL, "set window position");
}

#define FULLSCREEN_MASK (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)
#include <SDL2/SDL_video.h>
static void __full_screen(Window *window) 
{
    Sdl_Window *w = (Sdl_Window *)window;
    SDL_Window * sdl_w = (SDL_Window *)w->sdl_window;
    uint32_t flags = w->flags;

    /*
     *dbg_str(DBG_DETAIL, "full_screen");
     */
#if 0
    if (flags & FULLSCREEN_MASK){
        w->flags &= ~FULLSCREEN_MASK;
        SDL_SetWindowFullscreen(w->sdl_window, SDL_FALSE);
        SDL_GetWindowSize(w->sdl_window, 
                          &window->screen_width, 
                          &window->screen_height);
        dbg_str(DBG_DETAIL, "exit full_screen, window size, width=%d, height=%d", 
                window->screen_width, window->screen_height);
    } else {
        w->flags |= FULLSCREEN_MASK;
        SDL_SetWindowFullscreen(w->sdl_window, FULLSCREEN_MASK);
        SDL_GetWindowSize(w->sdl_window, 
                          &window->screen_width, 
                          &window->screen_height);
        dbg_str(DBG_DETAIL, "full_screen, window size, width=%d, height=%d", 
                window->screen_width, window->screen_height);
        /*
         *dbg_str(DBG_DETAIL, "full_screen, w width=%d, height=%d", 
         *        sdl_w->w, sdl_w->h);
         */
    }
#else 
    w->flags |= FULLSCREEN_MASK;
    SDL_SetWindowFullscreen(w->sdl_window, FULLSCREEN_MASK);
    SDL_GetWindowSize(w->sdl_window, 
                      &window->screen_width, 
                      &window->screen_height);

    dbg_str(DBG_DETAIL, "full_screen, window size, width=%d, height=%d", 
            window->screen_width, window->screen_height);
#endif
}

static void __maximize_window(Window *window) 
{
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(DBG_DETAIL, "maximize_window");

    SDL_MaximizeWindow(w->sdl_window);
    SDL_GetWindowSize(w->sdl_window, 
                      &window->screen_width, 
                      &window->screen_height);

    dbg_str(DBG_DETAIL, "maximize_window, window size, width=%d, height=%d", 
            window->screen_width, window->screen_height);
}

static void __minimize_window(Window *window) 
{
    Sdl_Window *w = (Sdl_Window *)window;

    dbg_str(DBG_DETAIL, "minimize_window");

    SDL_MinimizeWindow(w->sdl_window);
    SDL_GetWindowSize(w->sdl_window, 
                      &window->screen_width, 
                      &window->screen_height);
}

static void __restore_window(Window *window) 
{
    Sdl_Window *w = (Sdl_Window *)window;

    SDL_RestoreWindow(w->sdl_window);
    SDL_GetWindowSize(w->sdl_window, 
                      &window->screen_width, 
                      &window->screen_height);

    dbg_str(DBG_DETAIL, "restore_window, window size, width=%d, height=%d", 
            window->screen_width, window->screen_height);
}

static class_info_entry_t sdl_window_class_info[] = {
    Init_Obj___Entry(0 , Window, window),
    Init_Nfunc_Entry(1 , Sdl_Window, construct, __construct),
    Init_Nfunc_Entry(2 , Sdl_Window, deconstruct, NULL),
    Init_Vfunc_Entry(3 , Sdl_Window, set, NULL),
    Init_Vfunc_Entry(4 , Sdl_Window, get, NULL),
    Init_Vfunc_Entry(5 , Sdl_Window, create_render, __create_render),
    Init_Vfunc_Entry(6 , Sdl_Window, destroy_render, __destroy_render),
    Init_Vfunc_Entry(7 , Sdl_Window, create_font, __create_font),
    Init_Vfunc_Entry(8 , Sdl_Window, destroy_font, __destroy_font),
    Init_Vfunc_Entry(9 , Sdl_Window, create_event, __create_event),
    Init_Vfunc_Entry(10, Sdl_Window, destroy_event, __destroy_event),
    Init_Vfunc_Entry(11, Sdl_Window, create_background, __create_background),
    Init_Vfunc_Entry(12, Sdl_Window, destroy_background, __destroy_background),
    Init_Vfunc_Entry(13, Sdl_Window, init_window, __init_window),
    Init_Vfunc_Entry(14, Sdl_Window, open_window, __open_window),
    Init_Vfunc_Entry(15, Sdl_Window, close_window, __close_window),
    Init_Vfunc_Entry(16, Sdl_Window, create_timer, __create_timer),
    Init_Vfunc_Entry(17, Sdl_Window, remove_timer, __remove_timer),
    Init_Vfunc_Entry(18, Sdl_Window, destroy_timer, __destroy_timer),
    Init_Vfunc_Entry(19, Sdl_Window, poll_event, __poll_event),
    Init_Vfunc_Entry(20, Sdl_Window, set_window_title, __set_window_title),
    Init_Vfunc_Entry(21, Sdl_Window, set_window_icon, __set_window_icon),
    Init_Vfunc_Entry(22, Sdl_Window, set_window_size, __set_window_size),
    Init_Vfunc_Entry(23, Sdl_Window, set_window_position, __set_window_position),
    Init_Vfunc_Entry(24, Sdl_Window, full_screen, __full_screen),
    Init_Vfunc_Entry(25, Sdl_Window, maximize_window, __maximize_window),
    Init_Vfunc_Entry(26, Sdl_Window, minimize_window, __minimize_window),
    Init_Vfunc_Entry(27, Sdl_Window, restore_window, __restore_window),
    Init_End___Entry(28, Sdl_Window),
};
REGISTER_CLASS(Sdl_Window, sdl_window_class_info);

char *gen_window_setting_str()
{
    cjson_t *root, *w, *c, *e, *s;
    char *set_str;

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Window", w = cjson_create_object());{
            cjson_add_item_to_object(root, "Component", c = cjson_create_object());{
                cjson_add_item_to_object(root, "Container", e = cjson_create_object());{
                    cjson_add_item_to_object(e, "Subject", s = cjson_create_object());{
                        cjson_add_number_to_object(s, "x", 0);
                        cjson_add_number_to_object(s, "y", 0);
                        cjson_add_number_to_object(s, "width", 900);
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
            cjson_add_number_to_object(w, "screen_width", 900);
            cjson_add_number_to_object(w, "screen_height", 600);
        }
    }
    set_str = cjson_print(root);

    return set_str;
}

int sdl_window()
{
    Window *window;
    Render *r;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str;
    char buf[2048];
    configurator_t * c;

    c = cfg_alloc(allocator); 
    cfg_config_str(c, "/Window", "name", "Window");
    cfg_config_num(c, "/Window", "screen_width", 600);
    cfg_config_num(c, "/Window", "screen_height", 600);
    cfg_config_str(c, "/Window/Component", "name", "Component");
    cfg_config_str(c, "/Window/Component/Container", "name", "Container");
    cfg_config_num(c, "/Window/Component/Container", "map_type", 1);
    cfg_config_str(c, "/Window/Component/Container/Subject", "name", "Subject");
    cfg_config_num(c, "/Window/Component/Container/Subject", "x", 0);
    cfg_config_num(c, "/Window/Component/Container/Subject", "y", 0);
    cfg_config_num(c, "/Window/Component/Container/Subject", "width", 600);
    cfg_config_num(c, "/Window/Component/Container/Subject", "height", 600);
    cfg_config_num(c, "/Window/Component/Container/Subject", "x_speed", 2);
    cfg_config_num(c, "/Window/Component/Container/Subject", "y_speed", 2);
    dbg_str(DBG_DETAIL, "config:%s", c->buf);

#if 1

    dbg_str(DBG_VIP, "test_ui_sdl_window begin alloc count =%d", allocator->alloc_count);
    window  = OBJECT_NEW(allocator, Sdl_Window, c->buf);
    r       = window->render;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(SDL_INTERFACE_DETAIL, "Window dump: %s", buf);

    dbg_str(SDL_INTERFACE_DETAIL, "render draw test");
    r->set_color(r, 0, 0, 0, 0xff);
    /*
     *r->draw_image(r, 0, 0, window->background);
     */
    r->present(r);

    r->clear(r);
    r->set_color(r, 0xf, 0x25, 0x56, 0xff);
    r->draw_line(r, 20, 0, 50, 50);
    r->set_color(r, 0xff, 0x0, 0x0, 0xff);
    r->draw_rect(r, 20, 20, 100, 100);
    /*
     *r->fill_rect(r, 20, 20, 100, 100);
     */
    r->present(r);
    /*
     *sleep(5);
     */

    window->event->poll_event(window->event, window);
    /*
     *pause();
     */
    object_destroy(window);
    dbg_str(DBG_VIP, "test_ui_sdl_window end alloc count =%d", allocator->alloc_count);

#endif
    cfg_destroy(c);

    return 1;

}
REGISTER_TEST_CMD(sdl_window);


