#ifndef __ITERATOR_HMAP_H__
#define __ITERATOR_HMAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/hash_list.h>
#include <libobject/core/Iterator.h>

typedef struct hmap_iterator_s Hmap_Iterator;

struct hmap_iterator_s{
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
    hash_map_pos_t hash_map_pos;

};

#endif
