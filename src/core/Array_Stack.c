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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Stack *stack, char *init_str)
{
    allocator_t *allocator = stack->obj.allocator;
    Array_Stack *as = (Array_Stack *)stack;

    as->core = array_stack_alloc(allocator);
    array_stack_init(as->core);

    return 1;
}

static int __deconstrcut(Stack *stack)
{
    Array_Stack *as = (Array_Stack *)stack;
    int ret;

    array_stack_destroy(as->core);

    return 1;
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
    Init_Obj___Entry(0, Stack, parent),
    Init_Nfunc_Entry(1, Array_Stack, construct, __construct),
    Init_Nfunc_Entry(2, Array_Stack, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, Array_Stack, set, NULL),
    Init_Vfunc_Entry(4, Array_Stack, get, NULL),
    Init_Vfunc_Entry(5, Array_Stack, push, __push),
    Init_Vfunc_Entry(6, Array_Stack, pop, __pop),
    Init_Vfunc_Entry(7, Array_Stack, count, __count),
    Init_End___Entry(8, Array_Stack),
};
REGISTER_CLASS("Array_Stack", array_stack_class_info);
