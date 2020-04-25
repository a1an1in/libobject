/**
 * @file Object_Cache_Test.c
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
#include <libobject/core/Vector.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "Object_Cache_Test.h"

static int __construct(Object_Cache_Test *test, char *init_str)
{
    test->cache = object_new(test->parent.obj.allocator, "Object_Cache", NULL);
    return 0;
}

static int __deconstrcut(Object_Cache_Test *test)
{
    object_destroy(test->cache);
    return 0;
}

static int __setup(Object_Cache_Test *test, char *init_str)
{
    return 0;
}

static int __teardown(Object_Cache_Test *test)
{
    return 0;
}

static int __test_memery_leak(Object_Cache_Test *test)
{
    Object_Cache *cache = test->cache;
    String *s0;
    String *s1;
    String *s2;
    String *s3;
    String *s4;
    Vector *v0;
    Vector *v1;
    Vector *v2;
    Vector *v3;
    Vector *v4;
    allocator_t *allocator = test->parent.obj.allocator;
    int ret, pre_count, after_count;

    object_destroy(test->cache);

    pre_count = allocator->alloc_count;
    cache = object_new(allocator, "Object_Cache", NULL);
    test->cache = cache;

    s0 = cache->new(cache, "String", NULL);
    s1 = cache->new(cache, "String", NULL);
    s2 = cache->new(cache, "String", NULL);
    s3 = cache->new(cache, "String", NULL);
    s4 = cache->new(cache, "String", NULL);

    v0 = cache->new(cache, "Vector", NULL);
    v1 = cache->new(cache, "Vector", NULL);
    v2 = cache->new(cache, "Vector", NULL);
    v3 = cache->new(cache, "Vector", NULL);
    v4 = cache->new(cache, "Vector", NULL);

    object_destroy(s0);
    object_destroy(s1);
    object_destroy(s2);
    object_destroy(s3);

    object_destroy(v0);
    object_destroy(v1);
    object_destroy(v3);
    object_destroy(v4);

    s0 = cache->new(cache, "String", NULL);
    s1 = cache->new(cache, "String", NULL);
    s2 = cache->new(cache, "String", NULL);
    s3 = cache->new(cache, "String", NULL);
    s4 = cache->new(cache, "String", NULL);

    v0 = cache->new(cache, "Vector", NULL);
    v1 = cache->new(cache, "Vector", NULL);
    v2 = cache->new(cache, "Vector", NULL);
    v3 = cache->new(cache, "Vector", NULL);
    v4 = cache->new(cache, "Vector", NULL);

    object_destroy(test->cache);
    after_count = allocator->alloc_count;

    cache = object_new(allocator, "Object_Cache", NULL);
    test->cache = cache;

    ret = ASSERT_EQUAL(test, &pre_count, &after_count, sizeof(after_count));
    if (ret != 1) {
        dbg_str(DBG_ERROR, "pre_count=%d after_count=%d", pre_count, after_count);
    }
    
}

static class_info_entry_t object_cache_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Object_Cache_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Object_Cache_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Object_Cache_Test, set, NULL),
    Init_Vfunc_Entry(4 , Object_Cache_Test, get, NULL),
    Init_Vfunc_Entry(5 , Object_Cache_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Object_Cache_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Object_Cache_Test, test_memery_leak, __test_memery_leak),
    Init_End___Entry(8 , Object_Cache_Test),
};
REGISTER_CLASS("Object_Cache_Test", object_cache_test_class_info);
