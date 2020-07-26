/**
 * @file Trie_Test.c
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
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "Trie_Test.h"

static int __construct(Trie_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    test->trie = object_new(allocator, "Trie", NULL);

    return 0;
}

static int __deconstrcut(Trie_Test *test)
{
    object_destroy(test->trie);

    return 0;
}

static int __setup(Trie_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    dbg_str(DBG_DETAIL,"Trie_Test set up");

    return 0;
}

static int __teardown(Trie_Test *test)
{
    dbg_str(DBG_DETAIL,"Trie_Test teardown");

    return 0;
}

static int __test_insert(Trie_Test *test)
{
    Trie *trie = test->trie;
    int ret, expect_count;

    ret = trie->insert(trie, "hello");
    expect_count = 1;
    ASSERT_EQUAL(test, &ret, &expect_count, sizeof(expect_count));
    ret = trie->insert(trie, "world");
    expect_count = 1;
    ret = trie->insert(trie, "hello");
    expect_count = 2;
    dbg_str(DBG_DETAIL,"ret=%d, expect_count=%d", ret, expect_count);
    ASSERT_EQUAL(test, &ret, &expect_count, sizeof(expect_count));
}

static int __test_search(Trie_Test *test)
{
    Trie *trie = test->trie;
    int ret, expect_count;

    ret = trie->search(trie, "hello");
    expect_count = 2;
    ASSERT_EQUAL(test, &ret, &expect_count, sizeof(expect_count));
}

static int __test_search_prefix(Trie_Test *test)
{
    return 1;
}

static int __test_delete(Trie_Test *test)
{
    Trie *trie = test->trie;
    int ret, expect_count;

    trie->insert(trie, "hello");
    ret = trie->delete(trie, "hello");
    expect_count = 2;
    ASSERT_EQUAL(test, &ret, &expect_count, sizeof(expect_count));
}

static class_info_entry_t trie_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Trie_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Trie_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Trie_Test, set, NULL),
    Init_Vfunc_Entry(4 , Trie_Test, get, NULL),
    Init_Vfunc_Entry(5 , Trie_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Trie_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Trie_Test, test_insert, __test_insert),
    Init_Vfunc_Entry(8 , Trie_Test, test_search, __test_search),
    Init_Vfunc_Entry(9 , Trie_Test, test_search_prefix, __test_search_prefix),
    Init_Vfunc_Entry(10, Trie_Test, test_delete, __test_delete),
    Init_End___Entry(11, Trie_Test),
};
/*
 *REGISTER_CLASS("Trie_Test", trie_test_class_info);
 */
