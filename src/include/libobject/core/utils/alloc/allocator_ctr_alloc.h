/*
 * =====================================================================================
 *
 *       Filename:  allocator_ctr_alloc.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 09:51:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __ALLOCATOR_SGI_ALLOC_H__
#define __ALLOCATOR_SGI_ALLOC_H__

#include "libobject/basic_types.h"
#include "libobject/core/utils/thread/sync_lock.h"
#include "libobject/core/utils/data_structure/list.h"

#define SLAB_ARRAY_MAX_NUM 12 
#define MEM_POOL_MAX_SIZE 1024*4
#define MEM_POOL_MIN_DEPTH 8

typedef struct ctr_mem_pool_head_list{
	uint32_t size;
	uint32_t count;
	sync_lock_t head_lock;    
	struct list_head list_head;
}ctr_mempool_head_list_t;

typedef struct ctr_mem_pool{
	void *start;
	int depth;
	int min_depth;
	int size;
	struct list_head list_head;
}ctr_mempool_t;

typedef struct slab_head_list{
	uint16_t size;
	uint32_t count;
	sync_lock_t head_lock;    
	struct list_head list_head;
}ctr_slab_head_list_t;

typedef struct slab{
    char tag[40];
	uint32_t size;
	uint32_t data_size;
	uint32_t slab_size;
	uint8_t stat_flag;
	uint8_t *mem_addr;
	struct list_head list_head;
	uint8_t data[0];
}ctr_slab_t;

typedef struct ctr_alloc_s{
	uint32_t slab_array_max_num;
	uint32_t mempool_capacity;
	uint32_t data_min_size;
	struct list_head *empty_pool;
	struct list_head *pool;
	struct list_head *used_huge_slabs;
	struct list_head **free_slabs;
	struct list_head **used_slabs;
}ctr_alloc_t;

int allocator_ctr_alloc_register();

static inline uint32_t ____pow(uint32_t x,uint32_t y)
{

    uint32_t pow_value = 1;
    uint32_t i;
    
    for(i = 0;i < y; i++)
        pow_value *= x;

    return pow_value;
}

#endif
