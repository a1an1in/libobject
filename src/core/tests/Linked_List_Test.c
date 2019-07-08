/**
 * @file Linked_List_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/tests/Linked_List_Test.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/tests/Linked_List_Test.h>

static int __construct(Linked_List_Test *test, char *init_str)
{
    test->list = object_new(test->parent.obj.allocator, "Linked_List", NULL);
    return 0;
}

static int __deconstrcut(Linked_List_Test *test)
{
    object_destroy(test->list);
    return 0;
}

static int __setup(Linked_List_Test *test, char *init_str)
{
    dbg_str(DBG_DETAIL,"Linked_List_Test set up");

    return 0;
}

static int __teardown(Linked_List_Test *test)
{
    List *list = test->list;

    dbg_str(DBG_DETAIL,"Linked_List_Test teardown");
    list->clear(list);

    return 0;
}

static int __test_add(Linked_List_Test *test)
{
    return 1;
}

static int __test_count(Linked_List_Test *test)
{
    allocator_t *allocator = test->parent.obj.allocator;
    List *list = test->list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 5;

    list->add_back(list, str1);
    list->add_back(list, str2);
    list->add_back(list, str3);
    list->add_back(list, str4);
    list->add_back(list, str5);

    count = list->count(list);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_remove(Linked_List_Test *test)
{
    allocator_t *allocator = test->parent.obj.allocator;
    List *list = test->list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 4;

    list->add_back(list, str1);
    list->add_back(list, str2);
    list->add_back(list, str3);
    list->add_back(list, str4);
    list->add_back(list, str5);
    list->remove(list, NULL);

    count = list->count(list);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_clear(Linked_List_Test *test)
{
    allocator_t *allocator = test->parent.obj.allocator;
    List *list = test->list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 0;

    list->add_back(list, str1);
    list->add_back(list, str2);
    list->add_back(list, str3);
    list->add_back(list, str4);
    list->add_back(list, str5);

    list->clear(list);
    count = list->count(list);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_clear_string_value(Linked_List_Test *test)
{
    allocator_t *allocator = test->parent.obj.allocator;
    List *list = test->list;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_STRING;
    int count, expect_count = 0, expect_alloc_count;

    expect_alloc_count = allocator->alloc_count;

    list->set(list, "/List/trustee_flag", &trustee_flag);
    list->set(list, "/List/value_type", &value_type);

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

    list->add_back(list, t0);
    list->add_back(list, t1);
    list->add_back(list, t2);
    list->add_back(list, t3);
    list->add_back(list, t4);
    list->add_back(list, t5);

    list->clear(list);

    count = list->count(list);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));

    count = allocator->alloc_count;
    ASSERT_EQUAL(test, &count, &expect_alloc_count, sizeof(count));

}

static class_info_entry_t linked_list_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Linked_List_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Linked_List_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Linked_List_Test, set, NULL),
    Init_Vfunc_Entry(4 , Linked_List_Test, get, NULL),
    Init_Vfunc_Entry(5 , Linked_List_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Linked_List_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Linked_List_Test, test_add, __test_add),
    Init_Vfunc_Entry(8 , Linked_List_Test, test_count, __test_count),
    Init_Vfunc_Entry(9 , Linked_List_Test, test_remove, __test_remove),
    Init_Vfunc_Entry(10, Linked_List_Test, test_clear, __test_clear),
    Init_Vfunc_Entry(11, Linked_List_Test, test_clear_string_value, __test_clear_string_value),
    Init_End___Entry(12, Linked_List_Test),
};
REGISTER_CLASS("Linked_List_Test", linked_list_test_class_info);
