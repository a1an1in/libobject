/*
 * =====================================================================================
 *
 *       Filename:  hash_list.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/14/2015 09:06:17 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __HASH_LIST_H__
#define __HASH_LIST_H__

#include <libobject/utils/dbg/debug.h>
#include "libobject/utils/data_structure/hash_list_struct.h"

hash_map_t * hash_map_alloc(allocator_t *allocator);
int hash_map_set(hash_map_t *hmap,char *attrib,char *value);
int hash_map_init(hash_map_t *hmap,
		          uint32_t key_size,
		          uint32_t value_size,
		          uint32_t bucket_size);

int hash_map_insert_data(hash_map_t *hmap,void *data);
int hash_map_insert_data_wb(hash_map_t *hmap,void *data, hash_map_pos_t *out);
int hash_map_insert(hash_map_t *hmap,void *key,void *value);
int hash_map_insert_wb(hash_map_t *hmap,void *key,void *value, hash_map_pos_t *out);
int hash_map_search(hash_map_t *hmap, void *key,hash_map_pos_t *ret);
int hash_map_delete(hash_map_t *hmap, hash_map_pos_t *pos);
int hash_map_destroy(hash_map_t *hmap);
int hash_map_pos_next(hash_map_pos_t *pos,hash_map_pos_t *next);
void hash_map_print_mnode(struct hash_map_node *mnode);



static inline int
hash_map_pos_init(hash_map_pos_t *pos,
		struct hlist_node *hlist_node_p,
		uint32_t bucket_pos,
		struct hlist_head *hlist_head_p,
		struct hash_map_s *hmap)
{
	pos->hlist_node_p = hlist_node_p;
	pos->bucket_pos   = bucket_pos;
	pos->hlist        = hlist_head_p;
	pos->hmap         = hmap;

	return 0;
}
static inline int 
hash_map_begin(hash_map_t *hmap,hash_map_pos_t *begin)
{
	return hash_map_pos_init(begin, hmap->begin.hlist_node_p,
			hmap->begin.bucket_pos, hmap->begin.hlist, hmap);
}
static inline int 
hash_map_end(hash_map_t *hmap,hash_map_pos_t *end)
{
	return hash_map_pos_init(end, hmap->end.hlist_node_p,
			hmap->end.bucket_pos, hmap->end.hlist, hmap);
}
static inline int hash_map_pos_equal(hash_map_pos_t *pos1,hash_map_pos_t *pos2)
{
	return (pos1->hlist_node_p == pos2->hlist_node_p);
}
static inline void *hash_map_pos_get_pointer(hash_map_pos_t *pos)
{
	struct hash_map_node *mnode;
    void **data;

	mnode = container_of(pos->hlist_node_p,
			struct hash_map_node,
			hlist_node);
	/*
	 *dbg_buf(DBG_DETAIL,"key:",mnode->key,mnode->data_size);
	 */

    data = (void **)&mnode->key[mnode->value_pos];
	return *data;
}

static inline void *hash_map_pos_get_kpointer(hash_map_pos_t *pos)
{
	struct hash_map_node *mnode;

	mnode = container_of(pos->hlist_node_p,
			struct hash_map_node,
			hlist_node);
	/*
	 *dbg_buf(DBG_DETAIL,"key:",mnode->key,mnode->data_size);
	 */

	return mnode->key;
}
static inline void 
hash_map_for_each(struct hash_map_s *hmap,void (*func)(hash_map_pos_t *pos))
{
	hash_map_pos_t pos,next;
	struct hash_map_node *mnode;

	for(	hash_map_begin(hmap,&pos),hash_map_pos_next(&pos,&next); 
			!hash_map_pos_equal(&pos,&hmap->end);
			pos = next,hash_map_pos_next(&pos,&next)){
		func(&pos);
	}
}

#endif
