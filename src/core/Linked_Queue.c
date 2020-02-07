/**
 * @file Linked_Queue.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-07
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
#include <libobject/core/Linked_Queue.h>
#include <libobject/event/Event_Base.h>

static int __construct(Linked_Queue *queue, char *init_str)
{
    llist_t *llist;
    allocator_t *allocator = queue->parent.obj.allocator;
    Queue *q = (Queue *)queue;
    int value_size = sizeof(void *);
    int lock_type = 1;

    dbg_str(OBJ_DETAIL, "queue construct, queue addr:%p", queue);

    llist = llist_alloc(allocator);
    llist_set(llist, "lock_type", &lock_type);
    llist_init(llist);

    queue->llist = llist;

    q->b = OBJECT_NEW(allocator, LList_Iterator, NULL);
    q->e = OBJECT_NEW(allocator, LList_Iterator, NULL);

    return 0;
}

static int __deconstrcut(Linked_Queue *queue)
{
    int ret;
    Queue *q = (Queue *)queue;

    dbg_str(OBJ_DETAIL, "queue deconstruct, queue addr:%p", queue);
    q->clear(q);
    object_destroy(q->b);
    object_destroy(q->e);
    llist_destroy(queue->llist);

    return 0;
}

static int __add(Linked_Queue *queue, void *element)
{
    dbg_str(OBJ_DETAIL, "Linked_Queue add");

    return llist_add_back(queue->llist, element);
}

static int __add_back(Linked_Queue *queue, void *element)
{
    return llist_add_back(queue->llist, element);
}

static int __add_front(Linked_Queue *queue, void *element)
{
    return llist_add_front(queue->llist, element);
}

static int __remove(Linked_Queue *queue, void **element)
{
    dbg_str(OBJ_DETAIL, "Linked_Queue remove");

    return llist_remove_front(queue->llist, element);
}

static int __remove_back(Linked_Queue *queue, void **element)
{
    return llist_remove_back(queue->llist, element);
}

static int __remove_front(Linked_Queue *queue, void **element)
{
    return llist_remove_front(queue->llist, element);
}

static Iterator* __begin(Queue *queue)
{
    Linked_Queue *q        = (Linked_Queue *)queue;
    allocator_t *allocator = queue->obj.allocator;
    LList_Iterator *iter   = (LList_Iterator *)queue->b;

    dbg_str(DBG_DETAIL, "queue begin");

    llist_begin(q->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static Iterator* __end(Queue *queue)
{
    Linked_Queue *q        = (Linked_Queue *)queue;
    allocator_t *allocator = queue->obj.allocator;
    LList_Iterator *iter   = (LList_Iterator *)queue->e;

    llist_end(q->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static size_t __size(Linked_Queue *queue)
{   
    size_t count = 0;
    sync_trylock(&(queue->llist->list_lock), NULL);
    count = queue->llist->list_count;
    sync_unlock(&(queue->llist->list_lock));

    return  count;
}

static size_t __is_empty(Linked_Queue *queue)
{    
    return  queue->size(queue) == 0 ? 1 : 0;
}

static int __peek_front(Linked_Queue *queue, void **element)
{
    Iterator * begin;
    Queue * q = (Queue *)queue;

    int ret = 0;
    if (!q->is_empty(q)) {
        begin = q->begin(q);
        (*element) = begin->get_vpointer(begin);
    } else {
        ret = -1;
    }
    return ret;
}

static int __peek_back(Linked_Queue *queue, void **element)
{
    Iterator * end, *prev;
    Queue * q = (Queue *)queue;

    int ret = 0;
    if (!q->is_empty(q)) {
        end = q->end(q);
        prev = end->prev(end);
        (*element) = prev->get_vpointer(prev);
    } else {
        ret = -1;
    }

    return ret;
}

static class_info_entry_t linked_queue_class_info[] = {
    Init_Obj___Entry(0 , Queue, parent),
    Init_Nfunc_Entry(1 , Linked_Queue, construct, __construct),
    Init_Nfunc_Entry(2 , Linked_Queue, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Linked_Queue, add, __add),
    Init_Vfunc_Entry(4 , Linked_Queue, add_front, __add_front),
    Init_Vfunc_Entry(5 , Linked_Queue, add_back, __add_back),
    Init_Vfunc_Entry(6 , Linked_Queue, remove, __remove),
    Init_Vfunc_Entry(7 , Linked_Queue, remove_front, __remove_front),
    Init_Vfunc_Entry(8 , Linked_Queue, remove_back, __remove_back),
    Init_Vfunc_Entry(9 , Linked_Queue, begin, __begin),
    Init_Vfunc_Entry(10, Linked_Queue, end, __end),
    Init_Vfunc_Entry(11, Linked_Queue, size, __size),
    Init_Vfunc_Entry(12, Linked_Queue, is_empty, __is_empty),
    Init_Vfunc_Entry(13, Linked_Queue, clear, NULL),
    Init_Vfunc_Entry(14, Linked_Queue, peek_front, __peek_front),
    Init_Vfunc_Entry(15, Linked_Queue, peek_back, __peek_back),
    Init_End___Entry(16, Linked_Queue),
};
REGISTER_CLASS("Linked_Queue", linked_queue_class_info);

struct test{
    int a;
    int b;
};
static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

static void queue_print(void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(DBG_DETAIL, "t->a=%d, t->b=%d", t->a, t->b);
}


static void queue_print_int(void *element)
{
    int *p =  (int*)element;
    dbg_str(DBG_IMPORTANT," element %d",*p);
}

int test_peek_linked_queue_peek()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    void * element = NULL;

    allocator_t *allocator = allocator_get_default_alloc();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 1; i < 10; i++) {
        buf[i] = i;
        queue->add(queue, &buf[i]);
    }

    /*
     *queue->for_each(queue, queue_print_int);
     */

    dbg_str(DBG_DETAIL, " peek front at: %p ", element);

    queue->peek_front(queue, &element);
    ret = assert_equal(&buf[1], element, sizeof(void *));
    if (ret == 0) {
        goto end;
    }

    queue->peek_back(queue, &element);
    ret = assert_equal(&buf[9], element, sizeof(void *));

end:
    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_peek_linked_queue_peek);

static int test_linked_queue_clear()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_alloc();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    queue->clear(queue);
    
    if (queue->size(queue) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(queue);

    return ret;

}
REGISTER_TEST_FUNC(test_linked_queue_clear);

static int test_linked_queue_is_empty()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_alloc();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    if (queue->is_empty(queue)) {
        ret = 0;
        goto end;
    } 
    for( i = 0; i < 10; i++) {
        queue->remove(queue, NULL);
    }

    if (queue->is_empty(queue)) {
        ret = 1;
        goto end;
    } 

end:
    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_linked_queue_is_empty);

static int test_linked_queue_size()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_alloc();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    if (queue->size(queue) == 10) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_linked_queue_size);



