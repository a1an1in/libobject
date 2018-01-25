/*
 * =====================================================================================
 *
 *       Filename:  rbtree_map_struct.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/01/2015 05:39:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __RBTREE_MAP_STRUCT_H__
#define __RBTREE_MAP_STRUCP_H__

#include <libobject/utils/alloc/allocator.h>
#include <libobject/utils/data_structure/rbtree.h>
#include <libobject/utils/data_structure/map_pair.h>
#include "libobject/utils/thread/sync_lock.h"
/*
 *typedef unsigned int uint32_t;
 *typedef unsigned short uint16_t;
 *typedef unsigned char uint8_t;
 */

#ifndef __KEY_CMP_FPT__
#define __KEY_CMP_FPT__
typedef int (*key_cmp_fpt)(void *key1,void *key2,uint32_t key_size);
#endif

struct rbtree_map_node {
  	struct rb_node node;
	uint8_t value_pos;
  	char key[1];
};
typedef struct rbtree_map_pos_s{
	struct rb_node *rb_node_p;
	struct rb_root *tree_root;
	struct rbtree_map_s *map;
}rbtree_map_pos_t;

typedef struct rbtree_map_s{
	uint32_t data_size;
	uint32_t key_size;
	uint32_t value_size;
	key_cmp_fpt key_cmp_func;
	sync_lock_t map_lock;
	uint8_t lock_type;
    uint8_t enable_same_key;
    uint8_t key_type;
	allocator_t *allocator;
	struct rb_root *tree_root;
	rbtree_map_pos_t begin ,end;
    pair_t *pair;
}rbtree_map_t;

#endif
