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
void print_list_data(list_t *list)
{

    struct test *t = (struct test *)list->data;

    dbg_str(DBG_DETAIL,"a=%d,b=%d",t->a,t->b);
}
int test_datastructure_link_list()
{
    llist_t *llist;
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *struct test t1={1,2};
     *struct test t2={2,2};
     *struct test t3={3,2};
     *struct test t4={4,2};
     */
    struct test *t, *t1, *t2, *t3, *t4;
    int ret = 0;
    /*
     *int data_size = sizeof(struct test);
     */
    int data_size = sizeof(void *);
    int lock_type = 1;

    t1 = allocator_mem_alloc(allocator, sizeof(struct test));
    t1->a = 1;
    t1->b = 2;
    t2 = allocator_mem_alloc(allocator, sizeof(struct test));
    t2->a = 2;
    t2->b = 2;
    t3 = allocator_mem_alloc(allocator, sizeof(struct test));
    t3->a = 3;
    t3->b = 2;
    t4 = allocator_mem_alloc(allocator, sizeof(struct test));
    t4->a = 4;
    t4->b = 2;

    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_set(llist,"data_size",&data_size);
    llist_init(llist);

    llist_add_front(llist,t1);
    llist_add_front(llist,t2);
    llist_add_front(llist,t3);
    llist_add_front(llist,t4);

    /*
     *llist_add_back(llist,t1);
     *llist_add_back(llist,t2);
     *llist_add_back(llist,t3);
     *llist_add_back(llist,t4);
     */

    llist_for_each(llist,print_list_data);


    llist_remove_back(llist, (void **)&t);
    dbg_str(DBG_DETAIL,"llist_delete_back, a=%d,b=%d",t->a,t->b);

    allocator_mem_free(allocator, t);
    llist_destroy(llist);
    return ret;
}
