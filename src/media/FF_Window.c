/**
 * @file concurrent.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-09-24
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
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Sdl_Audio.h>
#include <libobject/event/event_compat.h>
#include <libobject/media/FF_Window.h>
#include <libobject/media/FF_Event.h>
#include <libobject/media/Player.h>

static int __construct(FF_Window *goya,char *init_str)
{
    allocator_t *allocator = ((Obj *)goya)->allocator;

    return 0;
}

static int __deconstrcut(FF_Window *goya)
{

    return 0;
}

static int __set(FF_Window *window, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        window->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        window->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        window->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        window->deconstruct = value;
    } else if (strcmp(attrib, "on_key_text_pressed") == 0) {
        window->on_key_text_pressed = value;
    } else if (strcmp(attrib, "on_key_backspace_pressed") == 0) {
        window->on_key_backspace_pressed = value;
    } else if (strcmp(attrib, "on_key_up_pressed") == 0) {
        window->on_key_up_pressed = value;
    } else if (strcmp(attrib, "on_key_down_pressed") == 0) {
        window->on_key_down_pressed = value;
    } else if (strcmp(attrib, "on_key_left_pressed") == 0) {
        window->on_key_left_pressed = value;
    } else if (strcmp(attrib, "on_key_right_pressed") == 0) {
        window->on_key_right_pressed = value;
    } else if (strcmp(attrib, "on_key_pageup_pressed") == 0) {
        window->on_key_pageup_pressed = value;
    } else if (strcmp(attrib, "on_key_pagedown_pressed") == 0) {
        window->on_key_pagedown_pressed = value;
    } else if (strcmp(attrib, "on_key_esc_pressed") == 0) {
        window->on_key_esc_pressed = value;
    } else if (strcmp(attrib, "on_mouse_pressed") == 0) {
        window->on_mouse_pressed = value;
    } else if (strcmp(attrib, "on_mouse_moved") == 0) {
        window->on_mouse_moved = value;
    } else if (strcmp(attrib, "on_mouse_entered") == 0) {
        window->on_mouse_entered = value;
    } else if (strcmp(attrib, "on_mouse_exited") == 0) {
        window->on_mouse_exited = value;
    } else if (strcmp(attrib, "on_mouse_wheel_moved") == 0) {
        window->on_mouse_wheel_moved = value;
    } else if (strcmp(attrib, "on_window_moved") == 0) {
        window->on_window_moved = value;
    } else if (strcmp(attrib, "on_window_resized") == 0) {
        window->on_window_resized = value;
    } else if (strcmp(attrib, "create_event") == 0) {
        window->create_event = value;
    } else if (strcmp(attrib, "destroy_event") == 0) {
        window->destroy_event = value;
    } else if (strcmp(attrib, "add_player") == 0) {
        window->add_player = value;
    } else if (strcmp(attrib, "add_component") == 0) {
        window->add_component = value;
    } else if (strcmp(attrib, "update_window") == 0) {
        window->update_window = value;
    } else if (strcmp(attrib, "poll_event") == 0) {
        window->poll_event = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"goya set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FF_Window *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"goya get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void __on_key_text_pressed(Window *window, char c, void *render)
{
    Render *g      = (Render *)render;
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;
    static int seek_count;

    dbg_str(DBG_WARN, "goya window, on_text_key_pressed: %c", c);

    switch (c) {
        case 'f':
            {
                if (player->is_state(player, STATE_FULLSCREEN)) {
                    SDL_SetWindowFullscreen(w->parent.sdl_window, SDL_FALSE);
                    SDL_GetWindowSize(w->parent.sdl_window,
                                      &window->screen_width,
                                      &window->screen_height);
                    player->clear_state(player, STATE_FULLSCREEN);
                } else {
                    window->full_screen(window);
                    player->set_state(player, STATE_FULLSCREEN);
                }
            }
            break;
        case 'p':
        case ' ':
            {
                if (player->is_state(player, STATE_PLAYING)) {
                    player->stop(player);
                } else if (player->is_state(player, STATE_STOPPED)) {
                    player->start(player);
                }
            }
            break;
        case 'q':
            player->exit(player);
            break;
        case 'd':
            {
                debugger_set_business(debugger_gp, 0, 1, 9);
            }
            break;
        case 'D':
            {
                debugger_set_business(debugger_gp, 0, 1, 6);
            }
            break;
        case '1':
            window->maximize_window(window);
            break;
        case '2':
            window->minimize_window(window);
            break;
        case '3':
            window->restore_window(window);
            break;
        case '4':
            {
                int ret;
                int64_t duration;

                duration = player->get_duration(player);
                ret = player->get_position(player);
                dbg_str(DBG_VIP,"play position:%d, duration=%d", ret, duration);
            }
            break;
        case '5':
            {
#if 1
                int ret = 0;
                /*
                 *int ret = 630;
                 */
                ret  += (seek_count + 1) * 100;
                dbg_str(DBG_VIP,"seek to :%ds", ret);
                ret = player->seek_to(player, ret);
                player->start(player);

                seek_count++;
                /*
                 *ret = player->get_position(player);
                 *dbg_str(DBG_VIP,"play position:%d", ret);
                 */
#else
                int ret = 630;
                dbg_str(DBG_VIP,"seek to :%ds", ret);
                ret = player->seek_to(player, ret);
                player->start(player);
#endif
            }
            break;
        case '6':
            {
                player->switch_bitrate(player, 460560);
            }
            break;
        case '7':
            {
                int data[1024 * 10];
                int len = 0;
                int ret = 0;

                ret = player->get_rgb_from_current_frame(player,
                        AV_PIX_FMT_RGB24,
                        600, 400,
                        data, &len);
                if (ret < 0) {
                    dbg_str(DBG_VIP,"get_rgb_from_current_frame err");
                } else {
                    dbg_str(DBG_VIP,"total len=%d, linesize=%d", ret, len);
                    dbg_buf(DBG_VIP,"rbg data:", (unsigned char *)data, 10);
                }
            }
            break;
        case '8':
            {
                seek_count = 0;
                /*
                 *player->open(player, "http://ys-qf.cloutropy.com/mts_in/softlink/17c32c42e3bd68c407cd99906c09a99e.mkv");
                 */
                /*
                 *player->open(player, "http://ys-mf.cloutropy.com/mts_in/softlink/0fa70594873d0a3701661c22883efdc8.mp4");
                 */
                player->open(player, "http://ys-mf.cloutropy.com/mts_in/softlink/a8a746b2e3e98fde9b760573b0fcdde5.mp4");
                player->start(player);
            }
            break;
        case '9':
            {
                seek_count = 0;
                player->open(player, "http://yunshangvideo.oss-cn-beijing.aliyuncs.com/mts_out/m3u8test/x36xhzz/x36xhzz.m3u8");
                player->start(player);
            }

            break;
        case '0':
            {
                player->print(player);
            }
            break;
        default:
            break;

    }
}

static void __on_key_up_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;

    dbg_str(DBG_VIP, "key_up_pressed");

}

static void __on_key_down_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;

    dbg_str(DBG_VIP, "key_down_pressed");

}

static void __on_key_left_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;
    int ret;

    dbg_str(DBG_VIP, "key_right_pressed");
    ret = player->get_position(player);

    dbg_str(DBG_VIP,"seek, play position:%d", ret);

    ret -= 10;
    if (ret <= 0) {
        return;
    }

    ret = player->seek_to(player, ret);

}

static void __on_key_right_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;
    int ret;
    int duration;

    dbg_str(DBG_VIP, "key_right_pressed");

    ret = player->get_position(player);
    duration = player->get_duration(player);

    dbg_str(DBG_VIP,"seek, play position:%d", ret);

    ret += 10;
    if (ret > duration) {
        return;
    }

    ret = player->seek_to(player, ret);
}

static void __on_key_pageup_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;

}

static void __on_key_pagedown_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;

}

static void __on_key_backspace_pressed(FF_Window *window, void *render) 
{
    FF_Window *w   = (FF_Window *)window;
    Player *player = w->player;

}

static void __on_mouse_pressed(Window *window, void *event, void *win) 
{
    FF_Window *ff_window = (FF_Window *)window;
    Player *player       = ff_window->player;
    Render *g            = window->render;
    Sdl_Event *sdl_event = (Sdl_Event *)event;
    SDL_Event *e         = &sdl_event->ev;
    static uint64_t last_mouse_left_click;
    static uint64_t cur_mouse_left_click;

    dbg_str(DBG_VIP, "on_mouse_pressed");
    if (e->button.button == SDL_BUTTON_LEFT) {
        cur_mouse_left_click = av_gettime_relative();
        dbg_str(DBG_VIP, "last_mouse_left_click = %u, cur_mouse_left_click=%u, diff =%u",
                last_mouse_left_click,
                cur_mouse_left_click,
                cur_mouse_left_click - last_mouse_left_click);
        if (cur_mouse_left_click - last_mouse_left_click <= 500000) {
            if (player->is_state(player, STATE_FULLSCREEN)) {
                SDL_SetWindowFullscreen(ff_window->parent.sdl_window, 
                                        SDL_FALSE);
                SDL_GetWindowSize(ff_window->parent.sdl_window,
                                  &window->screen_width,
                                  &window->screen_height);
                player->clear_state(player, STATE_FULLSCREEN);
                dbg_str(DBG_VIP, "left mouse dobule clicked, exit full_screen");
            } else {
                window->full_screen(window);
                player->set_state(player, STATE_FULLSCREEN);
                dbg_str(DBG_VIP, "left mouse dobule clicked, full_screen");
            }
            last_mouse_left_click = 0;
        } else {
            last_mouse_left_click = av_gettime_relative();
        }

    }
}

static void __on_mouse_moved(FF_Window *window, void *event, void *win) 
{
    Container *container = (Container *)window;
    Window *w            = (Window *)window;
    Render *g             = w->render;
    __Event *e           = (__Event *)event;
    FF_Window *cur;

    /*
     *dbg_str(DBG_DETAIL, "EVENT: Mouse: moved to %d, %d (%d, %d) in window %d", 
     *        e->x, e->y, e->xrel, e->yrel, e->windowid);
     */
}

static void __on_mouse_wheel_moved(FF_Window *window, void *event, void *win) 
{
    __Event *e = (__Event *)event;

    dbg_str(DBG_DETAIL, 
            "EVENT: Mouse: wheel scrolled %d in x and %d in y (direction: %d) in window %d", 
            e->x, e->y, e->direction, e->windowid);
}

static void __on_window_moved(FF_Window *window, void *event, void *win) 
{
    dbg_str(DBG_DETAIL, "window moved");
}

static void __on_window_resized(FF_Window *window, void *event, void *win) 
{
    Window *w            = (Window *)window;
    FF_Window *ff_window = (FF_Window *)window;
    Player *player       = ff_window->player;
    Image *image         = player->image;
    __Event *e           = (__Event *)event;
    Render *r            = w->render;

    dbg_str(DBG_VIP, "EVENT: window id %d, resized to %dx%d",
            e->windowid, e->data1, e->data2);

    w->screen_width = e->data1;
    w->screen_height = e->data2;
    player->screen_width  = w->screen_width;
    player->screen_height = w->screen_height;

    if (    player->is_state(player, STATE_STOPPING) || 
            player->is_state(player, STATE_STOPPED))
    {
        r->set_color(r, 0, 0, 0, 0xff);
        r->clear(r);
        r->present(r);
        player->draw_current_image(player);
    }
}

static void __create_event(FF_Window *window)
{
    Window *w = (Window *)window;
    allocator_t *allocator = ((Obj *)window)->allocator;

    dbg_str(DBG_INFO, "window create event");
    w->event = OBJECT_NEW(allocator, FF_Event, "");
}

void __destroy_event(FF_Window *window)
{
    Window *w = (Window *)window;

    dbg_str(DBG_INFO, "window destroy event");
    object_destroy(w->event);
}

void __add_player(FF_Window *window, void *player)
{
    window->player = player;
    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Sdl_Window", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_text_pressed", __on_key_text_pressed, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_backspace_pressed", __on_key_backspace_pressed, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_up_pressed", __on_key_up_pressed, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_down_pressed", __on_key_down_pressed, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_left_pressed", __on_key_left_pressed, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_right_pressed", __on_key_right_pressed, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_pageup_pressed", __on_key_pageup_pressed, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_pagedown_pressed", __on_key_pagedown_pressed, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_key_esc_pressed", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_mouse_pressed", __on_mouse_pressed, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_mouse_moved", __on_mouse_moved, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_mouse_entered", NULL, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_mouse_exited", NULL, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_mouse_wheel_moved", __on_mouse_wheel_moved, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_window_moved", __on_window_moved, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "on_window_resized", __on_window_resized, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "create_event", __create_event, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "destroy_event", __destroy_event, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_player", __add_player, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_component", NULL, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_VFUNC_POINTER, "", "update_window", NULL, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "poll_event", NULL, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS(FF_Window, concurent_class_info);
