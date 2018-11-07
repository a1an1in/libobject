#ifndef __MUTEX_LOCK_H__
#define __MUTEX_LOCK_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/lock.h>
#include <pthread.h>

typedef struct mutex_lock_s Mutex_Lock;

struct mutex_lock_s{
    Lock parent;

	int (*construct)(Mutex_Lock *,char *init_str);
	int (*deconstruct)(Mutex_Lock *);
	int (*set)(Mutex_Lock *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*lock)(Mutex_Lock *lock);
    int (*trylock)(Mutex_Lock *lock);
    int (*unlock)(Mutex_Lock *lock);

    pthread_mutex_t mutex;
};

#endif
