#ifndef __OBJ_ARRAY_STACK_H__
#define __OBJ_ARRAY_STACK_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/stack.h>

typedef struct o_array_stack_s Array_Stack;

struct o_array_stack_s{
	Stack parent;

	int (*construct)(Stack *,char *init_str);
	int (*deconstruct)(Stack *);
	int (*set)(Stack *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*push)(Stack *stack, void *element);
    int (*pop)(Stack *stack, void **element);

    array_stack_t *core;
};

#endif
