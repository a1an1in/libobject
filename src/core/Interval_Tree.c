/**
 * @file interval tree.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2032-09-13
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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Interval_Tree.h>
#include <libobject/core/Rbtree_Map.h>

static inline int __integer_key_cmp_func(void *key1, void *key2)
{
    if (key1 > key2) return 1;
    else if (key1 < key2) return -1;
    else return 0;
}

static int __construct(Interval_Tree *tree, char *init_str)
{
    allocator_t *allocator = tree->obj.allocator;
    Map *map;

    tree->map = object_new(allocator, "RBTree_Map", NULL);
    map = tree->map;
    map->set_cmp_func(map, __integer_key_cmp_func);
    // map->set(map, "/Map/trustee_flag", &trustee_flag);

    return 1;
}

static int __deconstruct(Interval_Tree *tree)
{
    dbg_str(DBG_DETAIL, "interval tree deconstruct, tree addr:%p", tree);
    object_destroy(tree->map);

    return 1;
}

static int __add(Interval_Tree *tree, void *key, interval_tree_node_t *value)
{
    return tree->map->add(tree->map, key, value);
}

static int __remove(Interval_Tree *tree, void *key, interval_tree_node_t **element)
{
    return tree->map->remove(tree->map, key, element);
}

static int __search(Interval_Tree *tree, void *key, interval_tree_node_t **element)
{
    rbtree_map_t *rbtree_map = ((RBTree_Map *)tree->map)->rbmap;
    struct rb_root_s *root = rbtree_map->tree_root;
    struct rb_node_s *node = root->rb_node;
    int ret = 0;
    void **addr;

    dbg_str(DBG_DETAIL, "interval tree search");
    if (element == NULL) return -1;
    else *element = NULL;

    sync_lock(&rbtree_map->map_lock, NULL);
    key_cmp_fpt key_cmp_func = rbtree_map->key_cmp_func;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        interval_tree_node_t *value = mnode->value;
        int result1, result2;

        result1 = key_cmp_func(key, value->start);
        result2 = key_cmp_func(key, value->end);
        if (result1 < 0) {
            node = node->rb_left;
        } else if (result2 > 0) {
            node = node->rb_right;
        } else {
            *element = mnode->value;
            ret = 1;
            break;
        }
    }
    sync_unlock(&rbtree_map->map_lock);

    return ret;
}

//not trustee key here, key is interger by default
static int __customize(Interval_Tree *tree, int value_type, 
                       int (*value_free_callback)(allocator_t *allocator, void *value))
{
    Map *map;
    int trustee_flag = 1;

    map = tree->map;
    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    map->set(map, "/Map/value_free_callback", value_free_callback);

    return 1;
}

static class_info_entry_t interval_tree_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Interval_Tree, construct, __construct),
    Init_Nfunc_Entry(2 , Interval_Tree, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Interval_Tree, add, __add),
    Init_Vfunc_Entry(4 , Interval_Tree, search, __search),
    Init_Vfunc_Entry(5 , Interval_Tree, remove, __remove),
    Init_Vfunc_Entry(6 , Interval_Tree, customize, __customize),
    Init_End___Entry(7 , Interval_Tree),
};
REGISTER_CLASS("Interval_Tree", interval_tree_class_info);
