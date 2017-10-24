#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct server_s Server;

struct server_s{
	Obj obj;

	int (*construct)(Server *,char *init_str);
	int (*deconstruct)(Server *);
	int (*set)(Server *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
