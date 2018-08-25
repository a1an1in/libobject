#ifndef __ITERATOR_LINKEDLIST_H__
#define __ITERATOR_LINKEDLIST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/link_list.h>
#include <libobject/core/iterator.h>

typedef struct llist_iterator_s LList_Iterator;

struct llist_iterator_s{
	Iterator iter;

	int (*construct)(Iterator *iter,char *init_str);
	int (*deconstruct)(Iterator *iter);
	int (*set)(Iterator *iter, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual funcs*/
    Iterator *(*next)(Iterator *it);
    Iterator *(*prev)(Iterator *it);
    int (*equal)(Iterator *it1,Iterator *it2);
    void *(*get_vpointer)(Iterator *it);
    void *(*get_kpointer)(Iterator *it);
    int (*is_null)(Iterator *it);
    int (*clear)(Iterator *it);

	/*virtual methods reimplement*/
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    list_pos_t list_pos;

};

#endif
