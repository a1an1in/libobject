#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct thread_s Thread;

struct thread_s{
	Obj obj;

	int (*construct)(Thread *,char *init_str);
	int (*deconstruct)(Thread *);
	int (*set)(Thread *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*start)(Thread *);
    int (*set_start_routine)(Thread *, void *);

	/*virtual methods reimplement*/
    void *(*start_routine)(void *);
    void *arg;
    pthread_t tid;
};

#endif
