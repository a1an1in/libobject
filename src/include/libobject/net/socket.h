#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct socket_s Socket;

struct socket_s{
	Obj obj;

	int (*construct)(Socket *,char *init_str);
	int (*deconstruct)(Socket *);
	int (*set)(Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
