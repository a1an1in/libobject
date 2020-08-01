/**
 * @file iter.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Iterator.h>

static int __construct(Iterator *iter, char *init_str)
{
    dbg_str(OBJ_DETAIL, "iter construct, iter addr:%p", iter);

    return 1;
}

static int __deconstrcut(Iterator *iter)
{
    dbg_str(OBJ_DETAIL, "iter deconstruct, iter addr:%p", iter);

    return 1;
}

static Iterator *__next(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator next");
}

static Iterator *__prev(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator prev");
}

static int __equal(Iterator *it1, Iterator *it2)
{
    dbg_str(OBJ_DETAIL, "Iterator equal");
}

static void *__get_vpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator get_vpointer");
}

static void *__get_kpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator get_kpointer");
}

static class_info_entry_t iter_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Iterator, construct, __construct),
    Init_Nfunc_Entry(2 , Iterator, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Iterator, next, __next),
    Init_Vfunc_Entry(4 , Iterator, prev, __prev),
    Init_Vfunc_Entry(5 , Iterator, equal, __equal),
    Init_Vfunc_Entry(6 , Iterator, get_vpointer, __get_vpointer),
    Init_Vfunc_Entry(7 , Iterator, get_kpointer, NULL),
    Init_Vfunc_Entry(8 , Iterator, is_null, NULL),
    Init_Vfunc_Entry(9 , Iterator, clear, NULL),
    Init_End___Entry(10, Iterator),
};
REGISTER_CLASS("Iterator", iter_class_info);

void test_obj_iter()
{
    Iterator *iter, *next, *prev;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str = NULL;
    cjson_t *root, *e, *s;
    char buf[2048];

    iter = OBJECT_NEW(allocator, Iterator, set_str);
    /*
     *next = OBJECT_NEW(allocator, Iterator, set_str);
     *prev = OBJECT_NEW(allocator, Iterator, set_str);
     */

    iter->next(iter);
    iter->prev(iter);
}


