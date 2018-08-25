/**
 * @file reg_heap.c
 * @Synopsis  
 * @author alan lin
 * @version 1.0.0
 * @date 2018-08-23
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
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/registry/reg_heap.h>

reg_heap_t *global_ctors_reg_heap = NULL;

static int greater_than(void *element1, void *element2) 
{
    init_func_entry_t *e1, *e2;

    e1 = (init_func_entry_t *) element1;
    e2 = (init_func_entry_t *) element2;

    return (e1->level > e2->level) ? 1 : 0;
}

reg_heap_t * get_global_ctors_reg_heap()
{
    reg_heap_t * reg_heap = global_ctors_reg_heap;

    if (reg_heap == NULL) {
        int capacity = 1024;
        reg_heap = reg_heap_alloc();
        reg_heap_set(reg_heap, "comparator", (void *)greater_than);
        reg_heap_set(reg_heap, "capacity", (void *) &capacity);
        reg_heap_init(reg_heap);
        global_ctors_reg_heap = reg_heap;
    } 

    return global_ctors_reg_heap;
}

int __register_ctor_func(int level, int (*func)()) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_ctors_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func = func;
    element->args_count = 0;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}

int __register_ctor_func1(int level, int (*func)(void *arg), void *arg) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_ctors_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func1 = func;
    element->args_count = 1;
    element->arg1 = arg;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}

int __register_ctor_func2(int level, int (*func)(void *arg1, void *arg2), 
        void *arg1, void *arg2) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_ctors_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func2 = func;
    element->args_count = 2;
    element->arg1 = arg1;
    element->arg2 = arg2;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}

int __register_ctor_func3(int level, int (*func)(void *arg1, void *arg2, void *arg3), 
        void *arg1, void *arg2, void *arg3) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_ctors_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func3 = func;
    element->args_count = 3;
    element->arg1 = arg1;
    element->arg2 = arg2;
    element->arg3 = arg3;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}

int execute_ctor_funcs() 
{
    int i, size = 0;
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_ctors_reg_heap();

    size = reg_heap_size(reg_heap);
    for(i=0; i< size; i++){
        reg_heap_remove(reg_heap, (void **)&element);

        /*
         *printf("%d ", element->level);
         */

        if (element->args_count == 0) {
            element->func();
            free(element);
        } else if (element->args_count == 1) {
            element->func1(element->arg1);
            free(element);
        } else if (element->args_count == 2) {
            element->func2(element->arg1, element->arg2);
            free(element);
        }else if (element->args_count == 3) {
            element->func3(element->arg1, element->arg2, element->arg3);
            free(element);
        } else {
        }
    }

    reg_heap_destroy(reg_heap);

    return 0;
}
