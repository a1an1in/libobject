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
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/vector.h>
#include <libobject/utils/config/config.h>

static int __construct(Vector *vector,char *init_str)
{
    allocator_t *allocator = vector->obj.allocator;
    dbg_str(OBJ_DETAIL,"vector construct, vector addr:%p",vector);

    vector->vector = vector_create(allocator,0);
    vector_init(vector->vector, vector->value_size,vector->capacity);

    return 0;
}

static int __deconstrcut(Vector *vector)
{
    dbg_str(OBJ_DETAIL,"vector deconstruct,vector addr:%p",vector);

    vector_destroy(vector->vector);

    return 0;
}

static int __set(Vector *vector, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        vector->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        vector->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        vector->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        vector->deconstruct = value;
    }
    else if (strcmp(attrib, "set_data") == 0) {
        vector->set_data = value;
    } else if (strcmp(attrib, "get_data") == 0) {
        vector->get_data = value;
    } else if (strcmp(attrib, "for_each_by_index") == 0) {
        vector->for_each_by_index = value;
    }
    else if (strcmp(attrib, "value_size") == 0) {
        vector->value_size = *(uint32_t *)value;
    } else if (strcmp(attrib, "capacity") == 0) {
        vector->capacity = *(uint32_t *)value;
    }
    else {
        dbg_str(OBJ_DETAIL,"vector set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Vector *vector, char *attrib)
{
    if (strcmp(attrib, "value_size") == 0) {
        return &vector->value_size;
    } else if (strcmp(attrib, "capacity") == 0) {
        return &vector->capacity;
    } else {
        dbg_str(OBJ_WARNNING,"vector get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __set_data(Vector *vector,int index, void *value)
{
   return vector_set(vector->vector,index,value);
}

static int __get_data(Vector *vector,int index, void **value)
{
    void *addr;

    addr = vector_get(vector->vector,index);
    *value = addr;

    return 0;
}

static void __for_each_by_index(Vector *vector,void (*func)(Vector *vector,int index))
{
	vector_pos_t pos,next;
    vector_t *v = vector->vector;

	for(	vector_begin(v, &pos), vector_pos_next(&pos,&next);
			!vector_pos_equal(&pos,&v->end);
			pos = next, vector_pos_next(&pos,&next))
	{
		func(vector, pos.vector_pos);
	}
}


static class_info_entry_t vector_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_data",__set_data,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_data",__get_data,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","for_each_by_index",__for_each_by_index,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_UINT32_T,"","value_size",0,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_UINT32_T,"","capacity",0,sizeof(void *)},
    [10] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Vector",vector_class_info);

#define name_to_str(name_31415926)  (#name_31415926)

struct test{
    int a;
    int b;
};

static void print_vector_data(Vector *vector, int index)
{
    struct test *t;
    
    vector->get_data(vector, index, (void **)&t);
    dbg_str(DBG_DETAIL,"t%d a =%d b=%d", index, t->a, t->b);
}

void test_obj_vector()
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int value_size = 25;
    struct test t0={0,2};
    struct test t1={1,2};
    struct test t2={2,2};
    struct test t3={3,2};
    struct test t4={4,2};
    struct test t5={5,2};
    struct test t6={6,2};
    struct test t7={7,2};
    struct test t8={8,2};
    struct test t9={9,2};
    struct test *t;

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p",c);
    cfg_config_num(c, "/Vector", "capacity", 4) ;  
    cfg_config_num(c, "/Vector", "value_size", value_size) ;

    dbg_str(DBG_DETAIL,"value_size :%s", name_to_str(value_size)) ;
    vector = OBJECT_NEW(allocator, Vector,c->buf);

    object_dump(vector, "Vector", buf, 2048);
    dbg_str(DBG_DETAIL,"Vector dump: %s",buf);

    vector->set_data(vector, 0, &t0);
    vector->set_data(vector, 1, &t1);
    vector->set_data(vector, 2, &t2);
    vector->set_data(vector, 3, &t3);
    vector->set_data(vector, 4, &t4);
    vector->set_data(vector, 5, &t5);
    vector->set_data(vector, 6, &t6);
    vector->set_data(vector, 7, &t7);
    vector->set_data(vector, 8, &t8);
    vector->set_data(vector, 9, &t9);

    vector->get_data(vector, 0, (void **)&t);
    dbg_str(DBG_DETAIL,"t0 a =%d b=%d", t->a, t->b);

    dbg_str(DBG_DETAIL,"vector for each");
    vector->for_each_by_index(vector, print_vector_data);

    object_destroy(vector);
    cfg_destroy(c);

}


