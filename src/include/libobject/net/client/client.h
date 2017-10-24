#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct client_s Client;

struct client_s{
	Obj obj;

	int (*construct)(Client *,char *init_str);
	int (*deconstruct)(Client *);
	int (*set)(Client *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
