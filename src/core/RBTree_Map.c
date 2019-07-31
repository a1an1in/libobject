/**
 * @file map.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/Linked_List.h>

static int
__find_same_key_node(rbtree_map_t *map, 
                     struct rb_node_s *node,
                     key_cmp_fpt func,
                     void *key, List *list)
{
    int ret = 0;
    void *value;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;

        result = func(key, mnode->key);
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            value  = mnode->value;
            list->add(list, value);
            __find_same_key_node(map, node->rb_left, func , key, list);
            __find_same_key_node(map, node->rb_right, func,  key, list);
            break;
        }
    }

    return ret;
}

static int __construct(Map *map, char *init_str)
{
    RBTree_Map *rbm = (RBTree_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    rbm->rbmap = rbtree_map_alloc(allocator);
    rbtree_map_init(rbm->rbmap); 

    map->b = OBJECT_NEW(allocator, RBTree_Iterator, NULL);
    map->e = OBJECT_NEW(allocator, RBTree_Iterator, NULL);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(RBTMAP_DETAIL, "hash map deconstruct, map addr:%p", map);
    object_destroy(map->b);
    object_destroy(map->e);
    map->clear(map);
    rbtree_map_destroy(((RBTree_Map *)map)->rbmap);

    return 0;
}

static int __set_cmp_func(Map *map, void *func)
{
    RBTree_Map *rbmap = (RBTree_Map *)map;

    rbtree_map_set(rbmap->rbmap, "key_cmp_func", func);
    return 0;
}

static int __add(Map *map, void *key, void *value)
{
    dbg_str(RBTMAP_DETAIL, "Rbtree Map add");
    RBTree_Map *rbmap = (RBTree_Map *)map;

    return rbtree_map_insert(rbmap->rbmap, key, value);
}

static int __search(Map *map, void *key, void **element)
{
    rbtree_map_pos_t pos;
    int ret;

    dbg_str(RBTMAP_IMPORTANT, "Rbtree Map search");
    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1 && element != NULL) {
        *element = rbtree_map_pos_get_pointer(&pos);
        return ret;
    }

    return ret;
}

static int __search_all_same_key(Map *map, void *key, List *list)
{
    rbtree_map_t *rbtree_map = ((RBTree_Map *)map)->rbmap;
    struct rb_root_s *root = rbtree_map->tree_root;
    struct rb_node_s *node = root->rb_node;
    int ret = 0;
    void **addr;

    dbg_str(RBTMAP_DETAIL, "rbtree_map_search all");

    sync_lock(&rbtree_map->map_lock, NULL);
    key_cmp_fpt key_cmp_func = rbtree_map->key_cmp_func;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;
        void *value;

        result = key_cmp_func(key, mnode->key);
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            value  = mnode->value;
            list->add(list, value);
            __find_same_key_node(rbtree_map, node->rb_left, key_cmp_func , key, list);
            __find_same_key_node(rbtree_map, node->rb_right, key_cmp_func,  key, list);
            break;
        }
    }
    sync_unlock(&rbtree_map->map_lock);

    return ret;
}

static int __remove(Map *map, void *key, void **element)
{
    rbtree_map_pos_t pos;
    int ret;

    dbg_str(RBTMAP_IMPORTANT, "Rbtree Map remove");

    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1) {
        ret = rbtree_map_remove(((RBTree_Map *)map)->rbmap, &pos, element);
    } else {
        dbg_str(RBTMAP_WARNNING, "map remove, not found key :%s", key);
    }

    return ret;
}

static int __del(Map *map, void *key)
{
    rbtree_map_pos_t pos;
    int ret = -1;
    dbg_str(RBTMAP_DETAIL, "Rbtree Map del");

    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1) {
        ret = rbtree_map_delete(((RBTree_Map *)map)->rbmap, &pos);
    }

    return ret;
}

static Iterator *__begin(Map *map)
{
    RBTree_Iterator *iter = (RBTree_Iterator *)map->b;

    dbg_str(RBTMAP_DETAIL, "Rbtree Map begin");

    rbtree_map_begin(((RBTree_Map *)map)->rbmap, &iter->rbtree_map_pos);

    return (Iterator *)iter;
}

static Iterator *__end(Map *map)
{
    RBTree_Iterator *iter = (RBTree_Iterator *)map->e;

    dbg_str(RBTMAP_DETAIL, "Rbtree Map end");

    rbtree_map_end(((RBTree_Map *)map)->rbmap, &iter->rbtree_map_pos);

    return (Iterator *)iter;
}

static int __count(Map *map)
{
    rbtree_map_t *rbtree_map = ((RBTree_Map *)map)->rbmap;

    return rbtree_map_count(rbtree_map);
}

/*deprecated*/
static int __clear(Map *map)
{
    rbtree_map_t *rbtree_map = ((RBTree_Map *)map)->rbmap;
    rbtree_map_pos_t it, end;
    void *element;

    for(    rbtree_map_begin(rbtree_map, &it); 
            !rbtree_map_pos_equal(&it, rbtree_map_end(rbtree_map, &end));
            rbtree_map_begin(rbtree_map, &it)) 
    {
        rbtree_map_remove(rbtree_map, &it, &element);
        if (map->trustee_flag != 1) {
            continue;
        }

        if (    map->value_type == VALUE_TYPE_OBJ_POINTER && 
                element != NULL) 
        {
            object_destroy(element);
        } else if (map->value_type  == VALUE_TYPE_STRING &&
                   element != NULL)
        {
            dbg_str(RBTMAP_WARNNING, "free string:%p", element);
            object_destroy(element);
        } else if (map->value_type  == VALUE_TYPE_ALLOC_POINTER &&
                   element != NULL)
        {
            allocator_mem_free(map->obj.allocator, element);
        } else if (map->value_type  == VALUE_TYPE_UNKNOWN_POINTER &&
                   element != NULL)
        {
            dbg_str(RBTMAP_WARNNING, "not support clear unkown pointer");
        } else {
        }
        element = NULL;
    }

    return 0;
}

static class_info_entry_t rbtree_map_class_info[] = {
    Init_Obj___Entry(0 , Map, map),
    Init_Nfunc_Entry(1 , RBTree_Map, construct, __construct),
    Init_Nfunc_Entry(2 , RBTree_Map, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , RBTree_Map, add, __add),
    Init_Vfunc_Entry(4 , RBTree_Map, contain_key, NULL),
    Init_Vfunc_Entry(5 , RBTree_Map, contain_value, NULL),
    Init_Vfunc_Entry(6 , RBTree_Map, contain_key_and_value, NULL),
    Init_Vfunc_Entry(7 , RBTree_Map, search, __search),
    Init_Vfunc_Entry(8 , RBTree_Map, search_all_same_key, __search_all_same_key),
    Init_Vfunc_Entry(9 , RBTree_Map, remove, __remove),
    Init_Vfunc_Entry(10, RBTree_Map, del, __del),
    Init_Vfunc_Entry(11, RBTree_Map, begin, __begin),
    Init_Vfunc_Entry(12, RBTree_Map, end, __end),
    Init_Vfunc_Entry(13, RBTree_Map, set_cmp_func, __set_cmp_func),
    Init_Vfunc_Entry(14, RBTree_Map, for_each, NULL),
    Init_Vfunc_Entry(15, RBTree_Map, count, __count),
    Init_Vfunc_Entry(16, RBTree_Map, clear, NULL),
    Init_End___Entry(17, RBTree_Map),
};

REGISTER_CLASS("RBTree_Map", rbtree_map_class_info);
