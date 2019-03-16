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
#include <libobject/event/event_base.h>
#include <libobject/core/stack.h>

static int __construct(Stack *stack, char *init_str)
{
    allocator_t *allocator = stack->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL, "stack construct, stack addr:%p", stack);

    return 0;
}

static int __deconstrcut(Stack *stack)
{
    dbg_str(DBG_DETAIL, "stack deconstruct, stack addr:%p", stack);
    int ret;
    void *tret;

    return 0;
}

static class_info_entry_t stack_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", NULL, sizeof(void *), offset_of_class(Stack, set)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", NULL, sizeof(void *), offset_of_class(Stack, get)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *), offset_of_class(Stack, construct)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *), offset_of_class(Stack, deconstruct)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "push", NULL, sizeof(void *), offset_of_class(Stack, push)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "pop", NULL, sizeof(void *), offset_of_class(Stack, pop)}, 
    [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "count", NULL, sizeof(void *), offset_of_class(Stack, count)}, 
    [8] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Stack", stack_class_info);
