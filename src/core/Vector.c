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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/core/String.h>
#include <libobject/argument/Command.h>

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

    if (vector->init_data != NULL) {
        cjson_t *c;

        dbg_str(DBG_DETAIL, "vector init data:%s", 
                vector->init_data->get_cstr(vector->init_data));

        c = cjson_parse(vector->init_data->get_cstr(vector->init_data));
        if (c->type & OBJECT_ARRAY) {
            c = c->child;
            dbg_str(DBG_DETAIL, "array name:%s", c->string);
        }

        while (c) {
            if (c->type & CJSON_NUMBER) {
                vector->add(vector, c->valueint);
            } else if (c->type & OBJECT_STRING) {
                String *s;
                dbg_str(DBG_DETAIL, "vector element:%s", c->valuestring);
                s = object_new(allocator, "String", NULL);
                s->assign(s, c->valuestring);
                vector->add(vector, s);
            } else if (c->type & OBJECT_ARRAY) {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            } else if (c->type & CJSON_OBJECT) {
                char *out;
                Obj *o;
                char *class_name = vector->class_name->get_cstr(vector->class_name);
                char init_str[1024];
                out = cjson_print(c);
                sprintf(init_str, "{\"%s\":%s}", class_name, out);
                dbg_str(DBG_DETAIL, "init_str:%s", init_str);
                o = object_new(allocator, class_name, init_str);
                vector->add(vector, o);

                dbg_str(DBG_DETAIL, "o: %s", o->to_json(o));
                free(out);
            } else {
                dbg_str(DBG_DETAIL, "vector element, not supported now!");
            }

            c = c->next;
        }

        cjson_delete(c);
    }

    return 0;
}

static int __deconstrcut(Vector *vector)
{
    dbg_str(OBJ_DETAIL, "vector deconstruct, vector addr:%p", vector);

    if (vector->init_data != NULL) {
        object_destroy(vector->init_data);
    }

    if (vector->class_name != NULL) {
        object_destroy(vector->class_name);
    }

    if (vector->trustee_flag == 1) {
        vector_pos_t pos, next;
        vector_t *v = vector->vector;
        void *element;
        int index = 0;

        for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
                !vector_pos_equal(&pos, &v->end);
                pos = next, vector_pos_next(&pos, &next))
        {
            vector->peek_at(vector, index++, (void **)&element);

            if (    vector->value_type == VALUE_TYPE_OBJ_POINTER && 
                    element != NULL) 
            {
                object_destroy(element);
            } else if (vector->value_type  == VALUE_TYPE_STRING &&
                       element != NULL)
            {
                object_destroy(element);
            } else {
            }
            element = NULL;
        }
    }

    vector_destroy(vector->vector);

    return 0;
}

static int __reconstruct(Vector *vector)
{
    allocator_t *allocator = vector->obj.allocator;

    dbg_str(DBG_WARNNING, "vector reconstruct, vector addr:%p", vector);

    vector_destroy(vector->vector);

    vector->construct(vector, NULL);

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

static void 
__for_each(Vector *vector, void (*func)(int index, void *element))
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

    for (   vector_begin(v, &pos), vector_pos_next(&pos, &next);
            !vector_pos_equal(&pos, &v->end);
            pos=next, vector_pos_next(&pos, &next))
    {
        vector->remove(vector, index++, (void **)&element);
    }

    vector_pos_init(&v->end, 0, v);
}

static char *__to_json(Obj *obj)
{
    Vector *vector = (Vector *)obj;
	vector_pos_t pos, next;
    vector_t *v = vector->vector;
    cjson_t *root, *item = NULL;
    String *json = (String *)obj->json;
    void *element = NULL;
    char *out;
    int index = 0;

    if (json == NULL) {
        json = object_new(obj->allocator, "String", NULL);
        obj->json = json;
    } else {
        json->clear(json);
    }

    root = cjson_create_array();

	for(	vector_begin(v, &pos), vector_pos_next(&pos, &next);
			!vector_pos_equal(&pos, &v->end);
			pos = next, vector_pos_next(&pos, &next))
	{
        vector->peek_at(vector, index++, (void **)&element);

        switch(vector->value_type) {
            case VALUE_TYPE_INT8_T: {
                    int8_t num = (int8_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT8_T: {
                    uint8_t num = (uint8_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT16_T: {
                    int16_t num = (int16_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT16_T: {
                    uint16_t num = (uint16_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT32_T: {
                    int32_t num = (int32_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT32_T: {
                    uint32_t num = (uint32_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_INT64_T: {
                    int64_t num = (int64_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_UINT64_T: {
                    uint64_t num = (uint64_t)element;
                    item = cjson_create_number(num);
                    break;
                }
            case VALUE_TYPE_FLOAT_T: {
                    float *num = (float *)element;
                    item = cjson_create_number(*num);
                    break;
                }
            case VALUE_TYPE_STRING: {
                    String *s = (String *)element;
                    item = cjson_create_string(s->get_cstr(s));
                    break;
                }
            case VALUE_TYPE_OBJ_POINTER: {
                    Obj *o = (Obj *)element;
                    item = cjson_parse(o->to_json(o));
                    break;
                }
            case VALUE_TYPE_NORMAL_POINTER:
                dbg_str(DBG_DETAIL, "value type:%d not supported now!!",
                        vector->value_type);
                break;
            default:
                break;
        }

        if (item != NULL)
            cjson_add_item_to_array(root, item);
        item = NULL;
	}

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static class_info_entry_t vector_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Vector, construct, __construct),
    Init_Nfunc_Entry(2 , Vector, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Vector, reconstruct, __reconstruct),
    Init_Vfunc_Entry(4 , Vector, set, NULL),
    Init_Vfunc_Entry(5 , Vector, get, NULL),
    Init_Vfunc_Entry(6 , Vector, add, __add),
    Init_Vfunc_Entry(7 , Vector, add_at, __add_at),
    Init_Vfunc_Entry(8 , Vector, add_back, __add_back),
    Init_Vfunc_Entry(9 , Vector, remove, __remove),
    Init_Vfunc_Entry(10, Vector, remove_back, __remove_back),
    Init_Vfunc_Entry(11, Vector, peek_at, __peek_at),
    Init_Vfunc_Entry(12, Vector, for_each, __for_each),
    Init_Vfunc_Entry(13, Vector, free_vector_elements, __free_vector_elements),
    Init_Vfunc_Entry(14, Vector, clear, __clear),
    Init_Vfunc_Entry(15, Vector, size, __size),
    Init_Vfunc_Entry(16, Vector, empty, __empty),
    /*
     *Init_Vfunc_Entry(17, Vector, to_json, NULL),
     */
    Init_Vfunc_Entry(17, Vector, to_json, __to_json),
    Init_U32___Entry(18, Vector, value_size, NULL),
    Init_U8____Entry(19, Vector, value_type, NULL),
    Init_U32___Entry(20, Vector, capacity, NULL),
    Init_Str___Entry(21, Vector, init_data, NULL),
    Init_Str___Entry(22, Vector, class_name, NULL),
    Init_U8____Entry(23, Vector, trustee_flag, 0),
    Init_End___Entry(24, Vector),
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

static int test_vector(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int pre_alloc_count, after_alloc_count;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int capacity = 10;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret;

    pre_alloc_count = allocator->alloc_count;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);


    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->reconstruct(vector);

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
REGISTER_TEST_FUNC(test_vector);

static int test_int_vector_to_json(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_size = 25, capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *result = "[0, 1, 2, 3, 4, 5]";

    vector = OBJECT_NEW(allocator, Vector, NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->reconstruct(vector);

    vector->add_at(vector, 0, 0);
    vector->add_at(vector, 1, 1);
    vector->add_at(vector, 2, 2);
    vector->add_at(vector, 3, 3);
    vector->add_at(vector, 4, 4);
    vector->add_at(vector, 5, 5);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
    if (strcmp(result, vector->to_json(vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(vector);
    
    return ret;

}
REGISTER_TEST_FUNC(test_int_vector_to_json);

static int test_string_vector_to_json(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_size = 25, capacity = 19, value_type = VALUE_TYPE_STRING;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret;
    char result[1024];

    t0 = object_new(allocator, "String", NULL);
    t1 = object_new(allocator, "String", NULL);
    t2 = object_new(allocator, "String", NULL);
    t3 = object_new(allocator, "String", NULL);
    t4 = object_new(allocator, "String", NULL);
    t5 = object_new(allocator, "String", NULL);

    t0->assign(t0, "Monday");
    t1->assign(t1, "Tuesday");
    t2->assign(t2, "Wednesday");
    t3->assign(t3, "Thursday");
    t4->assign(t4, "Friday");
    t5->assign(t5, "Saturday");

    sprintf(result, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", 
            t0->get_cstr(t0), t1->get_cstr(t1), t2->get_cstr(t2),
            t3->get_cstr(t3), t4->get_cstr(t4), t5->get_cstr(t5));

    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);

    vector->add_at(vector, 0, t0);
    vector->add_at(vector, 1, t1);
    vector->add_at(vector, 2, t2);
    vector->add_at(vector, 3, t3);
    vector->add_at(vector, 4, t4);
    vector->add_at(vector, 5, t5);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
    dbg_str(DBG_DETAIL, "result: %s", result);

    if (strcmp(result, vector->to_json(vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(vector);
    object_destroy(t0);
    object_destroy(t1);
    object_destroy(t2);
    object_destroy(t3);
    object_destroy(t4);
    object_destroy(t5);
    
    return ret;

}
REGISTER_TEST_FUNC(test_string_vector_to_json);

static int test_obj_vector_to_json(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Vector *vector;
    Command *cmd0 = NULL;
    Command *cmd1 = NULL;
    Command *cmd2 = NULL;
    int ret = 0, help = 0;

    cmd0 = object_new(allocator, "Test_Command", NULL);
    cmd1 = object_new(allocator, "Test_Command", NULL);
    cmd2 = object_new(allocator, "Test_Command", NULL);

    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/value_type", &value_type);

    help = 0;
    cmd0->set(cmd0, "/Test_Command/help", &help);
    cmd0->set(cmd0, "/Test_Command/option", "test cmd0 option");

    help = 1;
    cmd1->set(cmd1, "/Test_Command/help", &help);
    cmd1->set(cmd1, "/Test_Command/option", "test cmd1 option");

    help = 2;
    cmd2->set(cmd2, "/Test_Command/help", &help);
    cmd2->set(cmd2, "/Test_Command/option", "test cmd2 option");

    vector->add(vector, cmd0);
    vector->add(vector, cmd1);
    vector->add(vector, cmd2);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));

    object_destroy(cmd0);
    object_destroy(cmd1);
    object_destroy(cmd2);
    object_destroy(vector);

    ret = 1;

    return ret;

}
REGISTER_TEST_CMD(test_obj_vector_to_json);


static int test_int_vector_set_init_data(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_size = 25, capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *init_data = "[0, 1, 2, 3, 4, 5]";

    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
    if (strcmp(init_data, vector->to_json(vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(vector);
    
    return ret;

}
REGISTER_TEST_FUNC(test_int_vector_set_init_data);

static int test_string_vector_set_init_data(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_size = 25, capacity = 19, value_type = VALUE_TYPE_STRING;
    uint8_t trustee_flag = 1;
    int ret;
    char *init_data = "[\"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";

    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
    if (strcmp(init_data, vector->to_json(vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(vector);
    
    return ret;

}
REGISTER_TEST_FUNC(test_string_vector_set_init_data);

static int test_obj_vector_set_init_data(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_alloc();
    int value_size = 25, capacity = 19, value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    int ret;
    char *init_data = "[\
        {\
            \"option\": \"test cmd0 option\",\
            \"help\": 0\
        }, {\
            \"option\": \"test cmd1 option\",\
            \"help\": 1\
        }, {\
            \"option\": \"test cmd2 option\",\
            \"help\": 2\
        }]";

    vector = object_new(allocator, "Vector", NULL);
    vector->set(vector, "/Vector/capacity", &capacity);
    vector->set(vector, "/Vector/value_type", &value_type);
    vector->set(vector, "/Vector/init_data", init_data);
    vector->set(vector, "/Vector/class_name", "Test_Command");
    vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
    vector->reconstruct(vector);

    dbg_str(DBG_DETAIL, "Vector json: %s", vector->to_json(vector));
    if (strcmp(init_data, vector->to_json(vector)) != 0) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(vector);
    
    return ret;

}
REGISTER_TEST_CMD(test_obj_vector_set_init_data);
