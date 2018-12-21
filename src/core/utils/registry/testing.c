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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/dbg/debug.h>

reg_heap_t *global_testfunc_reg_heap = NULL;

static int greater_than(void *element1, void *element2) 
{
    init_func_entry_t *e1, *e2;

    e1 = (init_func_entry_t *) element1;
    e2 = (init_func_entry_t *) element2;

    return (strcmp(e1->func_name, e2->func_name) >= 0) ? 1 : 0;
}

reg_heap_t * get_global_testfunc_reg_heap()
{
    reg_heap_t * reg_heap = global_testfunc_reg_heap;

    if (reg_heap == NULL) {
        int capacity = 2048;
        reg_heap = reg_heap_alloc();
        reg_heap_set(reg_heap, "comparator", (void *)greater_than);
        reg_heap_set(reg_heap, "capacity", (void *) &capacity);
        reg_heap_init(reg_heap);
        global_testfunc_reg_heap = reg_heap;
    } 

    return global_testfunc_reg_heap;
}

int 
__register_test_func(int (*func)(void *),
                     const char *func_name,
                     const char *file,
                     int line) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->func1 = func;
    element->args_count = 1;
    element->func_name = func_name;
    element->file = file;
    element->line = line;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}

int 
__register_standalone_test_func(int (*func)(void *, void *, void *),
                                const char *func_name,
                                const char *file,
                                int line) 
{
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->func3 = func;
    element->args_count = 3;
    element->func_name = func_name;
    element->file = file;
    element->line = line;
    element->type = FUNC_ENTRY_TYPE_STANDALONE;

    reg_heap_add(reg_heap, (void *)element);

    return 0;
}


/**
 * @Synopsis  
 *
 * @Returns   
 *
 * note: this func can run dbg_str(), for it runs after main()
 */
int execute_test_funcs() 
{
    int i, size = 0, ret;
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();

    size = reg_heap_size(reg_heap);
    for(i=0; i< size; i++){
        reg_heap_remove(reg_heap, (void **)&element);

        if (element->type == FUNC_ENTRY_TYPE_STANDALONE) {
            continue;
        }
        if (element->args_count == 1) {
            ret = element->func1((void *)element);
            if (ret == 0) {
                dbg_str(DBG_ERROR, 
                        "test failed, func_name = %s,  file = %s, line = %d", 
                        element->func_name,
                        element->file,
                        element->line);
            } else {
                dbg_str(DBG_SUC, 
                        "test suc, func_name = %s,  file = %s, line = %d", 
                        element->func_name,
                        element->file,
                        element->line);
            }
            free(element);
        } else {
        }
    }

    reg_heap_destroy(reg_heap);

    return 0;
}

int execute_test_designated_func(char *func_name, void *arg1, void *arg2) 
{
    int i, size = 0, ret;
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();
    int flag = 0;

    size = reg_heap_size(reg_heap);
    for(i=0; i< size; i++){
        reg_heap_remove(reg_heap, (void **)&element);

        if (strcmp(element->func_name, func_name) != 0) {
            free(element);
            continue;
        }

        if (element->type != FUNC_ENTRY_TYPE_STANDALONE) {
            free(element);
            continue;
        }

        if (element->args_count == 1) {
            ret = element->func1((void *)element);
            flag = 1;
        } else if (element->args_count == 3) {
            ret = element->func3((void *)element, arg1, arg2);
            flag = 1;
        } else {
            free(element);
            continue;
        }

        if (ret == 0) {
            dbg_str(DBG_ERROR, 
                    "test failed, func_name = %s,  file = %s, line = %d", 
                    element->func_name,
                    element->file,
                    element->line);
        } else {
            dbg_str(DBG_SUC, 
                    "test suc, func_name = %s,  file = %s, line = %d", 
                    element->func_name,
                    element->file,
                    element->line);
        }
        free(element);
    }

    if (flag == 0) {
        dbg_str(DBG_ERROR, 
                "not found func_name = %s register map", 
                func_name);
    }

    reg_heap_destroy(reg_heap);

    return 0;
}

int assert_equal(void *peer1, void *peer2, unsigned int count)
{
    return memcmp(peer1, peer2, count) == 0;
}

int assert_int_equal(int peer1, int peer2)
{
    return peer1 == peer2;
}

static int add(int a, int b)
{
    return a + b;
}

static int sub(int a, int b)
{
    return a - b;
}

static int multiply(int a, int b)
{
    return a * b;
}

static int test_add(TEST_ENTRY *entry)
{
    return entry->ret = assert_int_equal(add(1, 2), 4);
}
REGISTER_TEST_FUNC(test_add);

static int test_sub(TEST_ENTRY *entry)
{
    return entry->ret = assert_int_equal(sub(2, 1), 1);
}
REGISTER_TEST_FUNC(test_sub);

static int test_multiply(TEST_ENTRY *entry)
{
    return entry->ret = assert_int_equal(multiply(2, 3), 6);
}
REGISTER_TEST_FUNC(test_multiply);
