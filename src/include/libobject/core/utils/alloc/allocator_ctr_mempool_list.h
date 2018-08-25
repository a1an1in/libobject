/*
 * =====================================================================================
 *
 *       Filename:  allocator_ctr_mempool_list.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 06:45:01 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __ALLOCATOR_CDS_MEMPOOL_LIST_H__
#define __ALLOCATOR_CDS_MEMPOOL_LIST_H__

//#include "allocator_ctr_alloc.h"

void mempool_init_head_list(struct list_head **hl_head,uint8_t lock_type);
void mempool_attach_list(struct list_head *new_head,struct list_head *hl_head);
void mempool_detach_list(struct list_head *del_head,struct list_head *hl_head);
void mempool_print_list(ctr_mempool_t *mempool_list);
void mempool_print_list_for_each(struct list_head *hl_head);
ctr_mempool_t *mempool_create_list(allocator_t *alloc);
void *mempool_release_list(ctr_mempool_t *mempool);
void mempool_destroy_lists(struct list_head *hl_head);
ctr_mempool_t *mempool_find_appropriate_pool(allocator_t *alloc,uint32_t size);
ctr_slab_t *mempool_alloc_slab_list(allocator_t *alloc,uint32_t size);

#endif
