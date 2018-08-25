/*
 * =====================================================================================
 *
 *       Filename:  allocator_ctr_mempool_list.c 
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 06:45:01 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan 
 *        Company:  
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
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/inc_files.h>

void mempool_init_head_list(struct list_head **hl_head, uint8_t lock_type)
{
    ctr_mempool_head_list_t *head_list;

    head_list = (ctr_mempool_head_list_t *)malloc(sizeof(ctr_mempool_head_list_t));
    if (head_list == NULL){
        dbg_str(ALLOC_ERROR, "malloc mempool list_head_list");
        return;
    }
    sync_lock_init(&head_list->head_lock, lock_type);
    head_list->count = 0;
    INIT_LIST_HEAD(&head_list->list_head);
    *hl_head = &head_list->list_head;
}

void mempool_attach_list(struct list_head *new_head, struct list_head *hl_head)
{
    ctr_mempool_head_list_t *head_list;

    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_add(new_head, hl_head);
    head_list->count++;
    sync_unlock(&head_list->head_lock);
}

/* not del list, just detach it from one link list */
void mempool_detach_list(struct list_head *del_head, struct list_head *hl_head)
{
    ctr_mempool_head_list_t *head_list;
    ctr_mempool_t *mempool_list;

    dbg_str(ALLOC_DETAIL, "mempool_detach_list");
    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);
    sync_lock(&head_list->head_lock, NULL);
    list_del(del_head);
    head_list->count--;
    mempool_list = container_of(del_head, ctr_mempool_t, list_head);
    sync_unlock(&head_list->head_lock);

    dbg_str(ALLOC_DETAIL, "del_mempool list, list count =%d", head_list->count);
}

void mempool_destroy_list(struct list_head *del_head, struct list_head *hl_head)
{
    ctr_mempool_head_list_t *head_list;
    ctr_mempool_t *mempool_list;

    dbg_str(ALLOC_DETAIL, "mempool_detach_list");
    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);
    sync_lock(&head_list->head_lock, NULL);
    list_del(del_head);
    head_list->count--;
    mempool_list = container_of(del_head, ctr_mempool_t, list_head);
    mempool_release_list(mempool_list); 
    sync_unlock(&head_list->head_lock);

    dbg_str(ALLOC_DETAIL, "del_mempool list, list count =%d", head_list->count);
}

void mempool_print_head_list(struct list_head *hl_head)
{
    ctr_mempool_head_list_t *head_list;
    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);
    dbg_str(ALLOC_VIP, "mempool_print_head_list:mempool count:%d", head_list->count);
}

void mempool_print_list(ctr_mempool_t *mempool_list)
{
    dbg_str(ALLOC_VIP, "mempool info, start addr:%p, depth:%d, min_depth:%d, "
            "mempool_capacity=%d, list_head_addr:%p", 
            mempool_list->start, 
            mempool_list->depth, 
            mempool_list->min_depth, 
            mempool_list->size, 
            &mempool_list->list_head);
}

void mempool_print_list_for_each(struct list_head *hl_head)
{
    ctr_mempool_head_list_t *head_list;
    ctr_mempool_t *mempool_list;
    struct list_head *pos, *n;

    mempool_print_head_list(hl_head);

    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_for_each_safe(pos, n, hl_head) {
        mempool_list = container_of(pos, ctr_mempool_t, list_head);
        mempool_print_list(mempool_list);
    }
    sync_unlock(&head_list->head_lock);
}

ctr_mempool_t *
mempool_create_list(allocator_t *allocator)
{
    ctr_alloc_t *ctr_alloc = &allocator->priv.ctr_alloc; 
    ctr_mempool_t *mempool;
    void *p;

    if ((p = malloc(sizeof(ctr_mempool_t))) == NULL){
        dbg_str(ALLOC_ERROR, "malloc ctr_mempool_t");
        return NULL;
    } else {
        mempool = (ctr_mempool_t *)p;
        dbg_str(ALLOC_DETAIL, "mempool addr:%p, sizeof(p)=%d", mempool, sizeof(mempool));
    }

    if ((p = malloc(ctr_alloc->mempool_capacity)) == NULL){
        dbg_str(ALLOC_ERROR, "malloc mem pool err");
        free(mempool);
        return NULL;
    }

    mempool->size      = ctr_alloc->mempool_capacity;
    mempool->start     = p;
    mempool->depth     = ctr_alloc->mempool_capacity;
    mempool->min_depth = ctr_alloc->data_min_size + sizeof(ctr_slab_t);

    return mempool;
}

void *mempool_release_list(ctr_mempool_t *mempool)
{
    dbg_str(ALLOC_DETAIL, "mempool_release_list");
    free(mempool->start);
    free(mempool);
}

//not release lock
void mempool_destroy_lists(struct list_head *hl_head)
{
    struct list_head *pos, *n;

    dbg_str(ALLOC_DETAIL, "mempool_destroy_lists");
    list_for_each_safe(pos, n, hl_head) {
        dbg_str(ALLOC_DETAIL, "run at here");
        mempool_destroy_list(pos, hl_head);
    }
}

#if 0

uint32_t get_real_alloc_mem_size(allocator_t *allocator, uint32_t size)
{
    int data_min_size = allocator->priv.ctr_alloc.data_min_size;

    return (slab_get_slab_index(allocator, size) + 1 ) * data_min_size;
}

#else

uint32_t get_real_alloc_mem_size(allocator_t *allocator, uint32_t size)
{
    int data_min_size = allocator->priv.ctr_alloc.data_min_size;
    int index;

    index = slab_get_slab_index(allocator, size);
    return ____pow(2, index) * data_min_size;
}

#endif

ctr_mempool_t *
mempool_find_appropriate_pool(allocator_t *allocator, uint32_t size)
{
    ctr_alloc_t *ctr_alloc = &allocator->priv.ctr_alloc; 
    struct list_head *hl_head = ctr_alloc->pool;
    struct list_head *empty_pool_hl_head = ctr_alloc->empty_pool;
    ctr_mempool_head_list_t *head_list;
    ctr_mempool_t *mempool_list;
    struct list_head *pos, *n;

    head_list = container_of(hl_head, ctr_mempool_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_for_each_safe(pos, n, hl_head) {
        mempool_list = container_of(pos, ctr_mempool_t, list_head);
        if (mempool_list->depth >= 
                get_real_alloc_mem_size(allocator, size) + sizeof(ctr_slab_t))
        {
            dbg_str(ALLOC_DETAIL, "find an appropriate_pool, mempool_list->depth=%d",
                    mempool_list->depth);
            sync_unlock(&head_list->head_lock);
            return mempool_list;
        } else if (mempool_list->depth < mempool_list->min_depth){
            dbg_str(ALLOC_DETAIL, "this mempool is empty");
            sync_unlock(&head_list->head_lock);
            mempool_detach_list(pos, hl_head);
            mempool_attach_list(pos, empty_pool_hl_head);
            sync_lock(&head_list->head_lock, NULL);
        }
    }
    sync_unlock(&head_list->head_lock);

    return NULL;
}

ctr_slab_t *
mempool_alloc_slab_list(allocator_t *allocator, uint32_t size)
{

    ctr_mempool_t *mempool_list;
    ctr_alloc_t *ctr_alloc = &allocator->priv.ctr_alloc;
    ctr_slab_t *slab_list;

    dbg_str(ALLOC_DETAIL, "mempool_alloc_slab_list, size=%d", size);

    if (!(mempool_list = mempool_find_appropriate_pool(allocator, size))){
        dbg_str(ALLOC_DETAIL, "not find appropriate_pool, create a new pool");
        if ((mempool_list = mempool_create_list(allocator))){
            mempool_attach_list(&mempool_list->list_head, ctr_alloc->pool);
        } else {
            dbg_str(ALLOC_ERROR, "can't create a new mempool_list");
            return NULL;
        }
    }

    slab_list            = mempool_list->start + mempool_list->size - mempool_list->depth;
    slab_list->size      = get_real_alloc_mem_size(allocator, size);
    slab_list->data_size = size;
    slab_list->mem_addr  = &slab_list->data[0];
    slab_list->stat_flag = 1;
    slab_list->slab_size = slab_list->size + sizeof(ctr_slab_t);

    dbg_str(ALLOC_DETAIL, "slab_list, size=%d, data_size=%d, slab_size=%d", 
            slab_list->size,  slab_list->data_size, slab_list->slab_size);
    assert(slab_list->size >= slab_list->data_size);

    
    mempool_list->depth -= slab_list->slab_size;

    if (mempool_list->depth < 0) {
        dbg_str(DBG_WARNNING, "mempool_list->depth=%d, slab_list->slab_size=%d",
                mempool_list->depth, slab_list->slab_size);
    }

    return slab_list;
}
