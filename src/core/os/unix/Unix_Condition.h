#ifndef __UNIX_CONDITION_H__
#define __UNIX_CONDITION_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Lock.h>
#include <libobject/core/Condition.h>

typedef struct unix_condition_s Unix_Condition;

struct unix_condition_s{
    Condition parent;

    int (*construct)(Unix_Condition *,char *init_str);
    int (*deconstruct)(Unix_Condition *);
    int (*set)(Unix_Condition *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setlock)(Unix_Condition *cond, Lock *lock);
    int (*wait)(Unix_Condition *cond);
    int (*signal)(Unix_Condition *cond);
    int (*broadcast)(Unix_Condition *cond);

    pthread_mutex_t mutex;
    pthread_condattr_t attr;
    pthread_cond_t cond;
};

#endif
