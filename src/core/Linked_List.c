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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/linked_list.h>
#include <libobject/core/utils/registry/registry.h>

static int __construct(List *list, char *init_str)
{
    llist_t *llist;
    allocator_t *allocator = ((Obj *)list)->allocator;
    int value_size;
    int lock_type = 0;

    dbg_str(OBJ_DETAIL, "llist list construct, list addr:%p", list);

    if (list->value_size == 0) {
        value_size = sizeof(void *);
        dbg_str(OBJ_DETAIL, "link list value is zero, we'll set it to default value 50");
    } else {
        value_size = list->value_size;
    }
    llist = llist_alloc(allocator);
    llist_set(llist, "lock_type", &lock_type);
    llist_set(llist, "data_size", &value_size);
    llist_init(llist);

    ((Linked_List *)list)->llist = llist;

    list->b = OBJECT_NEW(allocator, LList_Iterator, NULL);
    list->e = OBJECT_NEW(allocator, LList_Iterator, NULL);

    return 0;
}

static int __deconstrcut(List *list)
{
    dbg_str(OBJ_DETAIL, "llist list deconstruct, list addr:%p", list);

    object_destroy(list->b);
    object_destroy(list->e);
    llist_destroy(((Linked_List *)list)->llist);

    return 0;
}

static int __set(List *m, char *attrib, void *value)
{
    Linked_List *list = (Linked_List *)m;

    if (strcmp(attrib, "set") == 0) {
        list->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        list->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        list->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        list->deconstruct = value;
    } 
    else if (strcmp(attrib, "add") == 0) {
        list->add = value;
    } else if (strcmp(attrib, "add_front") == 0) {
        list->add_front = value;
    } else if (strcmp(attrib, "add_back") == 0) {
        list->add_back = value;
    } else if (strcmp(attrib, "remove") == 0) {
        list->remove = value;
    } else if (strcmp(attrib, "remove_front") == 0) {
        list->remove_front = value;
    } else if (strcmp(attrib, "remove_back") == 0) {
        list->remove_back = value;
    } else if (strcmp(attrib, "remove_element") == 0) {
        list->remove_element = value;
    } else if (strcmp(attrib, "count") == 0) {
        list->count = value;
    } else if (strcmp(attrib, "delete") == 0) {
        list->delete = value;
    } else if (strcmp(attrib, "detach_front") == 0) {
        list->detach_front = value;
    } else if (strcmp(attrib, "free_detached") == 0) {
        list->free_detached = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        list->for_each = value;
    } else if (strcmp(attrib, "begin") == 0) {
        list->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        list->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        list->destroy = value;
    }
    else if (strcmp(attrib, "name") == 0) {
        strncpy(list->name, value, strlen(value));
    } else {
        dbg_str(OBJ_WARNNING, "list set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(List *obj, char *attrib)
{
    Linked_List *list = (Linked_List *)obj;

    if (strcmp(attrib, "name") == 0) {
        return list->name;
    } else {
        dbg_str(OBJ_WARNNING, "llist list get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __add(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list add");

    return llist_add_back(l->llist, value);
}

static int __add_back(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list push back");

    return llist_add_back(l->llist, value);
}

static int __add_front(List *list, void *value)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list push back");

    return llist_add_front(l->llist, value);
}

static int __delete(List *list)
{
    Linked_List *l = (Linked_List *)list;
    dbg_str(OBJ_DETAIL, "Link list delete");

    return llist_delete(l->llist, &l->llist->begin);
}

static int __remove(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list remove");

    return llist_remove_front(l->llist, data);
}

static int __remove_front(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list remove front");

    return llist_remove_front(l->llist, data);
}

static int __remove_back(List *list, void **data)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "Link list remove");

    return llist_remove_back(l->llist, data);
}

static int __remove_element(List *list, void *data)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL, "List remove element");

    return llist_remove_element(l->llist, data);
}

static int __count(List *list)
{
    Linked_List *l = (Linked_List *)list;

    return llist_get_count(l->llist);
}

static int __detach_front(List *list, Iterator *iter)
{
    Linked_List *l    = (Linked_List *)list;
    LList_Iterator *i = (LList_Iterator *)iter;
    void *ret;

    i->list_pos = l->llist->begin;

    ret = llist_detach(l->llist, &(i->list_pos));
    if (ret == NULL) {
        i->list_pos.list_head_p = NULL;
    }

    return 0;
}

static int __free_detached(List *list, Iterator *iter)
{
    Linked_List *l    = (Linked_List *)list;
    LList_Iterator *i = (LList_Iterator *)iter;
    void *p;

    p = container_of(i->list_pos.list_head_p, list_t, list_head);
    allocator_mem_free(l->llist->allocator, p);

    return 0;
}

static Iterator *__begin(List *list)
{
    Linked_List *l         = (Linked_List *)list;
    allocator_t *allocator = list->obj.allocator;
    LList_Iterator *iter   = (LList_Iterator *)list->b;

    dbg_str(OBJ_DETAIL, "Linked List begin");

    llist_begin(l->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static Iterator *__end(List *list)
{
    Linked_List *l         = (Linked_List *)list;
    allocator_t *allocator = list->obj.allocator;
    LList_Iterator *iter   = (LList_Iterator *)list->e;

    dbg_str(OBJ_DETAIL, "Linked List end");

    llist_end(l->llist, &(iter->list_pos));

    return (Iterator *)iter;
}

static class_info_entry_t llist_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "List", "list", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "add", __add, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "add_front", __add_front, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "add_back", __add_back, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_FUNC_POINTER, "", "remove", __remove, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_FUNC_POINTER, "", "remove_front", __remove_front, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_FUNC_POINTER, "", "remove_back", __remove_back, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_FUNC_POINTER, "", "remove_element", __remove_element, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_FUNC_POINTER, "", "count", __count, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_FUNC_POINTER, "", "delete", __delete, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_FUNC_POINTER, "", "detach_front", __detach_front, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_FUNC_POINTER, "", "free_detached", __free_detached, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_FUNC_POINTER, "", "begin", __begin, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_FUNC_POINTER, "", "end", __end, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Linked_List", llist_class_info);

static void llist_list_print(void *element)
{
    dbg_str(OBJ_DETAIL, "value: %s", element);
}

int test_Linked_List(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    char buf[2048];
    List *list;
    char *str;
    char *str1 = "hello world1";
    char *str2 = "hello world2";
    char *str3 = "hello world3";
    char *str4 = "hello world4";
    char *str5 = "hello world5";

    dbg_str(DBG_DETAIL, "test_obj_llist_list");

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config_num(c, "/List", "value_size", 8) ;  

    list = OBJECT_NEW(allocator, Linked_List, c->buf);

    object_dump(list, "Linked_List", buf, 2048);
    dbg_str(DBG_DETAIL, "List dump: %s", buf);

    list->add_back(list, str3);
    list->add_back(list, str4);
    list->add_back(list, str5);
    list->add_front(list, str2);
    list->add_front(list, str1);

    dbg_str(DBG_DETAIL, "list for each test");
    dbg_str(DBG_DETAIL, "list count=%d", list->count(list));
    list->for_each(list, llist_list_print);

#if 0
    list->remove_back(list, (void **)&str);
    dbg_str(DBG_DETAIL, "remove back:%s", str);
    list->remove_front(list, (void **)&str);
    dbg_str(DBG_DETAIL, "remove front:%s", str);
#else
    dbg_str(DBG_DETAIL, "remove element:%s", str2);
    list->remove_element(list, (void *)str2);
#endif

    list->remove_all(list);

    dbg_str(DBG_DETAIL, "list for each test");
    dbg_str(DBG_DETAIL, "list count=%d", list->count(list));
    list->for_each(list, llist_list_print);

    object_destroy(list);
    cfg_destroy(c);

    return 1;
}
REGISTER_TEST_FUNC(test_Linked_List);
