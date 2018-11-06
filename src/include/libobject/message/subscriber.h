#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct subscriber_s Subscriber;

struct subscriber_s{
	Obj obj;

	int (*construct)(Subscriber *,char *init_str);
	int (*deconstruct)(Subscriber *);
	int (*set)(Subscriber *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
