/*
 * =====================================================================================
 *
 *       Filename:  hash_list.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/12/2015 03:25:59 PM
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
#include <assert.h>
#include "libobject/core/utils/dbg/debug.h"
#include "libobject/core/utils/data_structure/hash_list.h"
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

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

static inline uint32_t 
default_hash_func(void *key, uint32_t bucket_size)
{
    uint32_t k = (uint32_t)key;
    return k % bucket_size;
}

hash_map_t * hash_map_alloc(allocator_t *allocator)
{
    hash_map_t *map;

    dbg_str(HMAP_DETAIL, "hash_map_create");

    map = (hash_map_t *)allocator_mem_alloc(allocator, sizeof(hash_map_t));
    if (map == NULL) {
        dbg_str(HMAP_ERROR, "allocator_mem_alloc(map->allocator err");
        return NULL;
    }
    memset(map, 0, sizeof(hash_map_t));
    map->allocator = allocator;

    return map;
}

int hash_map_set(hash_map_t *hmap, char *attrib, void *value)
{
    if (!strcmp(attrib, "lock_type")) {
        hmap->lock_type = atoi((char *)value);
    } else if (!strcmp(attrib, "key_cmp_func")) {
        hmap->key_cmp_func = (key_cmp_fpt)value;
    } else if (!strcmp(attrib, "hash_func")) {
        hmap->hash_func = (hash_func_fpt)value;
    } else if (!strcmp(attrib, "bucket_size")) {
        hmap->bucket_size = *((uint8_t *)value);
    } else {
        dbg_str(HMAP_WARN, "not support attrib setting, please check");
        return -1;
    }
}

int hash_map_init(hash_map_t *hmap)
{
    dbg_str(HMAP_DETAIL, "hash_map_init");

    hash_map_t *map   = hmap;
    map->node_count   = 0;

    if (map->bucket_size == 0) {
        hmap->bucket_size = 20;
    }
    if (hmap->key_cmp_func == NULL) {
        hmap->key_cmp_func = default_key_cmp_func;
    }

    if (hmap->hash_func == NULL) {
        hmap->hash_func = default_hash_func;
    }
    
    if (map->lock_type == 0) {
        map->lock_type = 0;
    }

    map->hlist = allocator_mem_alloc(map->allocator, 
                                     sizeof(struct hlist_head)* hmap->bucket_size);
    if (map->hlist == NULL) {
        dbg_str(HMAP_ERROR, "hash map init");
        return -1;
    }
    memset(map->hlist, 0, sizeof(struct hlist_head) * hmap->bucket_size);

    hash_map_pos_init(&map->begin, NULL, 0, map->hlist, map);
    hash_map_pos_init(&map->end, NULL, hmap->bucket_size - 1, map->hlist, map);

    sync_lock_init(&map->map_lock, map->lock_type);
    
    return 1;
}

int hash_map_insert(hash_map_t *hmap, void *key, void *value)
{
    struct hash_map_node *mnode;
    uint32_t bucket_pos;
    struct hlist_head *hlist  = hmap->hlist;
    hash_func_fpt hash_func   = hmap->hash_func;
    uint32_t bucket_size      = hmap->bucket_size;;
    hash_map_pos_t *begin_pos = &hmap->begin;;
    int ret;

    mnode = (struct hash_map_node *)
            allocator_mem_alloc(hmap->allocator, 
                                sizeof(struct hash_map_node));
    if (mnode == NULL) {
        dbg_str(HMAP_ERROR, 
                "hash_map_insert, allocator_mem_alloc(map->allocator err");
        return -1;
    }

    mnode->key = key;
    mnode->value = value;

    INIT_HLIST_NODE(&mnode->hlist_node);

    bucket_pos = hash_func(mnode->key, bucket_size); 
    assert(bucket_pos <= bucket_size);

    sync_lock(&hmap->map_lock, NULL);

    hlist_add_head(&mnode->hlist_node, &hlist[bucket_pos]);
    if (begin_pos->hlist_node_p == NULL || bucket_pos <= begin_pos->bucket_pos) {
        hash_map_pos_init(&hmap->begin, hlist[bucket_pos].first, bucket_pos, hlist, hmap);
    }
    ret = hmap->node_count++;
    dbg_str(HMAP_INFO,
            "hash_map_insert, node_count=%d, bucket_pos =%d, insert_hash_node_pos=%p", 
            hmap->node_count, 
            bucket_pos, 
            &mnode->hlist_node);

    sync_unlock(&hmap->map_lock);

    return ret;
}


int hash_map_search(hash_map_t *hmap, void *key, hash_map_pos_t *ret)
{
    struct hash_map_node *mnode = NULL;
    uint32_t bucket_pos;
    struct hlist_head *hlist = hmap->hlist;
    hash_func_fpt hash_func  = hmap->hash_func;
    key_cmp_fpt key_cmp_func = hmap->key_cmp_func;
    uint32_t bucket_size     = hmap->bucket_size;;
    struct hlist_node *pos, *next;


    bucket_pos = hash_func(key, bucket_size); 
    assert(bucket_pos <= bucket_size);

    dbg_str(HMAP_DETAIL, "hash_map_search, bucket_pos=%d", bucket_pos);

    sync_lock(&hmap->map_lock, NULL);
    if (key_cmp_func == NULL) {
        return -1;
    } 
    hlist_for_each_safe(pos, next, &hlist[bucket_pos]) {
        mnode = container_of(pos, struct hash_map_node, hlist_node);
        if (!key_cmp_func(mnode->key, key)) {
            sync_unlock(&hmap->map_lock);
            dbg_str(HMAP_INFO, "found key:%s", key);
            hash_map_pos_init(ret, pos, bucket_pos, hlist, hmap);
            return 1;
        }
    }
    sync_unlock(&hmap->map_lock);
    dbg_str(HMAP_INFO, "not found key");

    return -1;
}

int hash_map_delete(hash_map_t *hmap, hash_map_pos_t *pos)
{
    struct hash_map_node *mnode;
    struct hlist_head *hlist = pos->hlist;
    hash_map_pos_t next;
    void **addr;

    sync_lock(&hmap->map_lock, NULL);

    if (hash_map_pos_equal(pos, &hmap->begin)) {
        /*
         *dbg_str(HMAP_WARN, "del iter equal begain");
         */
        hash_map_pos_next(pos, &next);
        hash_map_pos_init(&hmap->begin, 
                          next.hlist_node_p, 
                          next.bucket_pos, hlist, hmap);
    }
    hlist_del(pos->hlist_node_p);
    hmap->node_count--;
    dbg_str(HMAP_INFO, 
            "del hash_map , hash map count=%d, del node pos=%p", 
            hmap->node_count, pos->hlist_node_p);

    sync_unlock(&hmap->map_lock);

    mnode = container_of(pos->hlist_node_p, struct hash_map_node, hlist_node);
    if (mnode != NULL) {
        allocator_mem_free(hmap->allocator, mnode);
        mnode = NULL;
    }

    return 1;
}

int hash_map_remove(hash_map_t *hmap, hash_map_pos_t *pos, void **data)
{
    struct hash_map_node *mnode;
    struct hlist_head *hlist = pos->hlist;
    hash_map_pos_t next;

    sync_lock(&hmap->map_lock, NULL);
    if (hash_map_pos_equal(pos, &hmap->begin)) {
        hash_map_pos_next(pos, &next);
        hash_map_pos_init(&hmap->begin, 
                          next.hlist_node_p, 
                          next.bucket_pos, hlist, hmap);
    }
    hlist_del(pos->hlist_node_p);
    hmap->node_count--;
    dbg_str(HMAP_INFO,
            "del hash_map , hash map count=%d, del node pos=%p", 
            hmap->node_count,
            pos->hlist_node_p);

    sync_unlock(&hmap->map_lock);

    mnode = container_of(pos->hlist_node_p, struct hash_map_node, hlist_node);
    if (mnode != NULL && data != NULL) {
        *data = mnode->value;
    }

    if (mnode != NULL) {
        allocator_mem_free(hmap->allocator, mnode);
        mnode = NULL;
    }

    return 1;
}

int hash_map_destroy(hash_map_t *hmap)
{
    hash_map_pos_t it;

    dbg_str(HMAP_DETAIL, "hash_map_destroy");

    for(hash_map_begin(hmap, &it);
        !hash_map_pos_equal(&it, &hmap->end);
        hash_map_begin(hmap, &it)) {
        dbg_str(HMAP_DETAIL, "destroy node:%p", it.hlist_node_p);
        hash_map_delete(hmap, &it);
    }

    if (hash_map_pos_equal(&hmap->end, &it)) {
        dbg_str(HMAP_INFO, "hash_map_destroy, hash_map is NULL");
        sync_lock_destroy(&hmap->map_lock);
        allocator_mem_free(hmap->allocator, hmap->hlist);
    }

    allocator_mem_free(hmap->allocator, hmap);

    return 1;
}

int hash_map_pos_next(hash_map_pos_t *pos, hash_map_pos_t *next)
{
    struct hlist_node *hlist_node_p;
    uint32_t bucket_pos      = pos->bucket_pos;
    struct hlist_head *hlist = pos->hlist;
    hash_map_t *hmap         = pos->hmap;
    uint32_t bucket_size     = hmap->bucket_size;
    int ret = 0;

    if (pos->hlist_node_p == NULL) {
        dbg_str(HMAP_WARN, "hlist_node_p is nULL");
        return -1;
    }

    if (pos->hlist_node_p->next) {
        hlist_node_p = pos->hlist_node_p->next;
        ret          = hash_map_pos_init(next, hlist_node_p, bucket_pos, hlist, hmap);
        return ret;
    } else if (bucket_pos < bucket_size - 1 ) {
        for (++bucket_pos; bucket_pos < bucket_size;bucket_pos++) {
            if (hlist[bucket_pos].first) {
                hlist_node_p = hlist[bucket_pos].first;
                ret          = hash_map_pos_init(next, hlist_node_p, bucket_pos, hlist, hmap);

                dbg_str(HMAP_DETAIL,
                        "find new head, bucket_pos=%d, node:%p, next:%p", 
                        bucket_pos, hlist_node_p,
                        hlist_node_p->next);

                return ret;
            }
        }
        hash_map_pos_init(next, NULL, bucket_size -1, hlist, hmap);
    } else if (bucket_pos == bucket_size -1 ) {
        dbg_str(HMAP_INFO, "container is null");
        hash_map_pos_init(next, NULL, bucket_pos, hlist, hmap);
    }else{
        dbg_str(HMAP_ERROR, "hash_map_iterator_next err");
    }

    return ret;
}

int hash_map_get_count(hash_map_t *hmap)
{
    int ret;

    sync_lock(&hmap->map_lock, NULL);
    ret = hmap->node_count;
    sync_unlock(&hmap->map_lock);

    return ret;
}
