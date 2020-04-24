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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/List.h>

static int __construct(List *list, char *init_str)
{
    dbg_str(OBJ_DETAIL, "list construct, list addr:%p", list);

    return 0;
}

static int __deconstrcut(List *list)
{
    dbg_str(OBJ_DETAIL, "list deconstruct, list addr:%p", list);

    return 0;
}

static int __reset(List *list)
{
    void **element;

    dbg_str(OBJ_DETAIL, "List reset");

    while (list->count(list)) {
        list->remove_front(list, (void **)&element);
        if (list->trustee_flag != 1) {
            continue;
        }

        if (list->value_type == VALUE_TYPE_OBJ_POINTER && element != NULL) {
            dbg_str(OBJ_DETAIL, "reset list obj element, class name:%s",
                    ((Obj *)element)->name);
            object_destroy(element);
        } else if (list->value_type  == VALUE_TYPE_STRING &&
                   element != NULL) {
            dbg_str(OBJ_DETAIL, "reset list string element");
            object_destroy(element);
        } else if (list->value_type  == VALUE_TYPE_ALLOC_POINTER &&
                   element != NULL) {
            allocator_mem_free(list->obj.allocator, element);
        } else if (list->value_type  == VALUE_TYPE_UNKNOWN_POINTER &&
                   element != NULL) {
            dbg_str(OBJ_WARNNING, "not support reset unkown pointer");
        } else {
        }

        element = NULL;
    }
}

static int __is_empty(List *list)
{
    return  list->count(list) == 0 ? 1 : 0;
}

static void __for_each(List *list, void (*func)(void *element))
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "List for_each");
    cur = list->begin(list);
    end = list->end(list);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        /*
         *dbg_str(OBJ_IMPORTANT, "List for_each, element=%p", element);
         */
        func(element);
    }
}

static void
__for_each_arg(List *list, void (*func)(void *element, void *arg), void *arg)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "List for_each arg2");
    cur = list->begin(list);
    end = list->end(list);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg);
    }
}

static class_info_entry_t list_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , List, construct, __construct),
    Init_Nfunc_Entry(2 , List, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , List, set, NULL),
    Init_Vfunc_Entry(4 , List, get, NULL),
    Init_Vfunc_Entry(5 , List, add, NULL),
    Init_Vfunc_Entry(6 , List, add_front, NULL),
    Init_Vfunc_Entry(7 , List, add_back, NULL),
    Init_Vfunc_Entry(8 , List, remove, NULL),
    Init_Vfunc_Entry(9 , List, remove_front, NULL),
    Init_Vfunc_Entry(10, List, remove_back, NULL),
    Init_Vfunc_Entry(11, List, remove_element, NULL),
    Init_Vfunc_Entry(12, List, reset, __reset),
    Init_Vfunc_Entry(13, List, count, NULL),
    Init_Vfunc_Entry(14, List, delete, NULL),
    Init_Vfunc_Entry(15, List, detach_front, NULL),
    Init_Vfunc_Entry(16, List, free_detached, NULL),
    Init_Vfunc_Entry(17, List, for_each, __for_each),
    Init_Vfunc_Entry(18, List, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(19, List, begin, NULL),
    Init_Vfunc_Entry(20, List, end, NULL),
    Init_Vfunc_Entry(21, List, destroy, NULL),
    Init_Vfunc_Entry(22, List, is_empty, __is_empty),
    Init_U8____Entry(23, List, trustee_flag, NULL),
    Init_U8____Entry(24, List, value_type, NULL),
    Init_End___Entry(25, List),
};
REGISTER_CLASS("List", list_class_info);
