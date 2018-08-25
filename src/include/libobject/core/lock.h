#ifndef __LOCK_H__
#define __LOCK_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct lock_s Lock;

struct lock_s{
	Obj obj;

	int (*construct)(Lock *,char *init_str);
	int (*deconstruct)(Lock *);
	int (*set)(Lock *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*lock)(Lock *lock);
    int (*trylock)(Lock *lock);
    int (*unlock)(Lock *lock);
};

#endif
