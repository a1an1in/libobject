#ifndef __LIST_LINKED_H__
#define __LIST_LINKED_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/link_list.h>
#include <libobject/core/List.h>
#include <libobject/core/Linkedlist_Iterator.h>

typedef struct Linked_List_s Linked_List;

struct Linked_List_s{
    List list;

    int (*construct)(List *list,char *init_str);
    int (*deconstruct)(List *list);
    int (*set)(List *list, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*add)(List *list, void *value);
    int (*add_front)(List *list,void *value);
    int (*add_back)(List *list,void *value);
    int (*delete)(List *list,Iterator *iter);
    int (*remove)(List *list, void **data);
    int (*remove_front)(List *list, void **data);
    int (*remove_back)(List *list, void **data);
    int (*remove_element)(List *list, void *data);
    int (*count)(List *list);
    void* (*detach_front)(List *list, void **data);
    void (*for_each)(List *list,void (*func)(void *element));
    Iterator *(*begin)(List *list);
    Iterator *(*end)(List *list);
    int (*destroy)(List *list);


#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    llist_t *llist;
};

#endif
