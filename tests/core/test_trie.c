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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Trie.h>

static int test_trie_insert(TEST_ENTRY *entry, void *argc, void *argv)
{
    Trie *trie;
    allocator_t *allocator = allocator_get_default_instance();
    int ret, expect_count;

    TRY {
        trie = object_new(allocator, "Trie", NULL);
        ret = trie->insert(trie, "hello");
        expect_count = 1;
        THROW_IF(ret != expect_count, -1);

        ret = trie->insert(trie, "world");
        expect_count = 1;
        ret = trie->insert(trie, "hello");
        expect_count = 2;
        dbg_str(DBG_DETAIL,"ret=%d, expect_count=%d", ret, expect_count);
        THROW_IF(ret != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(trie);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_trie_insert);

static int test_trie_search(TEST_ENTRY *entry, void *argc, void *argv)
{
    Trie *trie;
    allocator_t *allocator = allocator_get_default_instance();
    int ret, expect_count;

    TRY {
        trie = object_new(allocator, "Trie", NULL);
        EXEC(trie->insert(trie, "hello"));
        EXEC(trie->insert(trie, "world"));
        EXEC(trie->insert(trie, "hello"));

        ret = trie->search(trie, "hello");
        expect_count = 2;
        THROW_IF(ret != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(trie);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_trie_search);

static int test_trie_delete(TEST_ENTRY *entry, void *argc, void *argv)
{
    Trie *trie;
    allocator_t *allocator = allocator_get_default_instance();
    int ret, expect_count;

    TRY {
        trie = object_new(allocator, "Trie", NULL);
        trie->insert(trie, "hello");
        trie->insert(trie, "hello");
        trie->insert(trie, "hello");
        ret = trie->delete(trie, "hello");
        expect_count = 2;
        THROW_IF(ret != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(trie);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_trie_delete);

