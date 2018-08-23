/**
 * @file heap.c
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

typedef struct heap_s {
    void **queue;
    int size;
    int capacity;
    int (*comparator)(void *element1, void *element2);
} heap_t;

typedef struct init_func_entry_s {
    int level;
    int args_count;
    void *arg1, *arg2, *arg3;
    int (*func)();
    int (*func1)(void * arg);
    int (*func2)(void * arg1, void *arg2);
    int (*func3)(void * arg1, void *arg2, void *arg3);
} init_func_entry_t;

heap_t *global_heap = NULL;

static int greater_than(void *element1, void *element2) 
{
    init_func_entry_t *e1, *e2;

    e1 = (init_func_entry_t *) element1;
    e2 = (init_func_entry_t *) element2;

    return (e1->level > e2->level) ? 1 : 0;
}

static void shiftup(heap_t *heap, int index, void *e)
{
    while(index > 0) {
        int parent = (index - 1) / 2;
        if (heap->comparator(e, heap->queue[parent])) {
            break;
        }

        /*if parent smaller than inserting child, exchange position*/
        heap->queue[index] = heap->queue[parent];
        index = parent;
    }
    heap->queue[index] = e;
}

static void shiftdown(heap_t *heap, int index, void *e)
{
    int size = heap->size;
    int half = size / 2;

    while (index < half) {
        int child = 2 * index + 1;
        int right = child + 1;
        if (    right < size &&
                heap->comparator(heap->queue[child], heap->queue[right])) {
            child = right;
        }
        if (heap->comparator(heap->queue[child], e)) {
            break;
        }

        /*save greater child to parent*/
        heap->queue[index] = heap->queue[child]; 
        index = child;
    }

    heap->queue[index] = e;
}

static heap_t *heap_alloc()
{
    heap_t *ret;

    ret = (heap_t *)malloc(sizeof(heap_t));
    if (ret != NULL) {
        memset(ret, 0, sizeof(heap_t));
    }
    
    return ret;
}

static int heap_set(heap_t *heap, char *key, void *value)
{
    if (strcmp(key, "comparator") == 0) {
        heap->comparator = value;
    } else if (strcmp(key, "capacity") == 0) {
        heap->capacity = *(int *)value;
    } else {
        printf("heap_set, not support %s setting\n", key);
    }
}

static heap_t *heap_init(heap_t *heap)
{
    if (heap->capacity == 0) {
        heap->queue = (void **)malloc(sizeof(void *) * 64);
        heap->capacity = 64;
    } else {
        heap->queue = (void **)malloc(sizeof(void *) * heap->capacity);
    }

    return heap;
}

static int heap_add(heap_t *heap, void *e)
{

    int index = heap->size;

    if (heap->size == heap->capacity) {
        printf("err: heap is full");
        return -1;
    }
    shiftup(heap, index, e);
    heap->size++;

    return 0;
}

static int heap_remove(heap_t *heap, void **element)
{
    if (heap->size <= 0)
        return -1;

    *element = heap->queue[0];

    shiftdown(heap, 0, heap->queue[heap->size - 1]);
    heap->queue[heap->size - 1] = 0;
    heap->size--;

    return 0;
}

static int heap_destroy(heap_t *heap)
{
    free(heap->queue);
    free(heap);

    return 0;
}

static int heap_size(heap_t *heap)
{
    return heap->size;
}

heap_t * get_global_heap()
{
    heap_t * heap = global_heap;

    if (heap == NULL) {
        int capacity = 1024;
        heap = heap_alloc();
        heap_set(heap, "comparator", (void *)greater_than);
        heap_set(heap, "capacity", (void *) &capacity);
        heap_init(heap);
        global_heap = heap;
    } 

    return global_heap;
}

int __register_init_func(int level, int (*func)()) 
{
    init_func_entry_t *element;
    heap_t * heap = get_global_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func = func;
    element->args_count = 0;

    heap_add(heap, (void *)element);

    return 0;
}

int __register_init_func1(int level, int (*func)(void *arg), void *arg) 
{
    init_func_entry_t *element;
    heap_t * heap = get_global_heap();

    element = (init_func_entry_t *) malloc(sizeof(init_func_entry_t));
    if (element == NULL) {
        printf("register init func, malloc err\n");
        return -1;
    }

    element->level = level;
    element->func1 = func;
    element->args_count = 1;
    element->arg1 = arg;

    heap_add(heap, (void *)element);

    return 0;
}

int __register_init_func2(int level, int (*func)(void *arg1, void *arg2), 
        void *arg1, void *arg2) 
{
    init_func_entry_t *element;
    heap_t * heap = get_global_heap();

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

    heap_add(heap, (void *)element);

    return 0;
}

int __register_init_func3(int level, int (*func)(void *arg1, void *arg2, void *arg3), 
        void *arg1, void *arg2, void *arg3) 
{
    init_func_entry_t *element;
    heap_t * heap = get_global_heap();

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

    heap_add(heap, (void *)element);

    return 0;
}

int execute_init_funcs() 
{
    int i, size = 0;
    init_func_entry_t *element;
    heap_t * heap = get_global_heap();

    size = heap_size(heap);
    for(i=0; i< size; i++){
        heap_remove(heap, (void **)&element);
        printf("%d ", element->level);
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

    printf("\n");

    heap_destroy(heap);

    return 0;
}



#if 0
int test1()
{
    printf("test1\n");

    return 0;
}

int test2(void *arg1)
{
    printf("test2, arg1=%d\n", (int)arg1);

    return 0;
}



int test3(void * arg1, void * arg2)
{
    printf("test3, arg1=%d, arg2=%d\n", (int)arg1, (int) arg2);

    return 0;
}


REGISTER_INIT_FUNC(5, test1);
REGISTER_INIT_FUNC_ARG2(1, test3, (void*)3,(void *)2);
REGISTER_INIT_FUNC_ARG1(4, test2, (void*) 1);

int main()
{
    execute_init_funcs();

    return 0;
}
#endif
