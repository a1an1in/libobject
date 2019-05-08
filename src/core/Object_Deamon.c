/**
 * @file Object_Deamon.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-04-27
 */
/* 
 *  Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <libobject/core/Object_Deamon.h>
#include <libobject/core/utils/dbg/debug.h>

static Object_Cache *
__get_cache(Object_Deamon *deamon, char *cache_name)
{
    dbg_str(DBG_DETAIL, "get a object cache");
}

static class_info_entry_t object_deamon_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Vfunc_Entry(1 , Object_Deamon, get_cache, __get_cache),
    Init_End___Entry(2 , Object_Deamon),
};
REGISTER_CLASS("Object_Deamon", object_deamon_info);

static int test_object_deamon_get_cache()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Object_Deamon *deamon;
    int ret = 1;

    deamon = object_new(allocator, "Object_Deamon", NULL);

    deamon->get_cache(deamon, "http cache");

    object_destroy(deamon);

    return ret;
}

REGISTER_TEST_FUNC(test_object_deamon_get_cache);
