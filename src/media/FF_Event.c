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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Sdl_Event.h>
#include <libobject/ui/Sdl_Window.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <libobject/media/FF_Event.h>
#include <libobject/media/FF_Window.h>
#include <libobject/media/Player.h>


static const SDL_Keycode SDL_default_keymap[SDL_NUM_SCANCODES] = {
    0, 0, 0, 0,
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_SPACE,
    '-',
    '=',
    '[',
    ']',
    '\\',
    '#',
    ';',
    '\'',
    '`',
    ',',
    '.',
    '/',
    SDLK_CAPSLOCK,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PRINTSCREEN,
    SDLK_SCROLLLOCK,
    SDLK_PAUSE,
    SDLK_INSERT,
    SDLK_HOME,
    SDLK_PAGEUP,
    SDLK_DELETE,
    SDLK_END,
    SDLK_PAGEDOWN,
    SDLK_RIGHT,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_NUMLOCKCLEAR,
    SDLK_KP_DIVIDE,
    SDLK_KP_MULTIPLY,
    SDLK_KP_MINUS,
    SDLK_KP_PLUS,
    SDLK_KP_ENTER,
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_0,
    SDLK_KP_PERIOD,
    0,
    SDLK_APPLICATION,
    SDLK_POWER,
    SDLK_KP_EQUALS,
    SDLK_F13,
    SDLK_F14,
    SDLK_F15,
    SDLK_F16,
    SDLK_F17,
    SDLK_F18,
    SDLK_F19,
    SDLK_F20,
    SDLK_F21,
    SDLK_F22,
    SDLK_F23,
    SDLK_F24,
    SDLK_EXECUTE,
    SDLK_HELP,
    SDLK_MENU,
    SDLK_SELECT,
    SDLK_STOP,
    SDLK_AGAIN,
    SDLK_UNDO,
    SDLK_CUT,
    SDLK_COPY,
    SDLK_PASTE,
    SDLK_FIND,
    SDLK_MUTE,
    SDLK_VOLUMEUP,
    SDLK_VOLUMEDOWN,
    0, 0, 0,
    SDLK_KP_COMMA,
    SDLK_KP_EQUALSAS400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_ALTERASE,
    SDLK_SYSREQ,
    SDLK_CANCEL,
    SDLK_CLEAR,
    SDLK_PRIOR,
    SDLK_RETURN2,
    SDLK_SEPARATOR,
    SDLK_OUT,
    SDLK_OPER,
    SDLK_CLEARAGAIN,
    SDLK_CRSEL,
    SDLK_EXSEL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_KP_00,
    SDLK_KP_000,
    SDLK_THOUSANDSSEPARATOR,
    SDLK_DECIMALSEPARATOR,
    SDLK_CURRENCYUNIT,
    SDLK_CURRENCYSUBUNIT,
    SDLK_KP_LEFTPAREN,
    SDLK_KP_RIGHTPAREN,
    SDLK_KP_LEFTBRACE,
    SDLK_KP_RIGHTBRACE,
    SDLK_KP_TAB,
    SDLK_KP_BACKSPACE,
    SDLK_KP_A,
    SDLK_KP_B,
    SDLK_KP_C,
    SDLK_KP_D,
    SDLK_KP_E,
    SDLK_KP_F,
    SDLK_KP_XOR,
    SDLK_KP_POWER,
    SDLK_KP_PERCENT,
    SDLK_KP_LESS,
    SDLK_KP_GREATER,
    SDLK_KP_AMPERSAND,
    SDLK_KP_DBLAMPERSAND,
    SDLK_KP_VERTICALBAR,
    SDLK_KP_DBLVERTICALBAR,
    SDLK_KP_COLON,
    SDLK_KP_HASH,
    SDLK_KP_SPACE,
    SDLK_KP_AT,
    SDLK_KP_EXCLAM,
    SDLK_KP_MEMSTORE,
    SDLK_KP_MEMRECALL,
    SDLK_KP_MEMCLEAR,
    SDLK_KP_MEMADD,
    SDLK_KP_MEMSUBTRACT,
    SDLK_KP_MEMMULTIPLY,
    SDLK_KP_MEMDIVIDE,
    SDLK_KP_PLUSMINUS,
    SDLK_KP_CLEAR,
    SDLK_KP_CLEARENTRY,
    SDLK_KP_BINARY,
    SDLK_KP_OCTAL,
    SDLK_KP_DECIMAL,
    SDLK_KP_HEXADECIMAL,
    0, 0,
    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT,
    SDLK_RGUI,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_MODE,
    SDLK_AUDIONEXT,
    SDLK_AUDIOPREV,
    SDLK_AUDIOSTOP,
    SDLK_AUDIOPLAY,
    SDLK_AUDIOMUTE,
    SDLK_MEDIASELECT,
    SDLK_WWW,
    SDLK_MAIL,
    SDLK_CALCULATOR,
    SDLK_COMPUTER,
    SDLK_AC_SEARCH,
    SDLK_AC_HOME,
    SDLK_AC_BACK,
    SDLK_AC_FORWARD,
    SDLK_AC_STOP,
    SDLK_AC_REFRESH,
    SDLK_AC_BOOKMARKS,
    SDLK_BRIGHTNESSDOWN,
    SDLK_BRIGHTNESSUP,
    SDLK_DISPLAYSWITCH,
    SDLK_KBDILLUMTOGGLE,
    SDLK_KBDILLUMDOWN,
    SDLK_KBDILLUMUP,
    SDLK_EJECT,
    SDLK_SLEEP,
    SDLK_APP1,
    SDLK_APP2,
    SDLK_AUDIOREWIND,
    SDLK_AUDIOFASTFORWARD,
};

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
    dbg_str(DBG_DETAIL, "Initial state:%s", message);
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
    dbg_str(DBG_DETAIL, "%s", message);
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

static int __construct(FF_Event *event, char *init_str)
{
    dbg_str(OBJ_DETAIL, "event construct, event addr:%p", event);

    return 0;
}

static int __deconstrcut(FF_Event *event)
{
    FF_Event *i = (FF_Event *)event;
    dbg_str(OBJ_DETAIL, "event deconstruct, event addr:%p", event);


    return 0;
}

static int __set(FF_Event *event, char *attrib, void *value)
{
    FF_Event *e = (FF_Event *)event;

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
        dbg_str(OBJ_WARNNING, "event set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(FF_Event *event, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING, "event get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}


int __send_key_pressed_event(Window *window, SDL_Scancode scancode)
{
    int ret;
    SDL_Event event;
    SDL_Keycode keycode;
    Sdl_Window *w = (Sdl_Window *)window;

    keycode = SDL_default_keymap[scancode];

    event.key.type = SDL_KEYDOWN;
    event.key.state = SDL_PRESSED;
    event.key.keysym.scancode = scancode;
    event.key.keysym.sym = keycode;
    event.key.windowID = SDL_GetWindowID(w->sdl_window);
    ret = (SDL_PushEvent(&event) > 0);

    return ret;
}

static int __poll_event(__Event *event, void *window)
{
    int quit             = 0;
    SDL_Event *e         = &((FF_Event *)event)->parent.ev;
    Window *w            = (Window *)window;
    FF_Window *ff_window = (FF_Window *)window;
    Render *g            = w->render;
    Component *component = (Component *)window, *cur;
    Player *player       = ff_window->player;
    static int count = 0;
     
    dbg_str(DBG_DETAIL, "sdl event poll");

    SDL_StartTextInput();

    while(!quit) {
        if (player->is_state(player, STATE_EXIT)) {
            quit = 1;
        } else if (player->is_state(player, STATE_STOPPED)) {
            dbg_str(DBG_DETAIL, "poll event, player is stoped");
            usleep(1000000);
        } else if (player->is_state(player, STATE_LOADING)) {
            dbg_str(DBG_DETAIL, "poll event, player is loading");
            usleep(1000000);
        } else {
#if 1
            player->draw_image(player);
#else

            int data[1024 * 10 * 10];
            int len = 0;
            int ret = 0;

            ret = player->get_rgb(player,
                    AV_PIX_FMT_RGB24,
                    200, 100,
                    data, &len);
            if (ret < 0) {
                dbg_str(DBG_SUC,"get_rgb_from_current_frame err");
            } else {
                dbg_str(DBG_SUC,"total len=%d, linesize=%d", ret, len);
                dbg_buf(DBG_SUC,"rbg data:", data, 10);
            }
#endif
        }
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
                             component->on_window_moved(component, event, window);
                             break;
                         case SDL_WINDOWEVENT_RESIZED:
                             event->data1    = e->window.data1;
                             event->data2    = e->window.data2;
                             event->windowid = e->button.windowID;
                             event->window   = window;
                             component->on_window_resized(component, event, window);
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
                         case SDLK_ESCAPE:
                             component->on_key_esc_pressed(component, g);
                             break;
                         case SDLK_UP:
                             /*
                              *dbg_str(DBG_DETAIL, "SDLK_UP, code :%x", e->key.keysym.sym);
                              */
                             component->on_key_up_pressed(component, g);
                             break;
                         case SDLK_DOWN:
                             /*
                              *dbg_str(DBG_DETAIL, "SDLK_DOWN, code :%x", e->key.keysym.sym);
                              */
                             component->on_key_down_pressed(component, g);
                             break;
                         case SDLK_LEFT:
                             /*
                              *dbg_str(DBG_DETAIL, "SDLK_LEFT, code :%x", e->key.keysym.sym);
                              */
                             component->on_key_left_pressed(component, g);
                             break;
                         case SDLK_RIGHT:
                             /*
                              *dbg_str(DBG_DETAIL, "SDLK_RIGHT, code :%x", e->key.keysym.sym);
                              */
                             component->on_key_right_pressed(component, g);
                             break;
                         case SDLK_PAGEUP:
                             component->on_key_pageup_pressed(component, g);
                             break;
                         case SDLK_PAGEDOWN:
                             component->on_key_pagedown_pressed(component, g);
                             break;
                         case SDLK_BACKSPACE:
                             /*
                              *dbg_str(DBG_DETAIL, "BACKSPACE, code :%d", e->key.keysym.sym);
                              */
                             component->on_key_backspace_pressed(component, g);
                             break;
                         case SDLK_j:
                              if (SDL_GetModState() & KMOD_CTRL) {
                                  component->on_key_onelineup_pressed(component, g);
                              } else{
                                  dbg_str(DBG_IMPORTANT, "key j down");
                              }
                             break;
                         case SDLK_k:
                              if (SDL_GetModState() & KMOD_CTRL) {
                                  /*
                                   *dbg_str(DBG_IMPORTANT, "ctrl + k");
                                   */
                                  component->on_key_onelinedown_pressed(component, g);
                              }
                             break;
                         default:
                             break;
                     } 
                     break;
                 case SDL_KEYUP:
                     /*
                      *dbg_str(DBG_DETAIL, "SDL_KEYUP");
                      */
                     break;
                 case SDL_CONTROLLERBUTTONDOWN:
                     dbg_str(DBG_DETAIL, "SDL EVENT: Controller %d button %d down", 
                             e->cbutton.which, e->cbutton.button);
                     break;
                 case SDL_MOUSEBUTTONDOWN:
                     event->x        = e->button.x;
                     event->y        = e->button.y;
                     event->button   = e->button.button;
                     event->clicks   = e->button.clicks;
                     event->windowid = e->button.windowID;
                     event->window   = window;

                     component->on_mouse_pressed(component, event, window);
                     break;
                 case SDL_MOUSEMOTION:
                     event->x        = e->motion.x;
                     event->xrel     = e->motion.xrel;
                     event->y        = e->motion.y;
                     event->yrel     = e->motion.yrel;
                     event->windowid = e->button.windowID;
                     event->window   = window;

                     component->on_mouse_moved(component, event, window);
                     break;
                 case SDL_MOUSEWHEEL: 
                     event->x         = e->wheel.x;
                     event->y         = e->wheel.y;
                     event->direction = e->wheel.direction;
                     event->windowid  = e->button.windowID;
                     event->window    = window;

                     component->on_mouse_wheel_moved(component, event, window);
                     break;
                 case SDL_TEXTEDITING:
                     print_text("EDIT", e->text.text);
                     break;
                 case SDL_TEXTINPUT:
                     event->window   = window;

                     component->on_key_text_pressed(component, e->text.text[0], g);
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
    [0 ] = {ENTRY_TYPE_OBJ, "Sdl_Event", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "poll_event", __poll_event, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("FF_Event", sdl_event_class_info);
extern char *gen_window_setting_str();
static int sdl_event()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Window *window;
    char *set_str;
    char buf[2048];

    set_str = (char *)gen_window_setting_str();

    /*
     *window  = OBJECT_NEW(allocator, Sdl_Window, set_str);
     */
    window  = OBJECT_NEW(allocator, Sdl_Window, set_str);

    object_dump(window, "Sdl_Window", buf, 2048);
    dbg_str(DBG_DETAIL, "Window dump: %s", buf);

    window->set_window_title(window, "hello world");

    window->poll_event(window);

    object_destroy(window);


    free(set_str);
}
REGISTER_TEST_CMD(sdl_event);


