#ifndef __SDL_UI_EVENT_H__
#define __SDL_UI_EVENT_H__

#include <libobject/obj.h>
#include <libobject/string.h>
#include <libobject/ui/event.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_events.h>

typedef struct ui_sdl_event_s Sdl_Event;

struct ui_sdl_event_s{
	__Event event;

	/*normal methods*/
	int (*construct)(__Event *event,char *init_str);
	int (*deconstruct)(__Event *event);
	int (*set)(__Event *event, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
    int (*poll_event)(__Event *event,void *window);

	/*attribs*/
	SDL_Event ev;
};

#endif
