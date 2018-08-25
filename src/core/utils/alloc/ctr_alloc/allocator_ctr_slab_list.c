/*
 * =====================================================================================
 *
 *       Filename:  allocator_ctr_slab_list.c 
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 06:16:22 PM
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

/*-----------------------------------------------------------------------------
 *  slab list operation
 *-----------------------------------------------------------------------------*/
void slab_init_head_list(struct list_head **hl_head, uint8_t lock_type)
{
    ctr_slab_head_list_t *head_list;

    head_list = (ctr_slab_head_list_t *)malloc(sizeof(ctr_slab_head_list_t));
    if (head_list == NULL){
        dbg_str(ALLOC_ERROR, "malloc slab list_head_list");
        return;
    }

    sync_lock_init(&head_list->head_lock, lock_type);
    head_list->count = 0;
    INIT_LIST_HEAD(&head_list->list_head);
    *hl_head = &head_list->list_head;
}

void slab_release_head_list(struct list_head *hl_head)
{
    ctr_slab_head_list_t *head_list;

    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);
    sync_lock_destroy(&head_list->head_lock);
    free(head_list);
}

void slab_attach_list(struct list_head *new_head, struct list_head *hl_head)
{
    ctr_slab_head_list_t *head_list;
    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_add(new_head, hl_head);
    head_list->count++;
    sync_unlock(&head_list->head_lock);
}

void slab_detach_list(struct list_head *del_head, struct list_head *hl_head)
{
    ctr_slab_head_list_t *head_list;

    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_del(del_head);
    head_list->count--;
    sync_unlock(&head_list->head_lock);
}

ctr_slab_t *slab_detach_front_list(struct list_head *hl_head)
{
    ctr_slab_head_list_t *head_list;
    ctr_slab_t *slab_list;

    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);
    sync_lock(&head_list->head_lock, NULL);
    if (hl_head->next != hl_head){
        head_list->count--;
        slab_list = container_of(hl_head->next, ctr_slab_t, list_head);
        list_del(hl_head->next);
    } else {
        sync_unlock(&head_list->head_lock);
        return NULL;
    }
    sync_unlock(&head_list->head_lock);

    slab_list->stat_flag = 1;

    return slab_list;
}

#if 0

uint32_t slab_get_slab_index(allocator_t *alloc, uint32_t size)
{
    int data_min_size = alloc->priv.ctr_alloc.data_min_size;

    return ((size + data_min_size - 1) / data_min_size) - 1;
}

#else

int slab_get_slab_index(allocator_t *alloc, int size)
{
    int i;
    int remainder, interger;
    int min_data_size, max_index;
    int ret = -1;

    min_data_size = alloc->priv.ctr_alloc.data_min_size;
    max_index     = alloc->priv.ctr_alloc.slab_array_max_num;
    for (i = 0; i < max_index; i++) {
        interger  = size / (____pow(2, i) * min_data_size);
        remainder = size % (____pow(2, i) * min_data_size);
        if (interger == 0 || (interger == 1 && remainder == 0)) {
            return i;
        }
    }

    return ret;
}

#endif

ctr_slab_t* 
slab_detach_front_list_from_free_slabs(allocator_t *alloc, uint32_t size)
{
    struct list_head **free_slabs = alloc->priv.ctr_alloc.free_slabs;
    uint32_t index;

    index = slab_get_slab_index(alloc, size);

    return slab_detach_front_list(free_slabs[index]);
}

void 
slab_detach_list_from_used_slabs(allocator_t *alloc,
                                 struct list_head *del_head, uint32_t size)
{
    uint32_t index;
    struct list_head **used_slabs = alloc->priv.ctr_alloc.used_slabs;

    index = slab_get_slab_index(alloc, size);

    return slab_detach_list(del_head, used_slabs[index]);
}

void 
slab_attach_list_to_used_slabs(allocator_t *alloc,
                               struct list_head *new_head, uint32_t size)
{
    uint32_t index;
    struct list_head **used_slabs = alloc->priv.ctr_alloc.used_slabs;

    index = slab_get_slab_index(alloc, size);

    return slab_attach_list(new_head, used_slabs[index]);
}

void 
slab_attach_list_to_free_slabs(allocator_t *alloc, 
                               struct list_head *new_head, uint32_t size)
{
    uint32_t index;
    struct list_head **free_slabs = alloc->priv.ctr_alloc.free_slabs;

    index = slab_get_slab_index(alloc, size);

    return slab_attach_list(new_head, free_slabs[index]);
}

void slab_print_list(ctr_slab_t *slab_list, uint16_t slab_index)
{
    if (slab_index != 0xffff) {
        dbg_str(ALLOC_VIP, 
                "slab info, slab index =%d\t, alloc size=%d\t, req_size =%d\t, "
                "slab_size=%d\t, slab_start:%p\t, slab_end:%p, tag:%s", 
                slab_index, 
                slab_list->size, slab_list->data_size, 
                slab_list->slab_size, 
                slab_list, 
                (uint8_t*)slab_list +  slab_list->size + sizeof(ctr_slab_t), 
                slab_list->tag);
    } else {
        dbg_str(ALLOC_VIP, 
                "slab info, alloc size=%d\t, req_size =%d\t, slab_size=%d\t, "
                "slab_start:%p\t, slab_end:%p, tag:%s", 
                slab_list->size, slab_list->data_size, 
                slab_list->slab_size, 
                slab_list, 
                (uint8_t*)slab_list +  slab_list->size + sizeof(ctr_slab_t), 
                slab_list->tag);
    }
}

void slab_print_list_for_each(struct list_head *hl_head, uint16_t slab_index)
{
    ctr_slab_head_list_t *head_list;
    ctr_slab_t *slab_list;
    struct list_head *pos, *n;

    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);

    sync_lock(&head_list->head_lock, NULL);
    list_for_each_safe(pos, n, hl_head) {
        slab_list = container_of(pos, ctr_slab_t, list_head);
        slab_print_list(slab_list, slab_index);
    }
    sync_unlock(&head_list->head_lock);
}

int slab_destroy_used_huge_slabs(struct list_head *hl_head)
{
    ctr_slab_head_list_t *head_list;
    ctr_slab_t *slab_list;

    head_list = container_of(hl_head, ctr_slab_head_list_t, list_head);
    sync_lock(&head_list->head_lock, NULL);
    while (hl_head->next != hl_head){
        slab_list = container_of(hl_head->next, ctr_slab_t, list_head);
        list_del(hl_head->next);
        dbg_str(ALLOC_VIP, "free huge slab leaked, size=%d", slab_list->size);
        free(slab_list);
    }
    sync_unlock(&head_list->head_lock);

    slab_release_head_list(hl_head);

    return 0;
}
