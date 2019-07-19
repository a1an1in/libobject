/*
 * =====================================================================================
 *
 *       Filename:  allocator_ctr_slab_list.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 06:16:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __ALLOCATOR_CDS_SLAB_LIST_H__
#define __ALLOCATOR_CDS_SLAB_LIST_H__
void slab_init_head_list(struct list_head **hl_head,uint8_t lock_type);
void slab_release_head_list(struct list_head *hl_head);
void slab_attach_list(struct list_head *new_head,struct list_head *hl_head);
void slab_detach_list(struct list_head *del_head,struct list_head *hl_head);
void slab_destroy_lists(struct list_head *hl_head);
ctr_slab_t *slab_detach_front_list(struct list_head *hl_head);
void slab_print_list_for_each(struct list_head *hl_head, uint16_t slab_index);
ctr_slab_t* slab_detach_front_list_from_free_slabs(allocator_t *alloc,uint32_t size);
void slab_detach_list_from_used_slabs(allocator_t *alloc,struct list_head *del_head,uint32_t size);
void slab_attach_list_to_used_slabs(allocator_t *alloc,struct list_head *new_head,uint32_t size);
void slab_attach_list_to_free_slabs(allocator_t *alloc,struct list_head *new_head,uint32_t size);
int slab_get_slab_index(allocator_t *alloc,int size);
int slab_destroy_used_huge_slabs(struct list_head *hl_head);
#endif

