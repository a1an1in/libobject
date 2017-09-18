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
int test_datastructure_vector()
{
    vector_t *vector;
    vector_pos_t pos;
    allocator_t *allocator = allocator_get_default_alloc();

    struct test t1={1,2};
    struct test t2={2,2};
    struct test t3={3,2};
    struct test t4={4,2};
    int ret = 0;

    vector = vector_create(allocator,0);
    vector_init(vector,sizeof(struct test),4);

    vector_push_back(vector,&t1);
    vector_push_back(vector,&t2);
    vector_push_back(vector,&t3);
    vector_push_back(vector,&t4);
    vector_push_back(vector,&t1);
    vector_push_back(vector,&t2);
    vector_push_back(vector,&t3);
    vector_push_back(vector,&t4);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    dbg_str(DBG_DETAIL,"test get");
    dbg_buf(DBG_DETAIL,"get data:",vector_get(vector,2),vector->data_size);

    dbg_str(DBG_DETAIL,"test set");
    vector_set(vector,1,&t4);

    vector_for_each(vector,(void (*)(void *))(print_vector_data));

    vector_destroy(vector);
    return ret;
}
