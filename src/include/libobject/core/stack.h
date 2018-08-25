#ifndef __STACK_H__
#define __STACK_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct stack_s Stack;

struct stack_s{
	Obj obj;

	int (*construct)(Stack *,char *init_str);
	int (*deconstruct)(Stack *);
	int (*set)(Stack *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*push)(Stack *stack, void *element);
    int (*pop)(Stack *stack, void **element);
};

#endif
