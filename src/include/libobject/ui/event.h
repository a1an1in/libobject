#ifndef __UI_EVENT_H__
#define __UI_EVENT_H__

#include <libobject/obj.h>
#include <libobject/string.h>
#include <libobject/ui/graph.h>

typedef struct ui_event_s __Event;
struct ui_event_s{
	Obj obj;

	/*normal methods*/
	int (*construct)(__Event *event,char *init_str);
	int (*deconstruct)(__Event *event);
	int (*set)(__Event *event, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
    int (*poll_event)(__Event *event,void *window);

	/*attribs*/
    int x, xrel;
    int y, yrel;
    int button;
    int clicks;
    int windowid;
    int data1, data2;
    int direction;
    void *window;
};

#endif
