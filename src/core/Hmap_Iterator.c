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
#include <libobject/core/Hmap_Iterator.h>

static int __construct(Iterator *iter, char *init_str)
{
    Hmap_Iterator *hiter;
    dbg_str(OBJ_DETAIL, "Hmap_Iterator construct, iter addr:%p", iter);

    return 1;
}

static int __deconstrcut(Iterator *iter)
{
    Hmap_Iterator *hiter;
    dbg_str(OBJ_DETAIL, "Hmap_Iterator deconstruct, iter addr:%p", iter);

    return 1;
}

static Iterator *__next(Iterator *it)
{
    Iterator *next = it;
    dbg_str(OBJ_DETAIL, "Hmap_Iterator next");

    hash_map_pos_next(&((Hmap_Iterator *)it)->hash_map_pos, 
                      &((Hmap_Iterator *)next)->hash_map_pos);

    return next;

}

static Iterator *__prev(Iterator *it)
{
    Hmap_Iterator *hiter = (Hmap_Iterator *)it;
    dbg_str(OBJ_DETAIL, "Hmap_Iterator prev, this func is not implemented");
}

static int __equal(Iterator *it1, Iterator *it2)
{
    dbg_str(OBJ_DETAIL, "Hmap_Iterator equal");

    return hash_map_pos_equal(&((Hmap_Iterator *)it1)->hash_map_pos, 
                              &((Hmap_Iterator *)it2)->hash_map_pos);
}

static void *__get_vpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Hmap_Iterator get_vpointer");
    return hash_map_pos_get_pointer(&((Hmap_Iterator *)it)->hash_map_pos);
}

static void *__get_kpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator get_kpointer");
    return hash_map_pos_get_kpointer(&((Hmap_Iterator *)it)->hash_map_pos);
}

static class_info_entry_t hmap_iter_class_info[] = {
    Init_Obj___Entry(0, Iterator, iter),
    Init_Nfunc_Entry(1, Hmap_Iterator, construct, __construct),
    Init_Nfunc_Entry(2, Hmap_Iterator, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, Hmap_Iterator, next, __next),
    Init_Vfunc_Entry(4, Hmap_Iterator, prev, __prev),
    Init_Vfunc_Entry(5, Hmap_Iterator, equal, __equal),
    Init_Vfunc_Entry(6, Hmap_Iterator, get_vpointer, __get_vpointer),
    Init_Vfunc_Entry(7, Hmap_Iterator, get_kpointer, __get_kpointer),
    Init_End___Entry(8, Hmap_Iterator),
};
REGISTER_CLASS(Hmap_Iterator, hmap_iter_class_info);


