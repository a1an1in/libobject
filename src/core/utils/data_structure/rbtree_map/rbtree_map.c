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
#include <libobject/core/utils/data_structure/rbtree_map.h>
#include <libobject/core/try.h>

struct rbtree_map_node * 
__rbtree_map_search(rbtree_map_t *map, struct rb_root_s *root, void *key)
{
    struct rb_node_s *node     = root->rb_node;
    key_cmp_fpt key_cmp_func = map->key_cmp_func;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;

        if (map->key_type == 1) {
            dbg_str(RBTMAP_DETAIL, "key1:%s", key);
            dbg_str(RBTMAP_DETAIL, "key2:%s", mnode->key);
        }
        result = key_cmp_func(key, mnode->key);
        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else {
            return mnode;
        }
    }
    return NULL;
}

int 
__rbtree_map_insert(rbtree_map_t *map, struct rb_root_s *root, 
                    struct rbtree_map_node *new_mnode)
{
    struct rb_node_s **new     = &(root->rb_node), *parent = NULL;
    key_cmp_fpt key_cmp_func = map->key_cmp_func;
    int result, len;
    struct rbtree_map_node *this;

    /* Figure out where to put new node */
    while (*new) {
        this  = rb_entry(*new, struct rbtree_map_node, node);
        result = key_cmp_func(new_mnode->key, this->key);

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else {
            if (map->enable_same_key)
                new = &((*new)->rb_left);
            else {
                dbg_str(RBTMAP_WARN, "rbtree_map_insert same key");
                return 0;
            }
        }
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&new_mnode->node, parent, new);
    rb_insert_color(&new_mnode->node, root);

    return 1;
}

rbtree_map_pos_t* 
rbtree_map_pos_init(rbtree_map_pos_t *pos, 
                    struct rb_node_s *node, 
                    struct rb_root_s *tree_root, 
                    rbtree_map_t *map)
{
    pos->tree_root = tree_root;
    pos->rb_node_p = node;
    pos->map       = map;

    return pos;
}

rbtree_map_pos_t * 
rbtree_map_pos_next(rbtree_map_pos_t *it, rbtree_map_pos_t *next)
{
    struct rb_node_s *node;

    if (it->rb_node_p == NULL) {
        node = NULL;
    } else {
        node = rb_next(it->rb_node_p);
    }
    return rbtree_map_pos_init(next, node, it->tree_root, it->map);
}

rbtree_map_pos_t* 
rbtree_map_pos_prev(rbtree_map_pos_t *it, rbtree_map_pos_t *prev)
{
    struct rb_node_s *node;

    if (it->rb_node_p == NULL) {
        node = NULL;
    } else {
        node = rb_prev(it->rb_node_p);
    }
    return rbtree_map_pos_init(prev, node, it->tree_root, it->map);
}

int rbtree_map_pos_equal(rbtree_map_pos_t *it1, rbtree_map_pos_t *it2)
{
    return it1->rb_node_p == it2->rb_node_p;
}

void *rbtree_map_pos_get_pointer(rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;

    if (it->rb_node_p == NULL) return NULL;

    mnode = rb_entry(it->rb_node_p, struct rbtree_map_node, node);

    return mnode->value;
}

void *rbtree_map_pos_get_kpointer(rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;

    if (it == NULL || it->rb_node_p == NULL) return NULL;

    mnode = rb_entry(it->rb_node_p, struct rbtree_map_node, node);

    return mnode->key;
}

rbtree_map_t * 
rbtree_map_alloc(allocator_t *allocator)
{
    rbtree_map_t *map;

    map = (rbtree_map_t *)allocator_mem_alloc(allocator, sizeof(rbtree_map_t));
    if (map == NULL) {
        dbg_str(RBTMAP_ERROR, "rbtree_map_create err");
        return NULL;
    }
    memset(map, 0, sizeof(rbtree_map_t)); 
    map->allocator = allocator;
    /*
     *map->lock_type = 0;
     */
    map->enable_same_key = 1;
    map->count = 0;

    dbg_str(RBTMAP_DETAIL, "rbtree_map_create suc");

    return map;
}

int rbtree_map_set(rbtree_map_t *map, char *attrib, void *value)
{
    if (!strcmp(attrib, "lock_type")) {
        map->lock_type = *((uint8_t *)value);
    } else if (!strcmp(attrib, "key_type")) {
        map->key_type = *((uint8_t *)value);
    } else if (!strcmp(attrib, "key_cmp_func")) {
        map->key_cmp_func = (key_cmp_fpt)value;
    } else {
        dbg_str(HMAP_WARN, "not support attrib setting, please check");
        return -1;
    }
}

int rbtree_map_init(rbtree_map_t *map)
{
    struct rb_root_s *tree_root;

    dbg_str(RBTMAP_DETAIL, "rbtree_map init");

    if (map->key_cmp_func == NULL) {
        map->key_cmp_func = default_key_cmp_func;
        dbg_str(RBTMAP_DETAIL, "set default key cmp func:%p", map->key_cmp_func);
    }

    tree_root = (struct rb_root_s *)allocator_mem_alloc(map->allocator, 
                                                      sizeof(struct rb_root_s));
    tree_root->rb_node = NULL;
    map->tree_root     = tree_root;
    rbtree_map_pos_init(&map->end, NULL, tree_root, map);
    rbtree_map_pos_init(&map->begin, NULL, tree_root, map);

    return 0;
}

rbtree_map_pos_t * 
rbtree_map_begin(rbtree_map_t *map, rbtree_map_pos_t *begin)
{
    return rbtree_map_pos_init(begin, 
                               map->begin.rb_node_p, 
                               map->tree_root, map);
}

rbtree_map_pos_t * 
rbtree_map_end(rbtree_map_t *map, rbtree_map_pos_t *end)
{
    return rbtree_map_pos_init(end, 
                               map->end.rb_node_p, 
                               map->tree_root, 
                               map);
}

int rbtree_map_insert(rbtree_map_t *map, void *key, void *value)
{
    struct rbtree_map_node *mnode;
    struct rb_root_s *tree_root = map->tree_root;
    int ret;

    dbg_str(RBTMAP_DETAIL, "rbtree_map_insert");

    mnode = (struct rbtree_map_node *)
            allocator_mem_alloc(map->allocator, 
                                sizeof(struct rbtree_map_node));
    if (mnode == NULL) {
        dbg_str(RBTMAP_ERROR, "rbtree_map_insert, malloc err");
        return -1;
    }

    mnode->key = key;
    mnode->value = value;
    mnode->key_type = map->key_type;

    sync_lock(&map->map_lock, NULL);
    ret = __rbtree_map_insert(map, tree_root, mnode);
    if (ret <= 0) {
        sync_unlock(&map->map_lock);
        return ret;
    }

    map->begin.rb_node_p = rb_first(tree_root);
    map->count++;
    sync_unlock(&map->map_lock);

    return 1;
}

int rbtree_map_delete(rbtree_map_t *map, rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;
    struct rb_node_s *rb_node_p = it->rb_node_p;
    struct rb_root_s *tree_root = map->tree_root;

    mnode = rb_entry(rb_node_p, struct rbtree_map_node, node);

    dbg_str(RBTMAP_DETAIL, "delete node");

    sync_lock(&map->map_lock, NULL);
    if (rbtree_map_pos_equal(it, &map->begin)) {
        rbtree_map_pos_init(&map->begin, rb_next(rb_node_p), tree_root, map);
    }
    rb_erase(rb_node_p, tree_root);
    map->count--;
    sync_unlock(&map->map_lock);

    if (mnode != NULL) {
        allocator_mem_free(map->allocator, mnode);
        mnode = NULL;
    }
    return 1;
}

int rbtree_map_remove(rbtree_map_t *map, rbtree_map_pos_t *it, void **element)
{
    struct rbtree_map_node *mnode;
    struct rb_node_s *rb_node_p = it->rb_node_p;
    struct rb_root_s *tree_root = map->tree_root;

    mnode = rb_entry(rb_node_p, struct rbtree_map_node, node);

    dbg_str(RBTMAP_DETAIL, "delete node");

    sync_lock(&map->map_lock, NULL);
    if (rbtree_map_pos_equal(it, &map->begin)) {
        rbtree_map_pos_init(&map->begin, rb_next(rb_node_p), tree_root, map);
    }
    rb_erase(rb_node_p, tree_root);
    map->count--;
    sync_unlock(&map->map_lock);

    if (element != NULL) {
        *element = mnode->value;
    }

    if (mnode != NULL) {
        allocator_mem_free(map->allocator, mnode);
        mnode = NULL;
    }
    return 1;
}

int
rbtree_map_search(rbtree_map_t *map, void *key, rbtree_map_pos_t *it)
{
    struct rbtree_map_node *mnode;
    struct rb_root_s *tree_root = map->tree_root;

    sync_lock(&map->map_lock, NULL);
    mnode = __rbtree_map_search(map, tree_root, key);
    sync_unlock(&map->map_lock);

    if (mnode == NULL) {
        dbg_str(RBTMAP_DETAIL, "not found the key, key addr:%p", key);
        return 0;
    } else {
        it->rb_node_p = &mnode->node;
    }
    it->map = map;

    return 1;
}

int rbtree_map_count(rbtree_map_t *map)
{
    uint32_t count;

    sync_lock(&map->map_lock, NULL);
    count = map->count;
    sync_unlock(&map->map_lock);

    return count;
}

int rbtree_map_destroy(rbtree_map_t *map)
{
    rbtree_map_pos_t it, end;
    struct rb_root_s *tree_root = map->tree_root;

    dbg_str(RBTMAP_DETAIL, "rbtree_map_destroy");

    for(rbtree_map_begin(map, &it); 
        !rbtree_map_pos_equal(&it, rbtree_map_end(map, &end));
        rbtree_map_begin(map, &it)) {
        rbtree_map_delete(map, &it);
    }
    if (rbtree_map_pos_equal(&map->end, &map->begin)) {
        dbg_str(RBTMAP_INFO, "rbtree_map_destroy, rbtree_map is NULL");
        allocator_mem_free(map->allocator, tree_root);
    }

    allocator_mem_free(map->allocator, map);
    dbg_str(RBTMAP_DETAIL, "rbtree_map_destroy end");

    return 1;
}