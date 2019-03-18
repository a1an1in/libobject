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
#include <libobject/core/rbtree_iterator.h>

static int __construct(Iterator *iter, char *init_str)
{
    RBTree_Iterator *hiter;
    dbg_str(OBJ_DETAIL, "RBTree_Iterator construct, iter addr:%p", iter);

    return 0;
}

static int __deconstrcut(Iterator *iter)
{
    RBTree_Iterator *hiter;
    dbg_str(OBJ_DETAIL, "RBTree_Iterator deconstruct, iter addr:%p", iter);

    return 0;
}

static Iterator *__next(Iterator *it)
{
    Iterator *next = it;
    dbg_str(OBJ_DETAIL, "RBTree_Iterator next");

    rbtree_map_pos_next(&((RBTree_Iterator *)it)->rbtree_map_pos, 
                      &((RBTree_Iterator *)next)->rbtree_map_pos);

    return next;

}

static Iterator *__prev(Iterator *it)
{
    RBTree_Iterator *hiter = (RBTree_Iterator *)it;
    dbg_str(OBJ_DETAIL, "RBTree_Iterator prev, this func is not implemented");
}

static int __equal(Iterator *it1, Iterator *it2)
{
    dbg_str(OBJ_DETAIL, "RBTree_Iterator equal");

    return rbtree_map_pos_equal(&((RBTree_Iterator *)it1)->rbtree_map_pos, 
                              &((RBTree_Iterator *)it2)->rbtree_map_pos);
}

static void *__get_vpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "RBTree_Iterator get_vpointer");
    return rbtree_map_pos_get_pointer(&((RBTree_Iterator *)it)->rbtree_map_pos);
}

static void *__get_kpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "Iterator get_kpointer");
    return rbtree_map_pos_get_kpointer(&((RBTree_Iterator *)it)->rbtree_map_pos);

}

static class_info_entry_t rbtree_iter_class_info[] = {
    Init_Obj___Entry(0, Iterator, iter),
    Init_Nfunc_Entry(1, RBTree_Iterator, construct, __construct),
    Init_Nfunc_Entry(2, RBTree_Iterator, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, RBTree_Iterator, next, __next),
    Init_Vfunc_Entry(4, RBTree_Iterator, prev, __prev),
    Init_Vfunc_Entry(5, RBTree_Iterator, equal, __equal),
    Init_Vfunc_Entry(6, RBTree_Iterator, get_vpointer, __get_vpointer),
    Init_Vfunc_Entry(7, RBTree_Iterator, get_kpointer, __get_kpointer),
    Init_End___Entry(8),
};
REGISTER_CLASS("RBTree_Iterator", rbtree_iter_class_info);
