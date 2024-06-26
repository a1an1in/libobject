/**
 * @file list.c
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Linked_List.h>

static int __construct(List *list, char *init_str)
{
    llist_t *llist;
    allocator_t *allocator = ((Obj *)list)->allocator;
    int lock_type = 0;

    llist = llist_alloc(allocator);
    llist_set(llist, "lock_type", &lock_type);
    llist_init(llist);

    ((Linked_List *)list)->llist = llist;

    list->b = OBJECT_NEW(allocator, LList_Iterator, NULL);
    list->e = OBJECT_NEW(allocator, LList_Iterator, NULL);

    return 1;
}

static int __deconstrcut(List *list)
{
    list->reset(list);
    object_destroy(list->b);
    object_destroy(list->e);
    llist_destroy(((Linked_List *)list)->llist);

    return 1;
}

static int __add(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    return llist_add_back(l->llist, value);
}

static int __add_back(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    return llist_add_back(l->llist, value);
}

static int __add_front(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    return llist_add_front(l->llist, value);
}

static int __delete(List *list)
{
    Linked_List *l = (Linked_List *)list;

    return llist_delete(l->llist, &l->llist->begin);
}

static int __remove(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    return llist_remove_front(l->llist, data);
}

static int __remove_front(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    return llist_remove_front(l->llist, data);
}

static int __remove_back(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    return llist_remove_back(l->llist, data);
}

static int __remove_element(List *list, void *data)
{
    Linked_List *l = (Linked_List *)list;

    return llist_remove_element(l->llist, data);
}

static int __count(List *list)
{
    Linked_List *l = (Linked_List *)list;

    return llist_get_count(l->llist);
}

static int __detach_front(List *list, void **data)
{
    Linked_List *ll = (Linked_List *)list;
    list_t *l;
    void *ret;

    TRY {
        l = llist_detach_front(ll->llist);
        THROW_IF(l == NULL, -1);

        *data = l->data;
        allocator_mem_free(ll->llist->allocator, l);
    } CATCH (ret) {}

    return ret;
}


static Iterator *__begin(List *list)
{
    Linked_List *l       = (Linked_List *)list;
    LList_Iterator *iter = (LList_Iterator *)list->b;

    llist_begin(l->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static Iterator *__end(List *list)
{
    Linked_List *l       = (Linked_List *)list;
    LList_Iterator *iter = (LList_Iterator *)list->e;

    llist_end(l->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static class_info_entry_t llist_class_info[] = {
    Init_Obj___Entry(0 , List, list),
    Init_Nfunc_Entry(1 , Linked_List, construct, __construct),
    Init_Nfunc_Entry(2 , Linked_List, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Linked_List, add, __add),
    Init_Vfunc_Entry(4 , Linked_List, add_front, __add_front),
    Init_Vfunc_Entry(5 , Linked_List, add_back, __add_back),
    Init_Vfunc_Entry(6 , Linked_List, remove, __remove),
    Init_Vfunc_Entry(7 , Linked_List, remove_front, __remove_front),
    Init_Vfunc_Entry(8 , Linked_List, remove_back, __remove_back),
    Init_Vfunc_Entry(9 , Linked_List, remove_element, __remove_element),
    Init_Vfunc_Entry(10, Linked_List, count, __count),
    Init_Vfunc_Entry(11, Linked_List, delete, __delete),
    Init_Vfunc_Entry(12, Linked_List, detach_front, __detach_front),
    Init_Vfunc_Entry(13, Linked_List, begin, __begin),
    Init_Vfunc_Entry(14, Linked_List, end, __end),
    Init_End___Entry(15, Linked_List),
};
REGISTER_CLASS(Linked_List, llist_class_info);
