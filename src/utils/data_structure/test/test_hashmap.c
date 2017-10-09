/*
 * =====================================================================================
 *
 *       Filename:  test_hashmap.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/14/2015 10:07:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "libobject/utils/data_structure/hash_list.h"
#include "libobject/utils/alloc/allocator.h"

struct test{
    int a;
    int b;
};
#if 0
void test_datastructure_hashlist()
{
    hash_map_t *hmap;
    pair_t *pair;
    hash_map_pos_t pos;
    struct hash_map_node *mnode;
    allocator_t *allocator = allocator_get_default_alloc();

    struct A t1 = {1,2};
    struct A t2 = {2,2};
    struct A t3 = {3,2};
    struct A t4 = {4,2};
    struct A t5 = {5,2};

    pair = create_pair(2,sizeof(struct A));

    hmap = hash_map_create(allocator,0);
    hash_map_init(hmap,
                  2,//uint32_t key_size,
                  sizeof(struct A)+ 2,
                  10,
                  NULL,
                  NULL);

    make_pair(pair,"11",&t1);
    hash_map_insert_data(hmap,pair->data);
    make_pair(pair,"22",&t2);
    hash_map_insert_data(hmap,pair->data);
    make_pair(pair,"33",&t3);
    hash_map_insert_data(hmap,pair->data);
    make_pair(pair,"55",&t4);
    hash_map_insert_data(hmap,pair->data);

    dbg_str(DBG_DETAIL,"hash map search");
    struct A *p;
    hash_map_search(hmap,"33",&pos);
    p = hash_map_pos_get_pointer(&pos);
    dbg_str(DBG_DETAIL,"a =%d, b=%d",p->a,p->b);

    dbg_str(DBG_DETAIL,"for each");
    hash_map_for_each(hmap,hash_map_print_mnode);

    hash_map_destroy(hmap);
}
#else

static struct test *genearte_test_instance(allocator_t *allocator, int a, int b)
{
    struct test *t;

    t = allocator_mem_alloc(allocator, sizeof(struct test));
    t->a = a;
    t->b = b;

    return t;
}

void print_hash_map_data(hash_map_pos_t *pos)
{
    struct test *t;
    char *key;
    char k[1024] = {0};

    key = (char *)hash_map_pos_get_kpointer(pos);
    memcpy(k, key, pos->hmap->key_size);
    t = (struct test *)hash_map_pos_get_pointer(pos);

    dbg_str(DBG_DETAIL,"key: %s, t->a=%d, t->b=%d", k, t->a, t->b);
}

void test_datastructure_hashlist()
{
    hash_map_t *hmap;
    pair_t *pair;
    hash_map_pos_t pos;
    struct hash_map_node *mnode;
    allocator_t *allocator = allocator_get_default_alloc();
    struct test *t, *t0, *t1, *t2, *t3, *t4, *t5;

    t0 = genearte_test_instance(allocator, 0, 2);
    t1 = genearte_test_instance(allocator, 1, 2);
    t2 = genearte_test_instance(allocator, 2, 2);
    t3 = genearte_test_instance(allocator, 3, 2);
    t4 = genearte_test_instance(allocator, 4, 2);
    /*
     *t5 = genearte_test_instance(allocator, 5, 2);
     */

    hmap = hash_map_alloc(allocator);
    hash_map_init(hmap, 2, sizeof(void *), 10);

    hash_map_insert(hmap,"00",t0);
    hash_map_insert(hmap,"11",t1);
    hash_map_insert(hmap,"22",t2);
    hash_map_insert(hmap,"33",t3);
    hash_map_insert(hmap,"44",t4);

    dbg_str(DBG_DETAIL,"hash map search");
    hash_map_search(hmap,"33",&pos);
    t = hash_map_pos_get_pointer(&pos);
    dbg_str(DBG_DETAIL,"a =%d, b=%d",t->a,t->b);

    dbg_str(DBG_DETAIL,"for each");
    hash_map_for_each(hmap,print_hash_map_data);

    hash_map_destroy(hmap);
}
#endif
