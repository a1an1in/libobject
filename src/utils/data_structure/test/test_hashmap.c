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

static struct test *init_test_instance(struct test *t, int a, int b)
{
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
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    hmap = hash_map_alloc(allocator);
    hash_map_init(hmap, 2, sizeof(void *), 10);

    hash_map_insert(hmap,"00",&t0);
    hash_map_insert(hmap,"11",&t1);
    hash_map_insert(hmap,"22",&t2);
    hash_map_insert(hmap,"33",&t3);
    hash_map_insert(hmap,"44",&t4);

    dbg_str(DBG_DETAIL,"hash map search");
    hash_map_search(hmap,"33",&pos);
    t = hash_map_pos_get_pointer(&pos);
    dbg_str(DBG_DETAIL,"a =%d, b=%d",t->a,t->b);

    dbg_str(DBG_DETAIL,"for each");
    hash_map_for_each(hmap,print_hash_map_data);

    hash_map_destroy(hmap);
}
