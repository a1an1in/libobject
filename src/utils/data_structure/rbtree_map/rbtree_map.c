/*
 * =====================================================================================
 *
 *       Filename:  rbtree_map.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/data_structure/rbtree_map.h>

static int default_key_cmp_func(void *key1,void *key2,uint32_t size)
{
    return memcmp(key1,key2,size);
}

struct rbtree_map_node * 
__rbtree_map_search(rbtree_map_t *map,struct rb_root *root, void *key)
{
    struct rb_node *node = root->rb_node;
    key_cmp_fpt key_cmp_func = map->key_cmp_func;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;

        //result = strcmp(key, mnode->key);
        result = key_cmp_func(key, mnode->key,mnode->value_pos);

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return mnode;
    }
    return NULL;
}

int 
__rbtree_map_insert(rbtree_map_t *map,struct rb_root *root, 
        struct rbtree_map_node *new_mnode)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;
    key_cmp_fpt key_cmp_func = map->key_cmp_func;
    int result;
    struct rbtree_map_node *this;

    /* Figure out where to put new node */
    while (*new) {
        this = rb_entry(*new, struct rbtree_map_node, node);
        result = key_cmp_func(new_mnode->key, this->key,this->value_pos);

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&new_mnode->node, parent, new);
    rb_insert_color(&new_mnode->node, root);

    return 1;
}

rbtree_map_pos_t* 
rbtree_map_pos_init(rbtree_map_pos_t *pos,
        struct rb_node *node,
        struct rb_root *tree_root,
        rbtree_map_t *map)
{
    pos->tree_root = tree_root;
    pos->rb_node_p = node;
    pos->map       = map;

    return pos;
}

rbtree_map_pos_t * 
rbtree_map_pos_next(rbtree_map_pos_t *it,rbtree_map_pos_t *next)
{
    struct rb_node *node;

    if(it->rb_node_p == NULL){
        node = NULL;
    }else{
        node = rb_next(it->rb_node_p);
    }
    return rbtree_map_pos_init(next, node, it->tree_root, it->map);
}

rbtree_map_pos_t* 
rbtree_map_pos_prev(rbtree_map_pos_t *it,rbtree_map_pos_t *prev)
{
    struct rb_node *node;

    if(it->rb_node_p == NULL){
        node = NULL;
    }else{
        node = rb_prev(it->rb_node_p);
    }
    return rbtree_map_pos_init(prev, node, it->tree_root, it->map);
}

int rbtree_map_pos_equal(rbtree_map_pos_t *it1,rbtree_map_pos_t *it2)
{
    return it1->rb_node_p == it2->rb_node_p;
}

void *rbtree_map_pos_get_pointer(rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;

    mnode = rb_entry(it->rb_node_p,struct rbtree_map_node,node);

    return &mnode->key[mnode->value_pos];
}

rbtree_map_t * 
rbtree_map_alloc(allocator_t *allocator)
{
    rbtree_map_t *map;
    map = (rbtree_map_t *)allocator_mem_alloc(allocator,sizeof(rbtree_map_t));
    if(map == NULL){
        dbg_str(DBG_ERROR,"rbtree_map_create err");
        return NULL;
    }
    memset(map,0, sizeof(rbtree_map_t)); 
    map->allocator = allocator;
    /*
     *map->lock_type = lock_type;
     */
    dbg_str(DBG_DETAIL,"hash_map_create suc");
    
    return map;
}

int rbtree_map_init(rbtree_map_t *map,
                    uint32_t key_size,
                    uint32_t value_size)
{
    struct rb_root *tree_root;

    dbg_str(DBG_DETAIL,"rbtree_map init");

    /*
     *strcpy(ct->name,"rbtree_map container");
     */
    map->pair = create_pair(key_size,value_size);
    if(map->pair == NULL) {
        dbg_str(DBG_ERROR,"hash_map_init,create_pair");
        return -1;
    }

    if(map->key_cmp_func == NULL){
        map->key_cmp_func = default_key_cmp_func;
    }
    map->data_size = key_size + value_size;
    map->key_size = key_size;

    tree_root = (struct rb_root *)allocator_mem_alloc(map->allocator,
                                                      sizeof(struct rb_root));
    tree_root->rb_node = NULL;
    map->tree_root = tree_root;
    rbtree_map_pos_init(&map->end,NULL,tree_root,map);
    rbtree_map_pos_init(&map->begin,NULL,tree_root,map);

    return 0;
}

rbtree_map_pos_t * 
rbtree_map_begin(rbtree_map_t *map, rbtree_map_pos_t *begin)
{
    return rbtree_map_pos_init(begin,
                               map->begin.rb_node_p,
                               map->tree_root,map);
}

rbtree_map_pos_t * rbtree_map_end(rbtree_map_t *map, rbtree_map_pos_t *end)
{
    return rbtree_map_pos_init(end,
                               map->end.rb_node_p,
                               map->tree_root,
                               map);
}

void rbtree_map_make_pair(rbtree_map_t *map,void *key,void *value)
{
    sync_lock(&map->map_lock,NULL);
    make_pair(map->pair,key,value);
    sync_unlock(&map->map_lock);
}

int rbtree_map_insert_data(rbtree_map_t *map, void *value)
{
    struct rbtree_map_node *mnode;
    struct rb_root *tree_root = map->tree_root;
    uint32_t data_size = map->data_size;
    uint32_t key_size = map->key_size;

    dbg_str(DBG_DETAIL,"rbtree_map_insert");

    mnode = (struct rbtree_map_node *)
            allocator_mem_alloc(map->allocator,
                                sizeof(struct rbtree_map_node) + data_size);
    if(mnode == NULL){
        dbg_str(DBG_ERROR,"rbtree_map_insert,malloc err");
        return -1;
    }

    memcpy(mnode->key,value,data_size);
    mnode->value_pos = key_size;

    sync_lock(&map->map_lock,NULL);
    __rbtree_map_insert(map,tree_root, mnode);

    map->begin.rb_node_p = rb_first(tree_root);
    sync_unlock(&map->map_lock);

    return 0;
}

int rbtree_map_insert(rbtree_map_t *map,void *key,void *value)
{
    rbtree_map_make_pair(map,key,value);

    return  rbtree_map_insert_data(map,map->pair->data);
}

int rbtree_map_delete(rbtree_map_t *map, rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;
    struct rb_node *rb_node_p = it->rb_node_p;
    struct rb_root *tree_root = map->tree_root;

    mnode = rb_entry(rb_node_p,struct rbtree_map_node,node);

    dbg_str(DBG_DETAIL,"delete node");

    sync_lock(&map->map_lock,NULL);
    if(rbtree_map_pos_equal(it,&map->begin)){
        rbtree_map_pos_init(&map->begin,rb_next(rb_node_p),tree_root,map);
    }
    rb_erase(rb_node_p, tree_root);
    sync_unlock(&map->map_lock);

    if (mnode != NULL) {
        allocator_mem_free(map->allocator,mnode);
        mnode = NULL;
    }
    return 0;
}

rbtree_map_pos_t *
rbtree_map_search(rbtree_map_t *map, void *key, rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;
    struct rb_root *tree_root = map->tree_root;

    dbg_str(DBG_DETAIL,"rbtree_map_search");

    sync_lock(&map->map_lock,NULL);
    mnode = __rbtree_map_search(map,tree_root, key);
    sync_unlock(&map->map_lock);

    if(mnode == NULL){
        it->rb_node_p = NULL;
    }else{
        it->rb_node_p = &mnode->node;
    }
    it->map = map;

    return it;
}

int rbtree_map_destroy(rbtree_map_t *map)
{
    rbtree_map_pos_t it,end;
    struct rb_root *tree_root = map->tree_root;

    dbg_str(DBG_DETAIL,"rbtree_map_destroy");

    destroy_pair(map->pair);

    for(    rbtree_map_begin(map,&it); 
            !rbtree_map_pos_equal(&it,rbtree_map_end(map,&end));
            rbtree_map_begin(map,&it)) 
    {
        rbtree_map_delete(map,&it);
    }
    if(rbtree_map_pos_equal(&map->end,&map->begin)){
        dbg_str(DBG_WARNNING,"rbtree_map_destroy,rbtree_map is NULL");
        allocator_mem_free(map->allocator,tree_root);
    }

    allocator_mem_free(map->allocator,map);
}

