#ifndef __RBTREE_MAP_STRUCT_H__
#define __RBTREE_MAP_STRUCT_H__

#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/data_structure/rbtree.h>
#include "libobject/core/utils/thread/sync_lock.h"

#ifndef __KEY_CMP_FPT__
#define __KEY_CMP_FPT__
typedef int (*key_cmp_fpt)(void *key1,void *key2);
#endif

struct rbtree_map_node {
  	struct rb_node node;
    uint8_t key_type;
  	void *key;
    void *value;
};
typedef struct rbtree_map_pos_s{
	struct rb_node *rb_node_p;
	struct rb_root *tree_root;
	struct rbtree_map_s *map;
}rbtree_map_pos_t;

typedef struct rbtree_map_s{
	key_cmp_fpt key_cmp_func;
	sync_lock_t map_lock;
	uint8_t lock_type;
    uint8_t enable_same_key;
    uint8_t key_type;
    uint8_t key_len;
    uint32_t count;
	allocator_t *allocator;
	struct rb_root *tree_root;
	rbtree_map_pos_t begin ,end;
}rbtree_map_t;

#endif
