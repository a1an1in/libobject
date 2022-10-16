#ifndef __FF_EVENT_H__
#define __FF_EVENT_H__

#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/ui/Event.h>
#include <libobject/ui/Sdl_Event.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct ff_event_s FF_Event;

struct ff_event_s{
    Sdl_Event parent;

    /*normal methods*/
    int (*construct)(FF_Event *event,char *init_str);
    int (*deconstruct)(FF_Event *event);
    int (*set)(FF_Event *event, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
    int (*poll_event)(FF_Event *event,void *window);
};

#endif
