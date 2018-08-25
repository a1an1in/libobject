#ifndef __ITERATOR_RBTREE_MAP_H__
#define __ITERATOR_RBTREE_MAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/hash_list.h>
#include <libobject/core/iterator.h>

typedef struct rbtree_iterator_s RBTree_Iterator;

struct rbtree_iterator_s{
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

	/*virtual methods reimplement*/
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    rbtree_map_pos_t rbtree_map_pos;

};

#endif
