/*
 * =====================================================================================
 *
 *       Filename:  vector.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/02/2015 02:47:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <string.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/utils/dbg/debug.h>

int 
vector_copy_backward(vector_t *vector, vector_pos_t *to, 
                     vector_pos_t *from, uint32_t count)
{
    uint32_t from_pos, to_pos;
    void *vector_head = vector->vector_head;
    uint32_t step     = vector->step;
    uint32_t num_per;
    
    from_pos = from->vector_pos + count;
    to_pos   = to->vector_pos + count;

    num_per = to_pos - from_pos;
    for (; count > 0; ) {
        to_pos   -= num_per;
        from_pos -= num_per;
        count    -= num_per;
        num_per   = count - num_per > 0 ? num_per :count;
        dbg_str(VECTOR_DETAIL, "to_pos=%d, from_pos=%d", to_pos, from_pos);
        memcpy(vector_head + to_pos * step, 
               vector_head + from_pos * step, num_per * step);
    }

    return 0;
}

int 
vector_copy_forward(vector_t *vector, vector_pos_t *to, 
                    vector_pos_t *from, uint32_t count)
{
    uint32_t from_pos = from->vector_pos;
    uint32_t to_pos   = to->vector_pos;
    void *vector_head = vector->vector_head;
    uint32_t step     = vector->step;
    uint32_t num_per;
    
    num_per = from_pos - to_pos;

    for (; count > 0; ) {
        num_per = count - num_per > 0 ? num_per :count;

        dbg_str(VECTOR_DETAIL, "to_pos=%d, from_pos=%d, num_per=%d", 
                to_pos, from_pos, num_per);

        memcpy(vector_head + to_pos * step, 
               vector_head + from_pos * step, num_per * step);
        to_pos   += num_per;
        from_pos += num_per;
        count    -= num_per;
    }

    return 0;
}

int vector_copy(vector_t *vector, vector_pos_t *to, vector_pos_t *from, uint32_t count)
{
    uint32_t from_pos = from->vector_pos;
    uint32_t to_pos   = to->vector_pos;
    
    dbg_str(VECTOR_IMPORTANT, "count=%d", count);
    if (from_pos > to_pos) {//forward
        vector_copy_forward(vector, to, from, count);
    } else {//backward
        vector_copy_backward(vector, to, from, count);
    }

    return 0;
}

vector_t *vector_create(allocator_t *allocator, uint8_t lock_type)
{
    vector_t *ret = NULL;

    ret = (vector_t *)allocator_mem_alloc(allocator, sizeof(vector_t));
    if (ret == NULL) {
        dbg_str(VECTOR_ERROR, "allock err");
    }
    ret->allocator = allocator;
    ret->lock_type = lock_type;
    return ret;
}

int vector_init(vector_t *vector, uint32_t data_size, uint32_t capacity)
{
    dbg_str(VECTOR_DETAIL, "vector init, capacity=%d, step=%d", capacity, vector->step);

    vector->step        = sizeof(void *);
    vector->data_size   = data_size;
    vector->capacity    = capacity;
    vector->size        = 0;
    vector->vector_head = allocator_mem_alloc(vector->allocator, 
                                              capacity * (vector->step));
    if (vector->vector_head == NULL) {
        dbg_str(VECTOR_ERROR, "vector_init");
    }

    memset(vector->vector_head, 0, capacity * (vector->step));
    dbg_str(VECTOR_DETAIL, "vector_head:%p, size=%d", 
            vector->vector_head, capacity * (vector->step));

    vector_pos_init(&vector->begin, 0, vector);
    vector_pos_init(&vector->end, 0, vector);
    sync_lock_init(&vector->vector_lock, vector->lock_type);

    return 0;
}

int vector_add(vector_t *vector, vector_pos_t *it, void *data)
{
    uint32_t insert_pos = it->vector_pos;
    uint32_t end_pos    = vector->end.vector_pos;
    uint32_t count      = end_pos - insert_pos;
    void **vector_head  = vector->vector_head;
    vector_pos_t to;
    
    vector_pos_init(&to, insert_pos + 1, vector);

    dbg_str(VECTOR_DETAIL, "insert_pos=%d, to_pos=%d", 
            insert_pos, 
            to.vector_pos);

    sync_lock(&vector->vector_lock, NULL);

    vector_copy(vector, &to, it, count);
    vector_head[insert_pos] = data;
    vector_pos_init(&vector->end, end_pos + 1, vector);
    vector->size+=1;
    sync_unlock(&vector->vector_lock);

    return 0;
}

int vector_add_at(vector_t *vector, int index, void *data)
{
    uint32_t set_pos   = index;
    uint32_t end_pos   = vector->end.vector_pos;
    void **vector_head = vector->vector_head;
    int ret  = 0;
    
    dbg_str(VECTOR_DETAIL, "set_pos=%d, index=%d, element=%p",
            set_pos, index, data);

    sync_lock(&vector->vector_lock, NULL);

    vector_head[set_pos] = data;
    if (set_pos + 1 > end_pos) {
        vector_pos_init(&vector->end, set_pos + 1, vector);
    }
    vector->size+=1;
    sync_unlock(&vector->vector_lock);

    return ret;
}

int vector_add_front(vector_t *vector, void *data)
{
    dbg_str(VECTOR_WARNNING, "not support vector_add_front");
    return 0;
}

int vector_add_back(vector_t *vector, void *data)
{
    uint32_t data_size = vector->data_size;
    void **vector_head = vector->vector_head;
    uint32_t step      = vector->step;
    uint32_t capacity  = vector->capacity;
    uint32_t push_pos  = vector->end.vector_pos;

    sync_lock(&vector->vector_lock, NULL);

    if (push_pos < capacity) {
        vector_head[push_pos++] = data;
        vector_pos_init(&vector->end, push_pos, vector);
    } else {
        dbg_str(VECTOR_WARNNING, "realloc mem");

        vector->vector_head = allocator_mem_alloc(vector->allocator, 
                                                  2*capacity * (vector->step));
        if(vector->vector_head == NULL){
            dbg_str(VECTOR_ERROR, "vector_add_back, realloc mem");
        }

        vector->capacity = 2 * capacity;
        memcpy(vector->vector_head, vector_head, capacity * step);
        allocator_mem_free(vector->allocator, vector_head);
        vector_head = vector->vector_head;
        vector_head[push_pos++] = data;
        vector_pos_init(&vector->end, push_pos, vector);
    }
    vector->size+=1;
    sync_unlock(&vector->vector_lock);

    dbg_str(VECTOR_DETAIL, "vector_add_back, push_pos=%d, capacity=%d", 
            push_pos, vector->capacity);

    return 0;
}

int vector_delete(vector_t *vector, vector_pos_t *it)
{
    uint32_t delete_pos = it->vector_pos;
    uint32_t end_pos    = vector->end.vector_pos;
    uint32_t count      = end_pos - delete_pos;
    void **vector_head  = vector->vector_head;
    vector_pos_t from;

    vector_pos_init(&from, delete_pos + 1, vector);

    sync_lock(&vector->vector_lock, 0);

    if (vector_pos_equal(it, &vector->end)) {
        dbg_str(VECTOR_WARNNING, "can't del end pos");
    } else if (vector_pos_equal(it, &vector->begin)&&
             vector_pos_equal(&from, &vector->end))
    {
        dbg_str(VECTOR_WARNNING, "vector is NULL");
    } else {
        vector_copy(vector, it, &from, count);
        vector_pos_init(&vector->end, end_pos - 1, vector);
    }
    vector->size-=1;
    sync_unlock(&vector->vector_lock);

    return 0;
}

int vector_delete_front(vector_t *vector)
{
    dbg_str(VECTOR_WARNNING, "not support vector_delete_front");
    return 0;
}

int vector_delete_back(vector_t *vector)
{
    uint32_t pop_pos  = vector->end.vector_pos;
    
    dbg_str(VECTOR_DETAIL, "pop back");

    sync_lock(&vector->vector_lock, NULL);
    if (!vector_pos_equal(&vector->begin, &vector->end)) {
        vector_pos_init(&vector->end, --pop_pos, vector);
    } else {
        dbg_str(VECTOR_WARNNING, "vector is NULL");
    }
    vector->size-=1;
    sync_unlock(&vector->vector_lock);

    return 0;
}

int vector_remove(vector_t *vector, int index, void **element)
{
    uint32_t remove_pos = index;
    uint32_t end_pos    = vector->end.vector_pos;
    uint32_t count      = end_pos - remove_pos;
    void **vector_head  = vector->vector_head;
    vector_pos_t from;
    vector_pos_t it;

    vector_pos_init(&it, remove_pos, vector);
    vector_pos_init(&from, remove_pos + 1, vector);

    sync_lock(&vector->vector_lock, 0);

    if (vector_pos_equal(&it, &vector->end)) {
        dbg_str(VECTOR_WARNNING, "can't del end pos");
    } else if (vector_pos_equal(&it, &vector->begin)&&
             vector_pos_equal(&from, &vector->end))
    {
        dbg_str(VECTOR_WARNNING, "vector is NULL");
    } else if (element != NULL) {
        *element = vector_head[remove_pos];
        vector_copy(vector, &it, &from, count);
        vector_pos_init(&vector->end, end_pos - 1, vector);
    } else {
        vector_copy(vector, &it, &from, count);
        vector_pos_init(&vector->end, end_pos - 1, vector);
    }
    vector->size-=1;
    sync_unlock(&vector->vector_lock);

    return 0;
}

int vector_remove_back(vector_t *vector, void **element)
{
    uint32_t end_pos    = vector->end.vector_pos;
    uint32_t remove_pos = end_pos - 1;
    uint32_t count      = end_pos - remove_pos;
    void **vector_head  = vector->vector_head;
    vector_pos_t it;
    vector_pos_t from;

    vector_pos_init(&it, remove_pos, vector);
    vector_pos_init(&from, remove_pos + 1, vector);

    sync_lock(&vector->vector_lock, 0);

    if (end_pos == 0) {
        dbg_str(VECTOR_WARNNING, "vector is NULL");
    } else if (element != NULL) {
        *element = vector_head[remove_pos];
        vector_copy(vector, &it, &from, count);
        vector_pos_init(&vector->end, end_pos - 1, vector);
    } else {
        vector_copy(vector, &it, &from, count);
        vector_pos_init(&vector->end, end_pos - 1, vector);
    }
    vector->size-=1;;
    sync_unlock(&vector->vector_lock);

    return 0;
}

int vector_peek_at(vector_t *vector, int index, void **element)
{
    uint32_t get_pos   = index;
    void **vector_head = vector->vector_head;
    int ret = 0;

    sync_lock(&vector->vector_lock, NULL);
    *element = vector_head[get_pos];
    sync_unlock(&vector->vector_lock);
    dbg_str(VECTOR_DETAIL, "get_pos=%d, element =%p", get_pos, *element);

    return 0;
}

int vector_destroy(vector_t *vector)
{
    dbg_str(VECTOR_DETAIL, "vector_destroy");

    sync_lock_destroy(&vector->vector_lock);
    allocator_mem_free(vector->allocator, vector->vector_head);
    allocator_mem_free(vector->allocator, vector);
}

struct test{
    int a;
    int b;
};

void print_vector_data(void *element)
{
    struct test *t = (struct test*)element;

    dbg_str(DBG_DETAIL,"t->a = %d, t->b = %d", t->a, t->b);
}

static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

int test_datastructure_vector()
{
    vector_t *vector;
    vector_pos_t pos;
    allocator_t *allocator = allocator_get_default_alloc();
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret;

    dbg_str(DBG_DETAIL,"test_datastructure_vector begin");
    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    vector = vector_create(allocator,0);
    vector_init(vector,sizeof(void *),10);

    vector_add_back(vector, &t0);
    vector_add_back(vector, &t1);
    vector_add_back(vector, &t2);
    vector_add_back(vector, &t3);
    vector_add_back(vector, &t4);

    dbg_str(DBG_DETAIL,"vector for each");
    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    dbg_str(DBG_DETAIL,"test get");
    vector_peek_at(vector,2, (void **)&t);
    dbg_buf(DBG_DETAIL,"get data:", (uint8_t *)t,vector->data_size);

    dbg_str(DBG_DETAIL,"test set");
    vector_add_at(vector,1, &t5);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    vector_destroy(vector);
    dbg_str(DBG_DETAIL,"test_datastructure_vector end");
    return ret;
}
