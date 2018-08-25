/**
 * @file lab.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-10-11
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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>     /* basic system data types */
#include <sys/socket.h>    /* basic socket definitions */
#include <netinet/in.h>    /* sock_addr_in{} and other Internet defns */
#include <arpa/inet.h>     /* inet(3) functions */
#include <fcntl.h>         /* nonblocking */
#include <sys/resource.h>  /*setrlimit */
#include <signal.h>
#include <sys/un.h>
#include <libobject/core/utils/dbg/debug.h>

#if 0
typedef struct small_heap_s {
    void *queue[1024];
    int size;
    int (*comparator)(void *element1, void *element2);
}heap_t;

int greater_than(void *element1, void *element2) 
{
    long long e1, e2;

    e1 = (long long) element1;
    e2 = (long long) element2;
    return (e1 < e2) ? 1 : 0;
}

void shiftup(heap_t *heap, int index, void *e)
{
    while(index > 0) {
        int parent = (index - 1) / 2;
        if (heap->comparator(e, heap->queue[parent])) {
            break;
        }
        heap->queue[index] = heap->queue[parent];
        index = parent;
    }
    heap->queue[index] = e;
}

void heap_add(heap_t *heap, void *e){

    int index = heap->size;

    shiftup(heap, index, e);
    heap->size++;
}

void shiftdown(heap_t *heap, int index, void *e)
{
    int size = heap->size;
    int half = size / 2;

    while (index < half) {
        int child = 2 * index + 1;
        int right = child + 1;
        if (right < size && heap->comparator(heap->queue[child], heap->queue[right])) {
            child = right;
        }
        if (heap->comparator(heap->queue[child], e)) {
            break;
        }
        heap->queue[index] = heap->queue[child];
        index = child;
    }

    heap->queue[index] = e;
}

int heap_remove(heap_t *heap, void **element)
{
    if (heap->size <= 0)
        return -1;

    *element = heap->queue[0];

    shiftdown(heap, 0, heap->queue[heap->size - 1]);
    heap->queue[heap->size--] = 0;

    return 0;
}

int heap_size(heap_t *heap)
{
    return heap->size;
}

int lab3()
{
    heap_t heap;

    memset(&heap, 0, sizeof(heap_t));
    heap.comparator = greater_than;
    heap_add(&heap, (void *)4);
    heap_add(&heap, (void *)3);
    heap_add(&heap, (void *)7);
    heap_add(&heap, (void *)2);

    dbg_buf(DBG_DETAIL,"heap:", (void *)heap.queue, 20);
    int size = heap_size(&heap);
    void *element;
    for(int i=0; i< size; i++){
        heap_remove(&heap, &element);
        dbg_str(DBG_DETAIL, "%d", (long long)element);
    }

    return 0;
}


#endif

int lab3()
{}
