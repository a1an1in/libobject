/**
 * @file event_sdl.c
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
#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/sdl_event.h>
#include <libobject/ui/sdl_window.h>

static void
print_string(char **text, size_t *maxlen, const char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = SDL_vsnprintf(*text, *maxlen, fmt, ap);
    if (len > 0) {
        *text += len;
        if ( ((size_t) len) < *maxlen ) {
            *maxlen -= (size_t) len;
        } else {
            *maxlen = 0;
        }
    }
    va_end(ap);
}

static void
print_modifiers(char **text, size_t *maxlen)
{
    int mod;
    print_string(text, maxlen, " modifiers:");
    mod = SDL_GetModState();
    if (!mod) {
        print_string(text, maxlen, " (none)");
        return;
    }
    if (mod & KMOD_LSHIFT)
        print_string(text, maxlen, " LSHIFT");
    if (mod & KMOD_RSHIFT)
        print_string(text, maxlen, " RSHIFT");
    if (mod & KMOD_LCTRL)
        print_string(text, maxlen, " LCTRL");
    if (mod & KMOD_RCTRL)
        print_string(text, maxlen, " RCTRL");
    if (mod & KMOD_LALT)
        print_string(text, maxlen, " LALT");
    if (mod & KMOD_RALT)
        print_string(text, maxlen, " RALT");
    if (mod & KMOD_LGUI)
        print_string(text, maxlen, " LGUI");
    if (mod & KMOD_RGUI)
        print_string(text, maxlen, " RGUI");
    if (mod & KMOD_NUM)
        print_string(text, maxlen, " NUM");
    if (mod & KMOD_CAPS)
        print_string(text, maxlen, " CAPS");
    if (mod & KMOD_MODE)
        print_string(text, maxlen, " MODE");
}

static void
print_modifier_state()
{
    char message[512];
    char *spot;
    size_t left;

    spot = message;
    left = sizeof(message);

    print_modifiers(&spot, &left);
    dbg_str(DBG_DETAIL,"Initial state:%s", message);
}

static void
print_key(SDL_Keysym * sym, SDL_bool pressed, SDL_bool repeat)
{
    char message[512];
    char *spot;
    size_t left;

    spot = message;
    left = sizeof(message);

    /* Print the keycode, name and state */
    if (sym->sym) {
        print_string(&spot, &left,
                     "Key %s:  scancode %d = %s, keycode 0x%08X = %s ",
                     pressed ? "pressed " : "released",
                     sym->scancode,
                     SDL_GetScancodeName(sym->scancode),
                     sym->sym, SDL_GetKeyName(sym->sym));
    } else {
        print_string(&spot, &left,
                     "Unknown Key (scancode %d = %s) %s ",
                     sym->scancode,
                     SDL_GetScancodeName(sym->scancode),
                     pressed ? "pressed " : "released");
    }
    print_modifiers(&spot, &left);
    if (repeat) {
        print_string(&spot, &left, " (repeat)");
    }
    dbg_str(DBG_DETAIL,"%s", message);
}

static void
print_text(char *eventtype, char *text)
{
    char *spot, expanded[1024];

    expanded[0] = '\0';
    for ( spot = text; *spot; ++spot )
    {
        size_t length = SDL_strlen(expanded);
        SDL_snprintf(expanded + length, sizeof(expanded) - length, "\\x%.2x", (unsigned char)*spot);
    }
    dbg_str(DBG_DETAIL, "%s Text (%s): \"%s%s\"", eventtype, expanded, *text == '"' ? "\\" : "", text);
}

static int __construct(__Event *event,char *init_str)
{
    dbg_str(OBJ_DETAIL,"event construct, event addr:%p",event);

    return 0;
}

static int __deconstrcut(__Event *event)
{
    __Event *i = (__Event *)event;
    dbg_str(OBJ_DETAIL,"event deconstruct,event addr:%p",event);


    return 0;
}

static int __set(__Event *event, char *attrib, void *value)
{
    Sdl_Event *e = (Sdl_Event *)event;

    if (strcmp(attrib, "set") == 0) {
        e->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        e->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        e->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        e->deconstruct = value;
    } else if (strcmp(attrib, "poll_event") == 0) {
        e->poll_event = value;
    } else {
        dbg_str(OBJ_WARNNING,"event set,  \"%s\" setting is not support",attrib);
    }

    return 0;
}

static void * __get(__Event *event, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING,"event get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __poll_event(__Event *event,void *window)
{
    int quit             = 0;
    SDL_Event *e         = &((Sdl_Event *)event)->ev;
    Window *w            = (Window *)window;
    Graph *g             = w->graph;
    Component *component = (Component *)window, *cur;
     
    dbg_str(DBG_DETAIL,"sdl event poll");

    SDL_StartTextInput();

    while(!quit) {
         while(SDL_PollEvent(e) != 0) {
             switch(e->type) {
                 case SDL_QUIT:
                     quit = 1; 
                     break;
                 case SDL_WINDOWEVENT:
                     switch (e->window.event) {
                         case SDL_WINDOWEVENT_MOVED:
                             event->windowid = e->button.windowID;
                             event->window   = window;
                             component->window_moved(component, event, window);
                             break;
                         case SDL_WINDOWEVENT_RESIZED:
                             event->data1    = e->window.data1;
                             event->data2    = e->window.data2;
                             event->windowid = e->button.windowID;
                             event->window   = window;
                             component->window_resized(component, event, window);
                             break;
                     }
                     break;
                 case SDL_KEYDOWN:
                     /*
                      *print_key(&e->key.keysym, 
                      *          (e->key.state == SDL_PRESSED) ? SDL_TRUE : SDL_FALSE,
                      *          (e->key.repeat) ? SDL_TRUE : SDL_FALSE);
                      */

                     if ((e->key.repeat) ? SDL_TRUE : SDL_FALSE) {
                         break;
                     }
                     switch(e->key.keysym.sym) {
                         case SDLK_UP:
                             /*
                              *dbg_str(DBG_DETAIL,"SDLK_UP, code :%x",e->key.keysym.sym);
                              */
                             component->key_up_pressed(component, g);
                             break;
                         case SDLK_DOWN:
                             /*
                              *dbg_str(DBG_DETAIL,"SDLK_DOWN, code :%x",e->key.keysym.sym);
                              */
                             component->key_down_pressed(component, g);
                             break;
                         case SDLK_LEFT:
                             /*
                              *dbg_str(DBG_DETAIL,"SDLK_LEFT, code :%x",e->key.keysym.sym);
                              */
                             component->key_left_pressed(component, g);
                             break;
                         case SDLK_RIGHT:
                             /*
                              *dbg_str(DBG_DETAIL,"SDLK_RIGHT, code :%x",e->key.keysym.sym);
                              */
                             component->key_right_pressed(component, g);
                             break;
                         case SDLK_PAGEUP:
                             component->key_pageup_pressed(component, g);
                             break;
                         case SDLK_PAGEDOWN:
                             component->key_pagedown_pressed(component, g);
                             break;
                         case SDLK_BACKSPACE:
                             /*
                              *dbg_str(DBG_DETAIL,"BACKSPACE, code :%d",e->key.keysym.sym);
                              */
                             component->key_backspace_pressed(component, g);
                             break;
                         case SDLK_j:
                              if (SDL_GetModState() & KMOD_CTRL) {
                                  component->key_onelineup_pressed(component, g);
                              } else{
                                  dbg_str(DBG_IMPORTANT,"key j down");
                              }
                             break;
                         case SDLK_k:
                              if (SDL_GetModState() & KMOD_CTRL) {
                                  /*
                                   *dbg_str(DBG_IMPORTANT,"ctrl + k");
                                   */
                                  component->key_onelinedown_pressed(component, g);
                              }
                             break;
                         default:
                             break;
                     } 
                     break;
                 case SDL_KEYUP:
                     /*
                      *dbg_str(DBG_DETAIL,"SDL_KEYUP");
                      */
                     break;
                 case SDL_CONTROLLERBUTTONDOWN:
                     dbg_str(DBG_DETAIL,"SDL EVENT: Controller %d button %d down",
                             e->cbutton.which, e->cbutton.button);
                     break;
                 case SDL_MOUSEBUTTONDOWN:
                     event->x        = e->button.x;
                     event->y        = e->button.y;
                     event->button   = e->button.button;
                     event->clicks   = e->button.clicks;
                     event->windowid = e->button.windowID;
                     event->window   = window;

                     component->mouse_pressed(component, event, window);
                     break;
                 case SDL_MOUSEMOTION:
                     event->x        = e->motion.x;
                     event->xrel     = e->motion.xrel;
                     event->y        = e->motion.y;
                     event->yrel     = e->motion.yrel;
                     event->windowid = e->button.windowID;
                     event->window   = window;

                     component->mouse_moved(component, event, window);
                     break;
                 case SDL_MOUSEWHEEL: 
                     event->x         = e->wheel.x;
                     event->y         = e->wheel.y;
                     event->direction = e->wheel.direction;
                     event->windowid  = e->button.windowID;
                     event->window    = window;

                     component->mouse_wheel_moved(component, event, window);
                     break;
                 case SDL_TEXTEDITING:
                     print_text("EDIT", e->text.text);
                     break;
                 case SDL_TEXTINPUT:
                     event->window   = window;

                     component->key_text_pressed(component,e->text.text[0], g);
                     break;
                 case SDL_FINGERDOWN:
                 case SDL_FINGERUP:
                     dbg_str(DBG_DETAIL,
                             "SDL EVENT: Finger: %s touch=%ld, finger=%ld, x=%f, y=%f, dx=%f, dy=%f, pressure=%f",
                             (e->type == SDL_FINGERDOWN) ? "down" : "up",
                             (long) e->tfinger.touchId,
                             (long) e->tfinger.fingerId,
                             e->tfinger.x, e->tfinger.y,
                             e->tfinger.dx, e->tfinger.dy, e->tfinger.pressure);
                     break;
                 default:
                     break;
             }
         }
    }

    SDL_StopTextInput();

    return 0;
}

static class_info_entry_t sdl_event_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"__Event","event",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","poll_event",__poll_event,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Sdl_Event",sdl_event_class_info);

void test_obj_sdl_event()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Window *window;
    Graph *g;
    __Event *event;
    char *set_str;
    char buf[2048];

    set_str = (char *)gen_window_setting_str();

    /*
     *window  = OBJECT_NEW(allocator, Sdl_Window,set_str);
     */
    window  = OBJECT_NEW(allocator, Sdl_Window,set_str);
    g       = window->graph;
    event   = window->event;

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL,"Window dump: %s",buf);

    event->poll_event(event,window);

    object_destroy(window);

    free(set_str);
}


