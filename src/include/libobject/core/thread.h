#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
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
    int (*set_start_arg)(Thread *, void *);
    int (*set_opaque)(Thread *, void *);
    int (*get_status)(Thread *);
    int (*stop)(Thread *);
    pthread_t (*get_tid)(Thread *);
    void (*join)(Thread *,Thread *);
    void (*detach)(Thread *);

    int (*set_run_routine)(Thread *,void *);
	/*virtual methods reimplement*/
    void *(*start_routine)(void *);
    void *(*execute)(void *);
    void *(*run)(Thread *);



    void *arg;
    pthread_t tid;
    void *opaque;
    int is_run;
    int joinable;
};

#endif
