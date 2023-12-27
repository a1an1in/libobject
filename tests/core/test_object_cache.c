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
#include <libobject/core/Object_Cache.h>
#include <libobject/core/utils/registry/registry.h>

static int test_object_cache_memery_leak(TEST_ENTRY *entry)
{
    Object_Cache *cache;
    String *s0, *s1, *s2, *s3, *s4;
    Vector *v0, *v1, *v2, *v3, *v4;
    allocator_t *allocator = allocator_get_default_instance();
    int ret, pre_count, after_count;

    TRY {
        sleep(2);
        pre_count = allocator->alloc_count;
        cache = object_new(allocator, "Object_Cache", NULL);

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
        object_destroy(cache);

        after_count = allocator->alloc_count;
        SET_CATCH_INT_PARS(after_count, pre_count);
        THROW_IF(after_count != pre_count, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_VIP); 
    } FINALLY {}
    
    return ret;
}
REGISTER_TEST_FUNC(test_object_cache_memery_leak);