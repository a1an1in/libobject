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
#include <libobject/core/Linked_List.h>
#include <libobject/core/String.h>
#include <libobject/mockery/mockery.h>

static int test_linked_list_count(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    List *list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 5, ret;

    TRY {
        list = object_new(allocator, "Linked_List", NULL);

        list->add_back(list, str1);
        list->add_back(list, str2);
        list->add_back(list, str3);
        list->add_back(list, str4);
        list->add_back(list, str5);

        count = list->count(list);
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_count);

static int test_linked_list_remove(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    List *list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 4, ret;

    TRY {
        list = object_new(allocator, "Linked_List", NULL);

        list->add_back(list, str1);
        list->add_back(list, str2);
        list->add_back(list, str3);
        list->add_back(list, str4);
        list->add_back(list, str5);
        list->remove(list, NULL);

        count = list->count(list);
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_remove);

static int test_linked_list_reset(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    List *list;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";
    int count, expect_count = 0, ret;

    TRY {
        list = object_new(allocator, "Linked_List", NULL);

        list->add_back(list, str1);
        list->add_back(list, str2);
        list->add_back(list, str3);
        list->add_back(list, str4);
        list->add_back(list, str5);

        list->reset(list);
        count = list->count(list);
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_reset);

static int test_linked_list_reset_string_value(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    List *list;
    String *t0, *t1, *t2, *t3, *t4, *t5;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_STRING_POINTER;
    int count, expect_count = 0, expect_alloc_count, ret;

    TRY {
        list = object_new(allocator, "Linked_List", NULL);
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

        list->reset(list);

        count = list->count(list);
        THROW_IF(count != expect_count, -1);

        count = allocator->alloc_count;
        THROW_IF(count != expect_alloc_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_reset_string_value);
