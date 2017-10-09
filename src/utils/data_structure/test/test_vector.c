/*
 * =====================================================================================
 *
 *       Filename:  test_datastructure_link_vector.c
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
#include "libobject/utils/data_structure/vector.h"

struct test{
    int a;
    int b;
};
void print_vector_data(vector_pos_t *pos)
{
    dbg_buf(DBG_DETAIL,"data:",vector_pos_get_pointer(pos),pos->vector->step);
}

static struct test *genearte_test_instance(allocator_t *allocator, int a, int b)
{
    struct test *t;

    t = allocator_mem_alloc(allocator, sizeof(struct test));
    t->a = a;
    t->b = b;

    return t;
}

int test_datastructure_vector()
{
    vector_t *vector;
    vector_pos_t pos;
    allocator_t *allocator = allocator_get_default_alloc();

    struct test *t, *t0, *t1, *t2, *t3, *t4, *t5;
    int ret = 0;

    t0 = genearte_test_instance(allocator, 0, 2);
    t1 = genearte_test_instance(allocator, 1, 2);
    t2 = genearte_test_instance(allocator, 2, 2);
    t3 = genearte_test_instance(allocator, 3, 2);
    t4 = genearte_test_instance(allocator, 4, 2);
    t5 = genearte_test_instance(allocator, 5, 2);

    vector = vector_create(allocator,0);
    vector_init(vector,sizeof(struct test),4);

    vector_add_back(vector, t0);
    vector_add_back(vector, t1);
    vector_add_back(vector, t2);
    vector_add_back(vector, t3);
    vector_add_back(vector, t4);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    dbg_str(DBG_DETAIL,"test get");
    t = vector_get(vector,2);
    dbg_buf(DBG_DETAIL,"get data:", (char *)t,vector->data_size);

    dbg_str(DBG_DETAIL,"test set");
    vector_set(vector,1, t5);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    vector_destroy(vector);
    return ret;
}
