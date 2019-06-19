/**
 * @file Array_Stack_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/Array_Stack_Test.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Test *test, char *init_str)
{
    dbg_str(DBG_DETAIL,"Array_Stack_Test construct");
    return 0;
}

static int __deconstrcut(Test *test)
{
    dbg_str(DBG_DETAIL,"Array_Stack_Test deconstruct");
    return 0;
}

static int __setup(Array_Stack_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;
    Stack *stack;

    dbg_str(DBG_DETAIL,"Array_Stack_Test set up");
    stack = object_new(allocator, "Array_Stack", NULL);
    test->stack = stack;

    return 0;
}

static int __teardown(Array_Stack_Test *test)
{
    dbg_str(DBG_DETAIL,"Array_Stack_Test teardown");
    object_destroy(test->stack);

    return 0;
}

static int __test_push(Array_Stack_Test *test)
{
    Stack *stack = (Stack *)test->stack;
    Test *t = (Test *)test;

    dbg_str(DBG_DETAIL,"Array_Stack_Test test_push");

    Init_Test_Case(test);

    stack->push(stack, (void *)4);
    stack->push(stack, (void *)5);
    stack->push(stack, (void *)6);
    stack->push(stack, (void *)7);

    if (stack->count(stack) != 4) {
        return -1;
    }

    return 1;
}

static int __test_pop(Array_Stack_Test *test)
{
    Stack *stack = (Stack *)test->stack;
    void *p;
    Test *t = (Test *)test;

    dbg_str(DBG_DETAIL,"Array_Stack_Test test_pop");

    Init_Test_Case(test);

    stack->push(stack, (void *)4);
    stack->push(stack, (void *)5);
    stack->push(stack, (void *)6);
    stack->push(stack, (void *)7);

    if (stack->count(stack) != 4) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if ((int)p != 7) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if ((int)p != 6) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if ((int)p != 5) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if ((int)p != 4) {
        return -1;
    }

    return 1;
}

static int __test_count(Array_Stack_Test *test) 
{
    Stack *stack = (Stack *)test->stack;
    void *p;
    Test *t = (Test *)test;

    dbg_str(DBG_DETAIL,"Array_Stack_Test test_count");

    Init_Test_Case(test);

    stack->push(stack, (void *)4);

    if (stack->count(stack) != 1) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if (stack->count(stack) != 0) {
        return -1;
    }

    return 1;
}

static class_info_entry_t array_stack_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Array_Stack_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Array_Stack_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Array_Stack_Test, set, NULL),
    Init_Vfunc_Entry(4 , Array_Stack_Test, get, NULL),
    Init_Vfunc_Entry(5 , Array_Stack_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Array_Stack_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Array_Stack_Test, test_push, __test_push),
    Init_Vfunc_Entry(8 , Array_Stack_Test, test_pop, __test_pop),
    Init_Vfunc_Entry(9 , Array_Stack_Test, test_count, __test_count),
    Init_End___Entry(10, Array_Stack_Test),
};
REGISTER_CLASS("Array_Stack_Test", array_stack_test_class_info);
