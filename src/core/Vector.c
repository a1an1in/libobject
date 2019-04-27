/**
 * @file Vector.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-08
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this vector of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this vector of conditions and the following disclaimer in the
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
#include <libobject/core/config.h>
#include <libobject/core/vector.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(Vector *vector, char *init_str)
{
    allocator_t *allocator = vector->obj.allocator;
    dbg_str(OBJ_DETAIL, "vector construct, vector addr:%p", vector);

    vector->vector = vector_create(allocator, 0);
    if (vector->value_size == 0) {
        vector->value_size = sizeof(void *);
    }
    if (vector->capacity == 0) {
        vector->capacity = 10;
    }
    vector_init(vector->vector, vector->value_size, vector->capacity);

    return 0;
}

static int __deconstrcut(Vector *vector)
{
    dbg_str(OBJ_DETAIL, "vector deconstruct, vector addr:%p", vector);

    vector_destroy(vector->vector);

    return 0;
}

static int __add(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __add_back(Vector *vector, void *value)
{
    return vector_add_back(vector->vector, value);
}

static int __remove(Vector *vector, int index, void **value)
{
    return vector_remove(vector->vector, index, value);
}

static int __remove_back(Vector *vector, void **value)
{
    return vector_remove_back(vector->vector, value);
}

static int __add_at(Vector *vector, int index, void *value)
{
   return vector_add_at(vector->vector, index, value);
}

static int __peek_at(Vector *vector, int index, void **value)
{
    return vector_peek_at(vector->vector, index, value);
}

static void __for_each(Vector *vector, void (*func)(int index, void *element))
{
	vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

	for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
			!vector_pos_equal(&pos, &v->end);
			pos = next, vector_pos_next(&pos, &next))
	{
        vector->peek_at(vector, index, (void **)&element);
		func(index++, element);
	}
}

static void __free_vector_elements(Vector *vector)
{   
    vector_pos_t pos, next;
    vector_t *v = vector->vector;
    void *element;
    int index = 0;

    for(vector_begin(v, &pos), vector_pos_next(&pos, &next);
        !vector_pos_equal(&pos, &v->end);
        pos = next, vector_pos_next(&pos, &next))
    {
        vector->peek_at(vector, index++, (void **)&element);
        if(element != NULL){
            object_destroy(element);
        }
    }
}

static uint32_t __size(Vector * vector)
{
    uint32_t count = 0;

    sync_trylock(&vector->vector->vector_lock, NULL);
    count = vector->vector->size;
    sync_unlock(&vector->vector->vector_lock);

    return count;
} 

static int  __empty(Vector * vector) 
{
    return vector->size(vector) == 0 ? 1 : 0;
}

static void __clear(Vector *vector)
{
    vector_pos_t pos,next;
    vector_t *v=vector->vector;
    void *element;
    int index =0;

    for(vector_begin(v,&pos),vector_pos_next(&pos,&next);
        !vector_pos_equal(&pos,&v->end);
        pos=next,vector_pos_next(&pos,&next))
    {
        vector->remove(vector,index,(void **)&element);
    }
}

static class_info_entry_t vector_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Vector, construct, __construct),
    Init_Nfunc_Entry(2 , Vector, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Vector, add, __add),
    Init_Vfunc_Entry(4 , Vector, add_at, __add_at),
    Init_Vfunc_Entry(5 , Vector, add_back, __add_back),
    Init_Vfunc_Entry(6 , Vector, remove, __remove),
    Init_Vfunc_Entry(7 , Vector, remove_back, __remove_back),
    Init_Vfunc_Entry(8 , Vector, peek_at, __peek_at),
    Init_Vfunc_Entry(9 , Vector, for_each, __for_each),
    Init_Vfunc_Entry(10, Vector, free_vector_elements, __free_vector_elements),
    Init_Vfunc_Entry(11, Vector, clear, __clear),
    Init_Vfunc_Entry(12, Vector, size, __size),
    Init_Vfunc_Entry(13, Vector, empty, __empty),
    Init_U32___Entry(15, Vector, value_size, NULL),
    Init_U32___Entry(14, Vector, capacity, NULL),
    Init_End___Entry(16, Vector),
};
REGISTER_CLASS("Vector", vector_class_info);

struct test{
    int a;
    int b;
};

static void print_vector_data(int index, void *element)
{
    struct test *t = (struct test *)element;
    
    dbg_str(DBG_DETAIL, "index=%d, a =%d b=%d", index, t->a, t->b);
}

static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;


    return t;
}

static int test_obj_vector(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int pre_alloc_count, after_alloc_count;
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int value_size = 25;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret;

    pre_alloc_count = allocator->alloc_count;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config_num(c, "/Vector", "capacity", 10) ;  
    cfg_config_num(c, "/Vector", "value_size", value_size) ;

    vector = OBJECT_NEW(allocator, Vector, c->buf);

    /*
     *object_dump(vector, "Vector", buf, 2048);
     *dbg_str(DBG_DETAIL, "Vector dump: %s", buf);
     */

    vector->add_at(vector, 0, &t0);
    vector->add_at(vector, 1, &t1);
    vector->add_at(vector, 2, &t2);
    vector->add_at(vector, 3, &t3);
    vector->add_at(vector, 4, &t4);
    vector->add_at(vector, 5, &t5);

    /*
     *vector->add(vector, &t0);
     *vector->add(vector, &t1);
     *vector->add(vector, &t2);
     *vector->add(vector, &t3);
     *vector->add(vector, &t4);
     *vector->add(vector, &t5);
     */

    /*
     *dbg_str(DBG_DETAIL, "vector for each");
     *vector->for_each(vector, print_vector_data);
     */

    vector->peek_at(vector, 1, (void **)&t);
    dbg_str(DBG_DETAIL, "peak at index =%d a =%d b=%d", 1 , t->a, t->b);

    ret = assert_equal(t, &t1, sizeof(void *));
    if (ret == 0) {
        return ret;
    }

    /*
     *dbg_str(DBG_DETAIL, "vector for each");
     *vector->for_each(vector, print_vector_data);
     */

    vector->remove(vector, 4, (void **)&t);
    dbg_str(DBG_DETAIL, "remove index 4, t->a=%d t->b=%d", t->a, t->b);
    ret = assert_equal(t, &t4, sizeof(void *));
    if (ret == 0) {
        return ret;
    }

    /*
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *vector->remove_back(vector, (void **)&t);
     *dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     *t = NULL;
     *vector->remove_back(vector, (void **)&t);
     *if (t != NULL)
     *    dbg_str(DBG_DETAIL, "t0 a =%d b=%d", t->a, t->b);
     */

    /*
     *dbg_str(DBG_DETAIL, "vector for each");
     *vector->for_each(vector, print_vector_data);
     */

    //vector->free_vector(vector);
    object_destroy(vector);
    cfg_destroy(c);
    

    after_alloc_count = allocator->alloc_count;
    ret = assert_equal(&pre_alloc_count, &after_alloc_count, sizeof(int));
    if (ret == 0) {
        dbg_str(DBG_WARNNING,
                "vector has memory omit, pre_alloc_count=%d, after_alloc_count=%d",
                pre_alloc_count, after_alloc_count);
        /*
         *allocator_mem_info(allocator);
         */
        return ret;
    }

    return 1;

}
REGISTER_TEST_FUNC(test_obj_vector);

