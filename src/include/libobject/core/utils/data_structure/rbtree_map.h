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
#include <libobject/core/utils/dbg/debug.h>

static inline int default_key_cmp_func(void *key1, void *key2)
{
    if (key1 > key2) return 1;
    else if (key1 < key2) return -1;
    else return 0;
}

static inline int string_key_cmp_func(void *key1, void *key2)
{
    return strcmp(key1, key2);
}

/*
 *static inline int timeval_key_cmp_func(void *key1,void *key2)
 *{
 *    struct timeval *k1, *k2;
 *
 *    k1 = (struct timeval *) key1;
 *    k2 = (struct timeval *) key2;
 *    
 *    if (    k1->tv_sec > k2->tv_sec || 
 *            (k1->tv_sec == k2->tv_sec && k1->tv_usec > k2->tv_usec))
 *    {
 *        return 1;
 *    } else if (k1->tv_sec == k2->tv_sec && k1->tv_usec == k2->tv_usec) {
 *        return 0;
 *    } else {
 *        return -1;
 *    }
 *}
 */

rbtree_map_t * rbtree_map_alloc(allocator_t *allocator);
int rbtree_map_init(rbtree_map_t *map);
rbtree_map_pos_t * rbtree_map_pos_next(rbtree_map_pos_t *it,rbtree_map_pos_t *next);
rbtree_map_pos_t* rbtree_map_pos_prev(rbtree_map_pos_t *it,rbtree_map_pos_t *prev);
int rbtree_map_pos_equal(rbtree_map_pos_t *it1,rbtree_map_pos_t *it2);
void *rbtree_map_pos_get_pointer(rbtree_map_pos_t *it);
void *rbtree_map_pos_get_kpointer(rbtree_map_pos_t *it);
rbtree_map_pos_t * rbtree_map_begin(rbtree_map_t *map, rbtree_map_pos_t *begin);
rbtree_map_pos_t * rbtree_map_end(rbtree_map_t *map, rbtree_map_pos_t *end);
int rbtree_map_insert(rbtree_map_t *map,void *key,void *value);
int rbtree_map_delete(rbtree_map_t *map, rbtree_map_pos_t *it);
int rbtree_map_remove(rbtree_map_t *map, rbtree_map_pos_t *it);
int rbtree_map_search(rbtree_map_t *map, void *key, rbtree_map_pos_t *it);
int rbtree_map_destroy(rbtree_map_t *map);
int rbtree_map_get_key_len(rbtree_map_t *map);

#endif
