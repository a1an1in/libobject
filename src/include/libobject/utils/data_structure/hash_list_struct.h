/*
 * =====================================================================================
 *
 *       Filename:  hash_list.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/14/2015 09:00:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __HASH_LIST_ST_H__
#define __HASH_LIST_ST_H__

#include "libobject/utils/data_structure/list.h"
#include "libobject/utils/data_structure/map_pair.h"
#include "libobject/utils/alloc/allocator.h"
#include "libobject/utils/thread/sync_lock.h"
#include "libobject/utils/data_structure/map_pair.h"

/*
 *typedef unsigned int uint32_t;
 *typedef unsigned short uint16_t;
 *typedef unsigned char uint8_t;
 */

typedef uint32_t (*hash_func_fpt)(void *key,uint32_t key_size,uint32_t bucket_size);

struct hash_map_node {
	struct hlist_node hlist_node;
	uint8_t value_pos;//value's pos at key array
	uint32_t data_size;
	char key[1];
};
typedef struct hash_map_pos_info{
	struct hlist_node *hlist_node_p; //node addr
	uint32_t bucket_pos;//bucket pos
	struct hlist_head *hlist;//hash table head addr
	struct hash_map_s *hmap;
}hash_map_pos_t;

typedef struct hash_map_s{
	uint32_t bucket_size;
	uint32_t data_size;
	uint32_t key_size;

	hash_func_fpt hash_func;
	key_cmp_fpt key_cmp_func;

	struct hlist_head *hlist;
	hash_map_pos_t begin,end;
	uint8_t lock_type;
	sync_lock_t map_lock;
	allocator_t *allocator;
	uint32_t node_count;
    uint8_t key_type;

    pair_t *pair;
}hash_map_t;

#endif
