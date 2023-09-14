#ifndef __INTERVAL_TREE_H__
#define __INTERVAL_TREE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/hash_list.h>
#include <libobject/core/Interval_Tree.h>
#include <libobject/core/Map.h>

typedef struct Interval_Tree_s Interval_Tree;

typedef struct interval_tree_node_s {
    long start;
    long end;
    void *value;
} interval_tree_node_t;

struct Interval_Tree_s {
    Obj obj;

    int (*construct)(Interval_Tree *tree, char *init_str);
    int (*deconstruct)(Interval_Tree *tree);

    /*virtual methods reimplement*/
    int (*set)(Interval_Tree *tree, char *attrib, void *value);
    void *(*get)(Interval_Tree *tree, char *attrib);
    int (*add)(Interval_Tree *tree, void *key, void *value);
    int (*search)(Interval_Tree *tree, void *key, void **element);
    int (*remove)(Interval_Tree *tree, void *key, void **element);
    int (*set_trustee)(Interval_Tree *tree, int value_type, int (*value_free_callback)(allocator_t *allocator, void *value));

    /*attribs*/
    Map *map;
};

#endif
