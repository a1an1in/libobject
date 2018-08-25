/*
 * =====================================================================================
 *
 *       Filename:  rbtree_map.h
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

#ifndef __RBTREE_MAP_H__
#define __RBTREE_MAP_H__

#include <libobject/core/utils/data_structure/rbtree_map_struct.h>

rbtree_map_t * rbtree_map_alloc(allocator_t *allocator);
int rbtree_map_init(rbtree_map_t *map,
		            uint32_t key_size,
		            uint32_t data_size);
rbtree_map_pos_t * rbtree_map_pos_next(rbtree_map_pos_t *it,rbtree_map_pos_t *next);
rbtree_map_pos_t* rbtree_map_pos_prev(rbtree_map_pos_t *it,rbtree_map_pos_t *prev);
int rbtree_map_pos_equal(rbtree_map_pos_t *it1,rbtree_map_pos_t *it2);
void *rbtree_map_pos_get_pointer(rbtree_map_pos_t *it);
void *rbtree_map_pos_get_kpointer(rbtree_map_pos_t *it);
rbtree_map_pos_t * rbtree_map_begin(rbtree_map_t *map, rbtree_map_pos_t *begin);
rbtree_map_pos_t * rbtree_map_end(rbtree_map_t *map, rbtree_map_pos_t *end);
int rbtree_map_insert_data(rbtree_map_t *map, void *value);
int rbtree_map_insert(rbtree_map_t *map,void *key,void *value);
int rbtree_map_insert_with_numeric_key(rbtree_map_t *map,int key,void *value);
int rbtree_map_delete(rbtree_map_t *map, rbtree_map_pos_t *it);
int rbtree_map_remove(rbtree_map_t *map, rbtree_map_pos_t *it);
int rbtree_map_search(rbtree_map_t *map, void *key, rbtree_map_pos_t *it);
int rbtree_map_search_by_numeric_key(rbtree_map_t *map, int key, rbtree_map_pos_t *it);
int rbtree_map_destroy(rbtree_map_t *map);

#endif
