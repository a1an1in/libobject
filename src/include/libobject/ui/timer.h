#ifndef ____TIMER_H__
#define ____TIMER_H__

#include <libobject/obj.h>

typedef struct timer_s __Timer;
struct timer_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(__Timer *timer,char *init_str);
	int (*deconstruct)(__Timer *timer);
	int (*set)(__Timer *timer, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
	int (*set_timer)(__Timer *timer,uint32_t time, void *callback);
	int (*reuse)(__Timer *timer);

	/*attribs*/
	void *opaque;
};

#endif
