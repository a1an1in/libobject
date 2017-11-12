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

void print_vector_data(void *element)
{
    struct test *t = (struct test*)element;

    dbg_str(DBG_DETAIL,"t->a = %d, t->b = %d", t->a, t->b);
}

static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

int test_datastructure_vector()
{
    vector_t *vector;
    vector_pos_t pos;
    allocator_t *allocator = allocator_get_default_alloc();
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret;

    dbg_str(DBG_DETAIL,"test_datastructure_vector begin");
    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    vector = vector_create(allocator,0);
    vector_init(vector,sizeof(void *),10);

    vector_add_back(vector, &t0);
    vector_add_back(vector, &t1);
    vector_add_back(vector, &t2);
    vector_add_back(vector, &t3);
    vector_add_back(vector, &t4);

    dbg_str(DBG_DETAIL,"vector for each");
    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    dbg_str(DBG_DETAIL,"test get");
    vector_peek_at(vector,2, (void **)&t);
    dbg_buf(DBG_DETAIL,"get data:", (char *)t,vector->data_size);

    dbg_str(DBG_DETAIL,"test set");
    vector_add_at(vector,1, &t5);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    vector_destroy(vector);
    dbg_str(DBG_DETAIL,"test_datastructure_vector end");
    return ret;
}
