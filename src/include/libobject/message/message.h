#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct message_s Message;

struct message_s{
	Obj obj;

	int (*construct)(Message *,char *init_str);
	int (*deconstruct)(Message *);
	int (*set)(Message *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
