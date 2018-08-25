#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/hash_map.h>
#include <libobject/event/event.h>
#include <libobject/event/signal.h>
#include <libobject/event/rbtree_timer.h>
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

#endif
