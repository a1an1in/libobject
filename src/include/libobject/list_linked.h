#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/list.h>
#include <libdata_structure/link_list.h>
#include <libobject/iterator_linkedlist.h>

typedef struct Linked_List_s Linked_List;

struct Linked_List_s{
	List list;

	int (*construct)(List *list,char *init_str);
	int (*deconstruct)(List *list);
	int (*set)(List *list, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*push_back)(List *list,void *value);
    int (*insert_after)(List *list,Iterator *iter, void *value);
    int (*del)(List *list,Iterator *iter);
    void (*for_each)(List *list,void (*func)(Iterator *iter));
    Iterator *(*begin)(List *list);
    Iterator *(*end)(List *list);
    int (*destroy)(List *list);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    llist_t *llist;
};

#endif
