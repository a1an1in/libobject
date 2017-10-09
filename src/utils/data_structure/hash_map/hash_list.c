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
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/data_structure/hash_list.h"

static inline int default_key_cmp_func(void *key1,void *key2,uint32_t size)
{
    return memcmp(key1,key2,size);
}

static inline uint32_t default_hash_func(void *key,uint32_t key_size,uint32_t bucket_size)
{
    uint32_t sum = 0;
    uint32_t i = 0;
    for(i = 0; i < key_size; i++){
        sum += *(uint8_t *)(key + i);
    }
    return sum % bucket_size;
}

hash_map_t * hash_map_alloc(allocator_t *allocator)
{
    hash_map_t *map;

    dbg_str(HMAP_DETAIL,"hash_map_create");

    map = (hash_map_t *)allocator_mem_alloc(allocator,sizeof(hash_map_t));
    if(map == NULL){
        dbg_str(DBG_ERROR,"allocator_mem_alloc(map->allocator err");
        return NULL;
    }
    memset(map,0,sizeof(hash_map_t));
    map->allocator = allocator;

    return map;
}

int hash_map_set(hash_map_t *hmap,char *attrib,char *value)
{
    if(!strcmp(attrib,"lock_type")){
        hmap->lock_type = atoi((char *)value);
    } else {
        dbg_str(DBG_WARNNING,"not support attrib setting,please check");
        return -1;
    }
}

int hash_map_init(hash_map_t *hmap,
                  uint32_t key_size,
                  uint32_t value_size,
                  uint32_t bucket_size)
{
    dbg_str(HMAP_DETAIL,"hash_map_init");

    hash_map_t *map   = hmap;
    map->data_size    = value_size + key_size; 
    map->key_size     = key_size;
    map->bucket_size  = bucket_size;
    map->node_count   = 0;

    map->pair = create_pair(key_size,value_size);
    if(map->pair == NULL) {
        dbg_str(DBG_ERROR,"hash_map_init,create_pair");
        return -1;
    }

    if(map->bucket_size == 0) {
        hmap->bucket_size = 20;
        bucket_size = hmap->bucket_size;
    }
    if(hmap->key_cmp_func == NULL){
        hmap->key_cmp_func = default_key_cmp_func;
    }

    if(hmap->hash_func == NULL){
        hmap->hash_func = default_hash_func;
    }

    map->hlist = allocator_mem_alloc(map->allocator,
                                     sizeof(struct hlist_head)*bucket_size);
    if(map->hlist == NULL){
        dbg_str(DBG_ERROR,"hash map init");
        return -1;
    }
    memset(map->hlist,0,sizeof(struct hlist_head)*bucket_size);

    hash_map_pos_init(&map->begin,NULL,0,map->hlist,map);
    hash_map_pos_init(&map->end,NULL,bucket_size - 1,map->hlist,map);

    sync_lock_init(&map->map_lock,map->lock_type);
    
    return 0;
}

void hash_map_make_pair(hash_map_t *hmap,void *key,void *value)
{
    sync_lock(&hmap->map_lock,NULL);
    if (hmap->key_type == 0) { /*key is string*/
        make_pair(hmap->pair,key,value);
    } else {
        dbg_str(DBG_DETAIL,"hash_map_make_pair with key_type");
        make_pair_with_fixed_key_len(hmap->pair,key,hmap->key_size, value);
    }
    sync_unlock(&hmap->map_lock);
}

int hash_map_insert_data(hash_map_t *hmap,void *data)
{
    struct hash_map_node *mnode;
    uint32_t bucket_pos;
    uint32_t data_size       = hmap->data_size;
    uint32_t key_size        = hmap->key_size;
    struct hlist_head *hlist = hmap->hlist;
    hash_func_fpt hash_func  = hmap->hash_func;
    uint32_t bucket_size     = hmap->bucket_size;;
    hash_map_pos_t *begin_pos = &hmap->begin;;
    int ret;

    mnode = (struct hash_map_node *)allocator_mem_alloc(hmap->allocator,
                                                        sizeof(struct hash_map_node) + data_size);
    if(mnode == NULL){
        dbg_str(DBG_ERROR,"hash_map_insert,allocator_mem_alloc(map->allocator err");
        return -1;
    }

    memcpy(mnode->key,data,data_size);
    mnode->value_pos = key_size;
    mnode->data_size = data_size;

    INIT_HLIST_NODE(&mnode->hlist_node);

    bucket_pos = hash_func(mnode->key,key_size,bucket_size); 
    assert(bucket_pos <= bucket_size);

    sync_lock(&hmap->map_lock,NULL);

    hlist_add_head(&mnode->hlist_node, &hlist[bucket_pos]);
    if(begin_pos->hlist_node_p == NULL || bucket_pos <= begin_pos->bucket_pos){
        hash_map_pos_init(&hmap->begin, hlist[bucket_pos].first, bucket_pos, hlist,hmap);
        /*
         *dbg_str(DBG_WARNNING,"change begin pos");
         */
    }
    ret = hmap->node_count++;
    dbg_str(HMAP_IMPORTANT,"hash_map_insert,node_count=%d,bucket_pos =%d,insert_hash_node_pos=%p",
            hmap->node_count,
            bucket_pos,
            &mnode->hlist_node);
    /*
     *dbg_str(HMAP_IMPORTANT,"hash_map_insert,node_count=%d,bucket_pos =%d,first =%p,next=%p,begin.hash_map_pos=%p",
     *        hmap->node_count,
     *        bucket_pos,
     *        hlist[bucket_pos].first,
     *        hlist[bucket_pos].first->next,
     *        hmap->begin.hlist_node_p);
     */

    sync_unlock(&hmap->map_lock);

    return ret;
}

/*
 * hash map insert data with writing back
 * */
int hash_map_insert_data_wb(hash_map_t *hmap,void *data, hash_map_pos_t *out)
{
    struct hash_map_node *mnode;
    uint32_t bucket_pos;
    uint32_t data_size       = hmap->data_size;
    uint32_t key_size        = hmap->key_size;
    struct hlist_head *hlist = hmap->hlist;
    hash_func_fpt hash_func  = hmap->hash_func;
    uint32_t bucket_size     = hmap->bucket_size;;
    hash_map_pos_t *begin_pos = &hmap->begin;;
    int ret;

    mnode = (struct hash_map_node *)allocator_mem_alloc(hmap->allocator,
                                                        sizeof(struct hash_map_node) + data_size);
    if(mnode == NULL){
        dbg_str(DBG_ERROR,"hash_map_insert,allocator_mem_alloc(map->allocator err");
        return -1;
    }

    memcpy(mnode->key,data,data_size);
    mnode->value_pos = key_size;
    mnode->data_size = data_size;

    INIT_HLIST_NODE(&mnode->hlist_node);

    bucket_pos = hash_func(mnode->key,key_size,bucket_size); 
    assert(bucket_pos <= bucket_size);

    sync_lock(&hmap->map_lock,NULL);

    hlist_add_head(&mnode->hlist_node, &hlist[bucket_pos]);
    if(begin_pos->hlist_node_p == NULL || bucket_pos <= begin_pos->bucket_pos){
        hash_map_pos_init(&hmap->begin, hlist[bucket_pos].first, bucket_pos, hlist,hmap);
        /*
         *dbg_str(DBG_WARNNING,"change begin pos");
         */
    }
    ret = hmap->node_count++;
    dbg_str(HMAP_IMPORTANT,"hash_map_insert,node_count=%d,bucket_pos =%d,insert_hash_node_pos=%p",
            hmap->node_count,
            bucket_pos,
            &mnode->hlist_node);
    /*
     *dbg_str(HMAP_IMPORTANT,"hash_map_insert,node_count=%d,bucket_pos =%d,first =%p,next=%p,begin.hash_map_pos=%p",
     *        hmap->node_count,
     *        bucket_pos,
     *        hlist[bucket_pos].first,
     *        hlist[bucket_pos].first->next,
     *        hmap->begin.hlist_node_p);
     */

    sync_unlock(&hmap->map_lock);

    hash_map_pos_init(out, &mnode->hlist_node, bucket_pos, hlist,hmap);

    return ret;
}

int hash_map_insert(hash_map_t *hmap,void *key,void *value)
{
    hash_map_make_pair(hmap,key,value);

    return  hash_map_insert_data(hmap,hmap->pair->data);
}

int hash_map_insert_wb(hash_map_t *hmap,void *key,void *value, hash_map_pos_t *out)
{
    hash_map_make_pair(hmap,key,value);

    return  hash_map_insert_data_wb(hmap,hmap->pair->data,out);
}

int hash_map_search(hash_map_t *hmap, void *key,hash_map_pos_t *ret)
{
    struct hash_map_node *mnode = NULL;
    uint32_t bucket_pos;
    uint32_t key_size        = hmap->key_size;
    struct hlist_head *hlist = hmap->hlist;
    hash_func_fpt hash_func  = hmap->hash_func;
    key_cmp_fpt key_cmp_func = hmap->key_cmp_func;
    uint32_t bucket_size     = hmap->bucket_size;;
    struct hlist_node *pos,*next;

    if(strlen(key) < key_size) {
        key_size = strlen(key);
    }

    bucket_pos = hash_func(key,key_size,bucket_size); 
    assert(bucket_pos <= bucket_size);

    dbg_str(HMAP_DETAIL,"hash_map_search,bucket_pos=%d",bucket_pos);

    sync_lock(&hmap->map_lock,NULL);
    if(key_cmp_func == NULL){
        return -1;
    } 
    hlist_for_each_safe(pos, next, &hlist[bucket_pos]){
        mnode = container_of(pos,struct hash_map_node,hlist_node);
        if(!key_cmp_func(mnode->key,key,key_size)){
            sync_unlock(&hmap->map_lock);
            dbg_str(HMAP_IMPORTANT,"found key:%s",key);
            hash_map_pos_init(ret, pos, bucket_pos, hlist,hmap);
            return 1;
        }
    }
    sync_unlock(&hmap->map_lock);
    dbg_str(HMAP_IMPORTANT,"not found key");
    return -1;
}

int hash_map_delete(hash_map_t *hmap, hash_map_pos_t *pos)
{
    struct hash_map_node *mnode;
    struct hlist_head *hlist = pos->hlist;
    hash_map_pos_t next;
    void **addr;

    /*
     *dbg_str(HMAP_IMPORTANT,"del hash_map ,bucket_pos:%d,cur node:%p,begin node:%p",
     *        pos->bucket_pos, pos->hlist_node_p, hmap->begin.hlist_node_p);
     */

    sync_lock(&hmap->map_lock,NULL);

    if(hash_map_pos_equal(pos,&hmap->begin)){
        /*
         *dbg_str(DBG_WARNNING,"del iter equal begain");
         */
        hash_map_pos_next(pos,&next);
        hash_map_pos_init(&hmap->begin,
                          next.hlist_node_p, 
                          next.bucket_pos, hlist, hmap);
    }
    hlist_del(pos->hlist_node_p);
    hmap->node_count--;
    dbg_str(HMAP_IMPORTANT,"del hash_map ,hash map count=%d,del node pos=%p",hmap->node_count,pos->hlist_node_p);

    sync_unlock(&hmap->map_lock);

    mnode = container_of(pos->hlist_node_p,struct hash_map_node,hlist_node);
    if (mnode != NULL) {
        allocator_mem_free(hmap->allocator, mnode);
        mnode = NULL;
    }
    return 0;
}

int hash_map_remove(hash_map_t *hmap, hash_map_pos_t *pos, void **data)
{
    struct hash_map_node *mnode;
    struct hlist_head *hlist = pos->hlist;
    hash_map_pos_t next;
    void **addr;

    sync_lock(&hmap->map_lock,NULL);

    if(hash_map_pos_equal(pos,&hmap->begin)){
        hash_map_pos_next(pos,&next);
        hash_map_pos_init(&hmap->begin,
                          next.hlist_node_p, 
                          next.bucket_pos, hlist, hmap);
    }
    hlist_del(pos->hlist_node_p);
    hmap->node_count--;
    dbg_str(HMAP_IMPORTANT,"del hash_map ,hash map count=%d,del node pos=%p",hmap->node_count,pos->hlist_node_p);

    sync_unlock(&hmap->map_lock);

    mnode = container_of(pos->hlist_node_p,struct hash_map_node,hlist_node);
    if (mnode != NULL) {
        *data = (void *)&mnode->key[mnode->value_pos];
        allocator_mem_free(hmap->allocator, mnode);
        mnode = NULL;
    }
    return 0;
}

int hash_map_destroy(hash_map_t *hmap)
{
     hash_map_pos_t it;

     dbg_str(HMAP_DETAIL,"hash_map_destroy");

     destroy_pair(hmap->pair);

     for(    hash_map_begin(hmap,&it);
             !hash_map_pos_equal(&it,&hmap->end);
             hash_map_begin(hmap,&it))
     {
         dbg_str(HMAP_DETAIL,"destroy node:%p",it.hlist_node_p);
         hash_map_delete(hmap,&it);
     }

     if(hash_map_pos_equal(&hmap->end,&it)){
         dbg_str(HMAP_IMPORTANT,"hash_map_destroy,hash_map is NULL");
         sync_lock_destroy(&hmap->map_lock);
         allocator_mem_free(hmap->allocator,hmap->hlist);
     }

     allocator_mem_free(hmap->allocator,hmap);

     return 0;
}

int hash_map_pos_next(hash_map_pos_t *pos,hash_map_pos_t *next)
{
    struct hlist_node *hlist_node_p;
    uint32_t bucket_pos      = pos->bucket_pos;
    struct hlist_head *hlist = pos->hlist;
    hash_map_t *hmap         = pos->hmap;
    uint32_t bucket_size     = hmap->bucket_size;
    int ret = 0;

    if(pos->hlist_node_p == NULL){
        dbg_str(DBG_WARNNING,"hlist_node_p is nULL");
        return -1;
    }

    if(pos->hlist_node_p->next){
        hlist_node_p = pos->hlist_node_p->next;
        ret          = hash_map_pos_init(next, hlist_node_p, bucket_pos, hlist, hmap);
        dbg_str(HMAP_DETAIL,"find next iter ,list next is not null,bucket_pos=%d,node:%p,next:%p",
                bucket_pos,hlist_node_p,hlist_node_p->next);
        return ret;
    } else if(bucket_pos < bucket_size - 1 ){
        for(++bucket_pos; bucket_pos < bucket_size;bucket_pos++){
            if(hlist[bucket_pos].first){
                hlist_node_p = hlist[bucket_pos].first;
                ret          = hash_map_pos_init(next, hlist_node_p, bucket_pos, hlist, hmap);
                dbg_str(HMAP_DETAIL,"find new head,bucket_pos=%d,node:%p,next:%p",
                        bucket_pos,hlist_node_p,hlist_node_p->next);
                return ret;
            }
        }
        hash_map_pos_init(next, NULL, bucket_size -1, hlist, hmap);
    } else if(bucket_pos == bucket_size -1 ){
        dbg_str(HMAP_IMPORTANT,"container is null");
        hash_map_pos_init(next, NULL, bucket_pos, hlist, hmap);
    }else{
        dbg_str(DBG_ERROR,"hash_map_iterator_next err");
    }

    return ret;
}

void hash_map_print_mnode(struct hash_map_node *mnode)
{
    dbg_buf(DBG_DETAIL,"data:",mnode->key,mnode->data_size);
}

