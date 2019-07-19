/*
 * =====================================================================================
 *
 *       Filename:  link_list_struct.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/14/2015 09:32:42 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __LINK_LIST_STRUCT_H__
#define __LINK_LIST_STRUCT_H__

#include "libobject/core/utils/data_structure/list.h"
#include "libobject/core/utils/alloc/allocator.h"
#include "libobject/core/utils/thread/sync_lock.h"

/*
 *typedef unsigned char uint8_t;
 *typedef unsigned short uint16_t;
 *typedef unsigned int uint32_t;
 */

typedef struct llist_list_s{
	struct list_head list_head;
	void * data;
}list_t;
typedef struct llist_pos_s{
	struct list_head *list_head_p;
	struct llist_s *llist;
}list_pos_t;
typedef struct llist_s{
	sync_lock_t list_lock;
	uint32_t list_count;
	uint32_t data_size;
	list_pos_t begin,end,head;
	list_t *list;
	uint8_t lock_type;
	allocator_t *allocator;
}llist_t;
#endif
