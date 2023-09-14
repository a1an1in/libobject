/**
 * @file RBTree_Map_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-13
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/Interval_Tree.h>
#include <libobject/core/utils/registry/registry.h>

#define INIT_TEST_INSTANCE(t, a, b, c) (t).start=a,(t).end=b,(t).value=c

static int test_interval_tree_search_key(TEST_ENTRY *entry)
{
    Interval_Tree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    int ret = 0;
    interval_tree_node_t *t, t0, t1, t2, t3, t4, t5;

    TRY {
        tree = object_new(allocator, "Interval_Tree", NULL);
        INIT_TEST_INSTANCE(t0, 0, 9, 0);
        INIT_TEST_INSTANCE(t1, 10, 19, 0);
        INIT_TEST_INSTANCE(t2, 20, 29, 0);
        INIT_TEST_INSTANCE(t3, 30, 39, 0);
        INIT_TEST_INSTANCE(t4, 40, 49, 0);
        INIT_TEST_INSTANCE(t5, 50, 59, 0);

        tree->add(tree, t0.start, &t0);
        tree->add(tree, t1.start, &t1);
        tree->add(tree, t2.start, &t2);
        tree->add(tree, t3.start, &t3);
        tree->add(tree, t4.start, &t4);
        tree->add(tree, t5.start, &t5);

        ret = tree->search(tree, 23, (void **)&t);
        THROW_IF(ret != 1, -1);
        THROW_IF(t->start != t2.start || t->end != t2.end, -1);
        dbg_str(DBG_VIP, "found key, start:%lx, end:%lx", t->start, t->end);
    } CATCH (ret) { } FINALLY { 
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_interval_tree_search_key);

static int tree_node_free_callback(allocator_t *allocator, void *value)
{
    dbg_str(DBG_VIP, "tree_node_free_callback");
    interval_tree_node_t *t = (interval_tree_node_t *)value;
    allocator_mem_free(allocator, t->value);
    allocator_mem_free(allocator, t);
    return 0;
}

static int test_interval_tree_free_tree_node(TEST_ENTRY *entry)
{
    Interval_Tree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    int ret = 0, count1 = 0, count2 = 0;
    interval_tree_node_t *t, *t0, *t1, *t2;

    TRY {
        sleep(2);
        count1 = allocator->alloc_count;
        t0 = allocator_mem_alloc(allocator, sizeof(interval_tree_node_t));
        t1 = allocator_mem_alloc(allocator, sizeof(interval_tree_node_t));
        t2 = allocator_mem_alloc(allocator, sizeof(interval_tree_node_t));

        INIT_TEST_INSTANCE(*t0, 0, 9, 0);
        INIT_TEST_INSTANCE(*t1, 10, 19, 0);
        INIT_TEST_INSTANCE(*t2, 20, 29, 0);

        tree = object_new(allocator, "Interval_Tree", NULL);
        tree->set_trustee(tree, VALUE_TYPE_STRUCT_POINTER, tree_node_free_callback);
        tree->add(tree, t0->start, t0);
        tree->add(tree, t1->start, t1);
        tree->add(tree, t2->start, t2);

        object_destroy(tree), tree = NULL;
        count2 = allocator->alloc_count;
        SET_CATCH_INT_PARS(count1, count2);
        THROW_IF(count1 != count2, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_interval_tree_free_tree_node);

