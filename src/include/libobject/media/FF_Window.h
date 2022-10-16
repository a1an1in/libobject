#ifndef __GOYA_H__
#define __GOYA_H__

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/message/Publisher.h> 
#include <libobject/message/Centor.h>
#include <libobject/message/Subscriber.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/ui/Window.h>
#include <libobject/ui/Audio.h>
#include <libobject/core/Lock.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/media/Player.h>

typedef struct ff_window_s FF_Window;

struct ff_window_s{
    Sdl_Window parent;

    int (*construct)(FF_Window *,char *init_str);
    int (*deconstruct)(FF_Window *);
    int (*set)(FF_Window *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/

    /*play control interface*/
    int (*init)(FF_Window *);
    int (*start)(FF_Window *);
    int (*stop)(FF_Window *);
    int (*exit)(FF_Window *);
    int (*seek_to)(FF_Window *, int position);
    int (*seek_to_specified_pos)(FF_Window *,int64_t position);
    int (*get_duration)(FF_Window *);
    int (*get_position)(FF_Window *);

    int (*run)(FF_Window *goya);
    int (*poll_event)(FF_Window *goya);
    void (*create_event)(FF_Window *window);
    void (*destroy_event)(FF_Window *window);

    int (*on_key_text_pressed)(Component *component,char c, void *graph);
    int (*on_key_backspace_pressed)(Component *component,void *graph);
    int (*on_key_up_pressed)(Component *component,void *graph);
    int (*on_key_down_pressed)(Component *component,void *graph);
    int (*on_key_left_pressed)(Component *component,void *graph);
    int (*on_key_right_pressed)(Component *component,void *graph);
    int (*on_key_pageup_pressed)(Component *component,void *graph);
    int (*on_key_pagedown_pressed)(Component *component,void *graph);
    int (*on_key_onelineup_pressed)(Component *component,void *graph);
    int (*on_key_onelinedown_pressed)(Component *component,void *graph);
    int (*on_key_esc_pressed)(Component *component,void *graph);

    void (*on_mouse_pressed)(Component *component,void *event, void *window);
    void (*on_mouse_released)(Component *component,void *event, void *window);
    void (*on_mouse_entered)(Component *component,void *event, void *window);
    void (*on_mouse_exited)(Component *component,void *event, void *window);
    void (*on_mouse_moved)(Component *component,void *event, void *window);
    void (*on_mouse_dragged)(Component *component,void *event, void *window);
    void (*on_mouse_wheel_moved)(Component *component,void *event, void *window);
    int  (*is_mouse_over_component)(Component *component,void *event);

    void (*on_window_moved)(Component *component,void *event, void *window);
    void (*on_window_resized)(Component *component,void *event, void *window);

    void (*add_player)(FF_Window *window, void *player);

    int (*add_component)(Container *obj, void *pos, void *component);
    int (*update_window)(Window *window);

    Player *player;
};

#endif


