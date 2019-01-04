#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/mutex_lock.h>

typedef struct condition_s Condition;

struct condition_s{
	Obj obj;

	int (*construct)(Condition *,char *init_str);
	int (*deconstruct)(Condition *);
	int (*set)(Condition *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*setlock)(Condition *condition, Mutex_Lock *lock);
    int (*wait)(Condition *condition);
    int (*signal)(Condition *condition);
    int (*broadcast)(Condition *condition);

    Mutex_Lock *       lock;
    pthread_condattr_t attr;
    pthread_cond_t     cond;
};

#endif
