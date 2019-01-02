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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/linked_queue.h>
#include <libobject/event/event_base.h>

static int __construct(Linked_Queue *queue, char *init_str)
{
    llist_t *llist;
    allocator_t *allocator = queue->parent.obj.allocator;
    Queue *q = (Queue *)queue;
    int value_size = sizeof(void *);
    int lock_type = 0;

    dbg_str(OBJ_DETAIL, "queue construct, queue addr:%p", queue);

    llist = llist_alloc(allocator);
    llist_set(llist, "lock_type", &lock_type);
    llist_set(llist, "data_size", &value_size);
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
    object_destroy(q->b);
    object_destroy(q->e);
    llist_destroy(queue->llist);

    return 0;
}

static int __set(Linked_Queue *queue, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        queue->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        queue->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        queue->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        queue->deconstruct = value;
    }
    else if (strcmp(attrib, "add") == 0) {
        queue->add = value;
    } else if (strcmp(attrib, "add_front") == 0) {
        queue->add_front = value;
    } else if (strcmp(attrib, "add_back") == 0) {
        queue->add_back = value;
    } else if (strcmp(attrib, "remove") == 0) {
        queue->remove = value;
    } else if (strcmp(attrib, "remove_front") == 0) {
        queue->remove_front = value;
    } else if (strcmp(attrib, "remove_back") == 0) {
        queue->remove_back = value;
    }else if (strcmp(attrib, "size") == 0) {
        queue->size = value;
    }else if (strcmp(attrib, "is_empty") == 0) {
        queue->is_empty = value;
    }else if (strcmp(attrib, "clear") == 0) {
        queue->clear = value;
    }else if (strcmp(attrib, "peek_front") == 0) {
        queue->peek_front = value;
    }else if (strcmp(attrib, "peek_back") == 0) {
        queue->peek_back = value;
    }
    else {
        dbg_str(OBJ_DETAIL, "queue set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Linked_Queue *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(OBJ_WARNNING, "queue get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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

static void __clear(Linked_Queue *queue)
{  
    list_pos_t pos ,next;

    sync_lock(&(queue->llist->list_lock), NULL);
    if (!queue->is_empty(queue)) {   
        for(llist_begin(queue->llist, &pos), llist_pos_next(&pos, &next);
            !llist_pos_equal(&pos, &queue->llist->head);
             pos = next, llist_pos_next(&pos, &next))
        {
            llist_delete(queue->llist,&pos);
        }
    sync_unlock(&(queue->llist->list_lock));

    }
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
    [0 ] = {ENTRY_TYPE_OBJ, "Queue", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add", __add, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_front", __add_front, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_back", __add_back, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove", __remove, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove_front", __remove_front, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove_back", __remove_back, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "begin", __begin, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "end", __end, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "size", __size, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "is_empty", __is_empty, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "clear", __clear, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "peek_front", __peek_front, sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "peek_back", __peek_back, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_END}, 
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

int Test_peek_linked_queue_peek()
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
REGISTER_TEST_FUNC(Test_peek_linked_queue_peek);

static int Test_linked_queue_clear()
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
REGISTER_TEST_FUNC(Test_linked_queue_clear);

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


