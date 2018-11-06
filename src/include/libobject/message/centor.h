#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>
#include <libobject/concurrent/worker.h>

typedef struct centor_s Centor;

struct centor_s{
	Obj obj;

	int (*construct)(Centor *,char *init_str);
	int (*deconstruct)(Centor *);
	int (*set)(Centor *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

    Socket *s, *c;
    Worker *worker;

};

#endif
