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
#include <libdbg/debug.h>
#include <libobject/list_linked.h>

static int __construct(List *list,char *init_str)
{
    llist_t *llist;
    allocator_t *allocator = ((Obj *)list)->allocator;
    int value_size;
    int lock_type = 0;

    dbg_str(OBJ_DETAIL,"llist list construct,list addr:%p",list);

    if (list->value_size == 0) {
        value_size = 50;
        dbg_str(DBG_WARNNING,"link list value is zero, we'll set it to default value 50");
    } else {
        value_size = list->value_size;
    }
    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_set(llist,"data_size",&value_size);
    llist_init(llist);

    ((Linked_List *)list)->llist = llist;

    return 0;
}

static int __deconstrcut(List *list)
{
    dbg_str(OBJ_DETAIL,"llist list deconstruct,list addr:%p",list);

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
    else if (strcmp(attrib, "push_back") == 0) {
        list->push_back = value;
    } else if (strcmp(attrib, "insert_after") == 0) {
        list->insert_after = value;
    } else if (strcmp(attrib, "del") == 0) {
        list->del = value;
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
        strncpy(list->name,value,strlen(value));
    } else {
        dbg_str(OBJ_WARNNING,"list set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(List *obj, char *attrib)
{

    Linked_List *list = (Linked_List *)obj;

    if (strcmp(attrib, "name") == 0) {
        return list->name;
    } else {
        dbg_str(OBJ_WARNNING,"llist list get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __push_back(List *list,void *value)
{
    Linked_List *l = (Linked_List *)list;

    dbg_str(OBJ_DETAIL,"Link list push back");

    return llist_push_back(l->llist,value);
}

static int __insert_after(List *list,Iterator *iter, void *value)
{
    Linked_List *l = (Linked_List *)list;
    LList_Iterator *i = (LList_Iterator *)iter;

    dbg_str(OBJ_DETAIL,"List insert");

    return llist_insert(l->llist, &(i->list_pos), value);
}

static int __del(List *list,Iterator *iter)
{
    Linked_List *l = (Linked_List *)list;
    LList_Iterator *i = (LList_Iterator *)iter;

    dbg_str(OBJ_DETAIL,"Link list del");

    return llist_delete(l->llist, &(i->list_pos));
}

static Iterator *__begin(List *list)
{
    Linked_List *l = (Linked_List *)list;
    LList_Iterator *iter;
    allocator_t *allocator = list->obj.allocator;

    dbg_str(OBJ_DETAIL,"Linked List begin");

    iter = OBJECT_NEW(allocator, LList_Iterator,NULL);

    llist_begin(l->llist,&(iter->list_pos));

    return (Iterator *)iter;
}

static Iterator *__end(List *list)
{
    Linked_List *l = (Linked_List *)list;
    LList_Iterator *iter;
    allocator_t *allocator = list->obj.allocator;

    dbg_str(OBJ_DETAIL,"Linked List end");
    iter = OBJECT_NEW(allocator, LList_Iterator,NULL);

    llist_end(l->llist,&(iter->list_pos));

    return (Iterator *)iter;
}

static class_info_entry_t llist_list_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"List","list",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","push_back",__push_back,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","insert_after",__insert_after,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","begin",__begin,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","end",__end,sizeof(void *)},
    [10] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Linked_List",llist_list_class_info);

static void llist_list_print(Iterator *iter)
{
    LList_Iterator *i = (LList_Iterator *)iter;
    dbg_str(DBG_DETAIL,"value: %s",i->get_vpointer(iter));
}

void test_obj_llist_list()
{
    Iterator *iter, *next,*prev;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *ll, *l;
    char buf[2048];

    dbg_str(DBG_DETAIL,"test_obj_llist_list");

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Linked_List", ll = cjson_create_object());{
            cjson_add_item_to_object(ll, "List", l = cjson_create_object());{
                cjson_add_number_to_object(l, "value_size", 25);
            }
        }
    }
    set_str = cjson_print(root);
    dbg_str(DBG_DETAIL,"set str:%s",set_str);

    List *list;
    list  = OBJECT_NEW(allocator, Linked_List,set_str);
    iter = OBJECT_NEW(allocator, LList_Iterator,NULL);

    object_dump(list, "Linked_List", buf, 2048);
    dbg_str(DBG_DETAIL,"List dump: %s",buf);

    list->push_back(list,"hello world");
    list->push_back(list,"sdfsafsdaf");
    dbg_str(DBG_DETAIL,"list for each test");
    list->for_each(list,llist_list_print);

    object_destroy(list);
    object_destroy(iter);
    free(set_str);
}


