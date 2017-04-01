#ifndef __SDL_TIMER_H__
#define __SDL_TIMER_H__

#include <libobject/obj.h>
#include <libobject/ui/timer.h>
#include <SDL2/SDL.h>

typedef struct sdl_timer_s Sdl_Timer;
struct sdl_timer_s{
	__Timer timer;

	/*normal methods*/
	int (*construct)(__Timer *timer,char *init_str);
	int (*deconstruct)(__Timer *timer);
	int (*set)(__Timer *timer, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*set_timer)(__Timer *timer,uint32_t time, void *callback);
	int (*reuse)(__Timer *timer);

	uint32_t interval;
	SDL_TimerID timer_id;
	uint32_t (*callback)(uint32_t interval, void* param);
};

#endif
