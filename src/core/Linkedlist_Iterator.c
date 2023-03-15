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
#include <libobject/core/Linkedlist_Iterator.h>

static Iterator *__next(Iterator *it)
{
    Iterator *next = it;

    dbg_str(OBJ_DETAIL, "LList_Iterator next");

    llist_pos_next(&((LList_Iterator *)it)->list_pos, 
                   &((LList_Iterator *)next)->list_pos);

    return next;

}

static Iterator *__prev(Iterator *it)
{
    Iterator *prev = it;

    dbg_str(OBJ_DETAIL, "LList_Iterator prev");

    llist_pos_prev(&((LList_Iterator *)it)->list_pos, 
                   &((LList_Iterator *)prev)->list_pos);

    return prev;
}

static int __equal(Iterator *it1, Iterator *it2)
{
    dbg_str(OBJ_DETAIL, "LList_Iterator equal");

    return llist_pos_equal(&((LList_Iterator *)it1)->list_pos, 
                           &((LList_Iterator *)it2)->list_pos);
}

static void *__get_vpointer(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "LList_Iterator get_vpointer");
    return llist_pos_get_pointer(&((LList_Iterator *)it)->list_pos);;
}

static int __is_null(Iterator *it)
{
    dbg_str(OBJ_DETAIL, "LList_Iterator get_vpointer");
    return ((LList_Iterator *)it)->list_pos.list_head_p == NULL;
}

static int __clear(Iterator *it)
{
    ((LList_Iterator *)it)->list_pos.list_head_p = NULL;
}

static class_info_entry_t hmap_iter_class_info[] = {
    Init_Obj___Entry(0, Iterator, iter),
    Init_Vfunc_Entry(1, LList_Iterator, next, __next),
    Init_Vfunc_Entry(2, LList_Iterator, prev, __prev),
    Init_Vfunc_Entry(3, LList_Iterator, equal, __equal),
    Init_Vfunc_Entry(4, LList_Iterator, get_vpointer, __get_vpointer),
    Init_Vfunc_Entry(5, LList_Iterator, is_null, __is_null),
    Init_Vfunc_Entry(6, LList_Iterator, clear, __clear),
    Init_End___Entry(7, LList_Iterator),
};
REGISTER_CLASS("LList_Iterator", hmap_iter_class_info);

void test_obj_linked_list_iter()
{
    Iterator *iter, *next, *prev;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str = NULL;
    cjson_t *root, *e, *s;
    char buf[2048];

    iter = OBJECT_NEW(allocator, LList_Iterator, set_str);
    next = OBJECT_NEW(allocator, LList_Iterator, set_str);

    iter->next(iter);
}


