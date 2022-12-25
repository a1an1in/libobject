#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/concurrent/event/event.h>
#include <libobject/concurrent/event/signal.h>
#include <libobject/concurrent/event/Rbtree_Timer.h>
#include <libobject/core/utils/miscellany/buffer.h>

typedef struct concurrent_s Concurrent;

struct concurrent_s{
	Obj obj;

	int (*construct)(Concurrent *,char *init_str);
	int (*deconstruct)(Concurrent *);
	int (*set)(Concurrent *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

int libobject_init_concurrent();
int libobject_destroy_concurrent();

#endif
