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
 9999* documentation and/or other materials provided with the distribution.
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
#include <libobject/core/list.h>

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

static int __add_back(List *list, void *value)
{
    dbg_str(OBJ_DETAIL, "List insert");
}

static int __delete(List *list, Iterator *iter)
{
    dbg_str(OBJ_DETAIL, "List delete");
}

static int __remove(List *list, Iterator *iter, void **data)
{
    dbg_str(OBJ_DETAIL, "List remove");
}

static int __remove_back(List *list, void **data)
{
    dbg_str(OBJ_DETAIL, "List remove back");
}

static int __remove_all(List *list)
{
    dbg_str(OBJ_DETAIL, "List remove all");
    while (list->count(list)) {
        dbg_str(OBJ_DETAIL, "List remove a element");
        list->remove_front(list, NULL);
    }
}

static int __remove_element(List *list, void *data)
{
    dbg_str(OBJ_DETAIL, "List remove element");
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
        dbg_str(DBG_IMPORTANT, "List for_each, element=%p", element);
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

static Iterator *__begin(List *list)
{
    dbg_str(OBJ_DETAIL, "List begin");
}

static Iterator *__end(List *list)
{
    dbg_str(OBJ_DETAIL, "List end");
}

static class_info_entry_t list_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , List, construct, __construct),
    Init_Nfunc_Entry(2 , List, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , List, add, NULL),
    Init_Vfunc_Entry(4 , List, add_front, NULL),
    Init_Vfunc_Entry(5 , List, add_back, __add_back),
    Init_Vfunc_Entry(6 , List, remove, __remove),
    Init_Vfunc_Entry(7 , List, remove_front, NULL),
    Init_Vfunc_Entry(8 , List, remove_back, __remove_back),
    Init_Vfunc_Entry(9 , List, remove_all, __remove_all),
    Init_Vfunc_Entry(10, List, remove_element, __remove_element),
    Init_Vfunc_Entry(11, List, count, NULL),
    Init_Vfunc_Entry(12, List, delete, __delete),
    Init_Vfunc_Entry(13, List, detach_front, NULL),
    Init_Vfunc_Entry(14, List, free_detached, NULL),
    Init_Vfunc_Entry(15, List, for_each, __for_each),
    Init_Vfunc_Entry(16, List, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(17, List, begin, __begin),
    Init_Vfunc_Entry(18, List, end, __end),
    Init_Vfunc_Entry(19, List, destroy, NULL),
    Init_U32___Entry(20, List, value_size, NULL),
    Init_End___Entry(21),
};
REGISTER_CLASS("List", list_class_info);

void test_obj_list()
{
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "List", e = cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str = cjson_print(root);

    list = OBJECT_NEW(allocator, List, set_str);

    object_dump(list, "List", buf, 2048);
    dbg_str(OBJ_DETAIL, "List dump: %s", buf);

    free(set_str);

}


