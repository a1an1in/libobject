/*
 * =====================================================================================
 *
 *       Filename:  test_datastructure_link_list.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/14/2015 11:16:44 AM
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
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/alloc/allocator.h"
#include "libobject/utils/data_structure/link_list.h"

struct test{
    int a;
    int b;
};
void print_list_data(struct test *t)
{
    dbg_str(DBG_DETAIL,"a=%d,b=%d",t->a,t->b);
}

static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

int test_datastructure_link_list()
{
    llist_t *llist;
    allocator_t *allocator = allocator_get_default_alloc();
    int data_size = sizeof(void *);
    int lock_type = 1;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_set(llist,"data_size",&data_size);
    llist_init(llist);

    llist_add_front(llist, &t0);
    llist_add_front(llist, &t1);
    llist_add_front(llist, &t2);
    llist_add_front(llist, &t3);

    /*
     *llist_add_back(llist,t1);
     *llist_add_back(llist,t2);
     *llist_add_back(llist,t3);
     *llist_add_back(llist,t4);
     */

    dbg_str(DBG_DETAIL,"llist for each:");
    dbg_str(DBG_DETAIL,"llist count=%d", llist_get_count(llist));
    llist_for_each(llist, (void (*)(void *))print_list_data);

    dbg_str(DBG_DETAIL,"remove t2");
    llist_remove_element(llist, &t2);

    dbg_str(DBG_DETAIL,"llist for each:");
    dbg_str(DBG_DETAIL,"llist count=%d", llist_get_count(llist));
    llist_for_each(llist, (void (*)(void *))print_list_data);
    /*
     *llist_remove_back(llist, (void **)&t);
     *dbg_str(DBG_DETAIL,"llist_delete_back, a=%d,b=%d",t->a,t->b);
     *allocator_mem_free(allocator, t);
     */

    llist_destroy(llist);
    return ret;
}
