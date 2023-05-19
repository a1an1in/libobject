#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/try.h>
#include <libobject/core/Vector.h>

static int test_vector_add(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int expect_ret = 1, ret;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/capacity", &capacity);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 1);
        vector->remove(vector, 0, (void **)&t);

        THROW_IF(t != expect_ret, -1);
    } CATCH(ret) {} FINALLY {
        object_destroy(vector);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_add);

static int test_vector_remove(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int expect_ret = 2, ret;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);
        vector->remove(vector, 2, (void **)&t);

        THROW_IF(t != expect_ret, -1);
    } CATCH(ret) {} FINALLY {
        object_destroy(vector);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_remove);

static int test_vector_count(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int *t = 0;
    int ret, count, expect_count = 5;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);

        count = vector->count(vector);
        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);
        count = vector->count(vector);

        THROW_IF(count != expect_count, -1);
    } CATCH(ret) {} FINALLY {
        object_destroy(vector);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_count);

static int test_vector_to_json_case1()
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *result = "[0, 1, 2, 3, 4, 5]";

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);
        vector->add_at(vector, 5, 5);

        THROW_IF(strcmp(vector->to_json(vector), result) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_to_json_case2()
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    int ret;
    uint8_t trustee_flag = 1;
    char result[1024];

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        t0 = object_new(allocator, "String", "Monday");
        t1 = object_new(allocator, "String", "Tuesday");
        t2 = object_new(allocator, "String", "Wednesday");
        t3 = object_new(allocator, "String", "Thursday");
        t4 = object_new(allocator, "String", "Friday");
        t5 = object_new(allocator, "String", "Saturday");

        sprintf(result, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", 
                t0->get_cstr(t0), t1->get_cstr(t1), t2->get_cstr(t2),
                t3->get_cstr(t3), t4->get_cstr(t4), t5->get_cstr(t5));

        vector->set(vector, "/Vector/capacity", &capacity);
        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

        vector->add_at(vector, 0, t0);
        vector->add_at(vector, 1, t1);
        vector->add_at(vector, 2, t2);
        vector->add_at(vector, 3, t3);
        vector->add_at(vector, 4, t4);
        vector->add_at(vector, 5, t5);

        dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
        dbg_str(DBG_DETAIL, "result: %s", result);

        THROW_IF(strcmp(vector->to_json(vector), result) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_to_json_case3()
{
    int ret = 0, help = 0;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Vector *vector;
    uint8_t trustee_flag = 1;
    Obj *obj1 = NULL;
    Obj *obj2 = NULL;
    String *string = NULL;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        obj1 = object_new(allocator, "Simplest_Obj", NULL);
        obj2 = object_new(allocator, "Simplest_Obj", NULL);

        help = 1;
        obj1->set(obj1, "/Simplest_Obj/help", &help);
        obj1->set(obj1, "/Simplest_Obj/name", "simplest obj1");

        help = 2;
        obj2->set(obj2, "/Simplest_Obj/help", &help);
        obj2->set(obj2, "/Simplest_Obj/name", "simplest obj2");


        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);

        vector->add(vector, obj1);
        vector->add(vector, obj2);

        string = object_new(allocator, "String", NULL);
        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);

        SET_CATCH_STR_PARS(string->get_cstr(string), init_data);
        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_vector_to_json_case3 error, par1=%s, par2=%s", ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    } FINALLY {
        object_destroy(string);
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_to_json(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_vector_to_json_case1());
        EXEC(test_vector_to_json_case2());
        EXEC(test_vector_to_json_case3());
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_to_json);

static int test_vector_assign_case1()
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int capacity = 19, value_type = VALUE_TYPE_INT8_T;
    int ret;
    char *init_data = "[0, 1, 2, 3, 4, 5]";

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        vector->set(vector, "/Vector/capacity", &capacity);
        vector->assign(vector,  init_data);

        THROW_IF(strcmp(vector->to_json(vector), init_data) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_assign_case2()
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int capacity = 19, value_type = VALUE_TYPE_STRING;
    uint8_t trustee_flag = 1;
    int ret;
    char *init_data = "[\"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        vector->set(vector, "/Vector/capacity", &capacity);
        vector->assign(vector,  init_data);

        dbg_str(DBG_DETAIL, "Vector dump: %s", vector->to_json(vector));
        THROW_IF(strcmp(vector->to_json(vector), init_data) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_assign_case3()
{
    int ret;
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int capacity = 19, value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";
    String *string, **addr;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        addr = vector->get(vector, "/Vector/class_name");
        vector->assign(vector, init_data);

        string = object_new(allocator, "String", NULL);
        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);

        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
        object_destroy(vector);
    }

    return ret;
}

static int test_vector_assign(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_vector_assign_case1());
        EXEC(test_vector_assign_case2());
        EXEC(test_vector_assign_case3());
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_vector_assign);

static int test_vector_new(TEST_ENTRY *entry)
{
    int ret;
    Vector *vector = NULL;
    String *string = NULL;
    allocator_t *allocator = allocator_get_default_instance();
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1}, {\"name\":\"simplest obj2\",\"help\":2}]";

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, "(Simplest_Obj):");
        string->append(string, init_data, -1);
        vector = object_new(allocator, "Vector", STR2A(string));

        string->assign(string, vector->to_json(vector));
        string->replace(string, "\t", "", -1);
        string->replace(string, "\r", "", -1);
        string->replace(string, "\n", "", -1);
        dbg_str(DBG_DETAIL, "expect:%s, result:%s", string->get_cstr(string), init_data);

        THROW_IF(strcmp(string->get_cstr(string), init_data) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
        object_destroy(string);
    }
}
REGISTER_TEST_FUNC(test_vector_new);

static int __int_vector_element_cmp(void *element, void *key)
{
    int data;

    data = (int)element;

    return data == *(int *)key ? 1 : 0;
}

static int test_vector_search(TEST_ENTRY *entry)
{
    Vector *vector;
    int value_type = VALUE_TYPE_INT8_T;
    allocator_t *allocator = allocator_get_default_instance();
    int t = 2;
    int ret, count, expect_count = 5, index;
    void *element;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        EXEC(vector->search(vector, __int_vector_element_cmp, &t, &element, &index));
        SET_CATCH_INT_PARS(index, t);
        THROW_IF(index != 2, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return 1;
}
REGISTER_TEST_FUNC(test_vector_search);

static int test_vector_get_end_index(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        ret = vector->get_end_index(vector);
        SET_CATCH_INT_PARS(ret, 0);
        THROW_IF(ret != 5, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_get_end_index);

static int test_vector_remove_back(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);

        vector->add_at(vector, 0, 0);
        vector->add_at(vector, 1, 1);
        vector->add_at(vector, 2, 2);
        vector->add_at(vector, 3, 3);
        vector->add_at(vector, 4, 4);

        EXEC(vector->remove_back(vector, &element));
        ret = vector->get_end_index(vector);
        SET_CATCH_INT_PARS(ret, 0);
        THROW_IF(ret != 4, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_remove_back);

static int __vector_int_cmp(void *e1, void *e2)
{
    return (int)e1 > (int)e2 ? 1 : 0;
}

static int test_vector_sort(TEST_ENTRY *entry)
{
    Vector *vector;
    allocator_t *allocator = allocator_get_default_instance();
    int value_type = VALUE_TYPE_INT8_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *expect = "[-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500]";

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->set(vector, "/Vector/value_type", &value_type);
        vector->assign(vector,  init_data);
        dbg_str(DBG_DETAIL, "vector json:%s", vector->to_json(vector));
        vector->sort(vector, 0, __vector_int_cmp);
        SET_CATCH_STR_PARS(vector->to_json(vector), expect);
        THROW_IF(strcmp(vector->to_json(vector), expect) != 0, -1);
        dbg_str(DBG_DETAIL, "vector json:%s", vector->to_json(vector));
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
    }

    return 1;
}
REGISTER_TEST_FUNC(test_vector_sort);

static int __test_vector_filter_condition(void *element, void *cond)
{
    int c = *(int *)cond;
    int e = (int)element;

    return e > c ? 1 : 0;
}

static int test_vector_filter(TEST_ENTRY *entry)
{
    Vector *vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *expect  = "[900, 2, 3, 34, 76, 32, 43, 56, 35, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_instance();
    int cond = 0;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        out = object_new(allocator, "Vector", NULL);
        out->set(out, "value_type", &value_type);

        vector->set(vector, "value_type", &value_type);
        vector->assign(vector,  init_data);
        dbg_str(DBG_DETAIL, "vector before filter:%s", vector->to_json(vector));
        EXEC(vector->filter(vector, __test_vector_filter_condition, &cond, out));

        SET_CATCH_STR_PARS(init_data, out->to_json(out));
        THROW_IF(strcmp(out->to_json(out), expect) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
        object_destroy(out);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_filter);

static int test_vector_add_vector(TEST_ENTRY *entry)
{
    Vector *vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data1  = "[900, 2, 3, -58, 34, 76, 32, 43]";
    char *init_data2  = "[56, -70, 35, -234, 532, 543, 2500]";
    char *expect  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_instance();
    int cond = 0;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        out = object_new(allocator, "Vector", init_data2);
        vector->assign(vector,  init_data1);

        dbg_str(DBG_DETAIL, "vector src json:%s", out->to_json(out));
        dbg_str(DBG_DETAIL, "vector dst json:%s", vector->to_json(vector));
        EXEC(vector->add_vector(vector, out));

        SET_CATCH_STR_PARS(expect, vector->to_json(vector));
        THROW_IF(strcmp(vector->to_json(vector), expect) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
        object_destroy(out);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_vector_add_vector);

static int test_vector_copy_case1()
{
    Vector *vector, *out = NULL;
    int value_type = VALUE_TYPE_INT16_T;
    uint8_t trustee_flag = 1;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *expect  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    char *init_data  = "[900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500]";
    allocator_t *allocator = allocator_get_default_instance();
    int cond = 0;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        out = object_new(allocator, "Vector", NULL);
        out->set(out, "/Vector/trustee_flag", &trustee_flag);
        out->set(out, "/Vector/value_type", &value_type);

        EXEC(vector->assign(vector, init_data));
        EXEC(vector->copy(vector, out));

        SET_CATCH_STR_PARS(expect, out->to_json(out));
        THROW_IF(strcmp(out->to_json(out), expect) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
        object_destroy(out);
    }

    return ret;
}

static int test_vector_copy_case2()
{
    Vector *vector, *out = NULL;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    int t = 2;
    int ret, count, expect_count = 5;
    void *element = NULL;
    char *init_data = "[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]";
    char *expect = "[{\"name\":\"simplest obj1\",\"help\":1},{\"name\":\"simplest obj2\",\"help\":2}]";
    allocator_t *allocator = allocator_get_default_instance();
    String *json;
    int cond = 0;

    TRY {
        vector = object_new(allocator, "Vector", NULL);
        vector->reset(vector);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/class_name", "Simplest_Obj");
        vector->assign(vector,  init_data);

        out = object_new(allocator, "Vector", NULL);
        THROW_IF(out == NULL, -1);
        out->set(out, "/Vector/trustee_flag", &trustee_flag);
        out->set(out, "/Vector/value_type", &value_type);
        out->set(out, "/Vector/class_name", "Simplest_Obj");

        EXEC(vector->copy(vector, out));
        out->to_json(out);
        json = ((Obj *)out)->json;
        json->replace(json, "\t", "" , -1);
        json->replace(json, "\r", "" , -1);
        json->replace(json, "\n", "" , -1);
        json->replace(json, ", ", ",", -1);
        dbg_str(DBG_WARNNING, "json:%s", STR2A(json));

        SET_CATCH_STR_PARS(expect, STR2A(json));
        THROW_IF(strcmp(STR2A(json), expect) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(vector);
        object_destroy(out);
    }

    return ret;
}

static int test_vector_copy(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_vector_copy_case1());
        EXEC(test_vector_copy_case2());
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_vector_copy);