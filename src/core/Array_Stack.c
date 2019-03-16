/**
 * @file Stack.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-10-11
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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/array_stack.h>
#include <libobject/event/event_base.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Stack *stack, char *init_str)
{
    allocator_t *allocator = stack->obj.allocator;
    Array_Stack *as = (Array_Stack *)stack;

    as->core = array_stack_alloc(allocator);
    array_stack_init(as->core);

    dbg_str(DBG_DETAIL, "stack construct, stack addr:%p", stack);

    return 0;
}

static int __deconstrcut(Stack *stack)
{
    Array_Stack *as = (Array_Stack *)stack;
    int ret;

    dbg_str(DBG_DETAIL, "stack deconstruct, stack addr:%p", stack);
    array_stack_destroy(as->core);

    return 0;
}

static int __push(Stack *stack, void *element)
{
    Array_Stack *as = (Array_Stack *)stack;

    return array_stack_push(as->core, element);
}

static int __pop(Stack *stack, void **element)
{
    Array_Stack *as = (Array_Stack *)stack;

    return array_stack_pop(as->core, element);
}

static int __count(Stack *stack) 
{
    Array_Stack *as = (Array_Stack *)stack;

    return as->core->count;
}

static class_info_entry_t array_stack_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ, "Stack", "parent", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *), offset_of_class(Array_Stack, construct)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *), offset_of_class(Array_Stack, deconstruct)}, 
    [3] = {ENTRY_TYPE_VFUNC_POINTER, "", "push", __push, sizeof(void *), offset_of_class(Array_Stack, push)}, 
    [4] = {ENTRY_TYPE_VFUNC_POINTER, "", "pop", __pop, sizeof(void *), offset_of_class(Array_Stack, pop)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "count", __count, sizeof(void *), offset_of_class(Stack, count)}, 
    [6] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Array_Stack", array_stack_class_info);

int test_Array_Stack(TEST_ENTRY *entry)
{
    Stack *stack;
    allocator_t *allocator = allocator_get_default_alloc();
    void *a, *p;

    stack  = OBJECT_NEW(allocator, Array_Stack, NULL);

    dbg_str(DBG_DETAIL, "push data:%d", a = (void *)4 );
    stack->push(stack, a);
    dbg_str(DBG_DETAIL, "push data:%d", a = (void *)5 );
    stack->push(stack, a);
    dbg_str(DBG_DETAIL, "push data:%d", a = (void *)6 );
    stack->push(stack, a);
    dbg_str(DBG_DETAIL, "push data:%d", a = (void *)7);
    stack->push(stack, a);


    if (stack->count(stack) != 4) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if (p != 7) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if (p != 6) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if (p != 5) {
        return -1;
    }

    stack->pop(stack, (void **)&p);
    if (p != 4) {
        return -1;
    }

    object_destroy(stack);

    return 1;
}
REGISTER_TEST_FUNC(test_Array_Stack);
