#ifndef __OBJ_RBTREE_MAP_H__
#define __OBJ_RBTREE_MAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/rbtree_map.h>
#include <libobject/core/Map.h>
#include <libobject/core/Rbtree_Iterator.h>

typedef struct RBTree_Map_s RBTree_Map;

struct RBTree_Map_s{
    Map map;

    int (*construct)(Map *map,char *init_str);
    int (*deconstruct)(Map *map);
    int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_cmp_func)(Map *map, void *func);
    int (*add)(Map *map,void *key,void *value);
    int (*contain_key)(Map *map,void *key);
    int (*contain_value)(Map *map,void *value);
    int (*contain_key_and_value)(Map *map,void *key, void *value);
    int (*search)(Map *map,void *key,void **element);
    int (*search_all_same_key)(Map *map,void *key, List *list);
    int (*remove)(Map *map,void *key,void **element);
    int (*del)(Map *map, void *key);
    void (*for_each)(Map *map,void (*func)(void *key, void *element));
    void (*for_each_arg)(Map *map,void (*func)(void *key, void *element, void *arg),void *arg);
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);
    int (*destroy)(Map *map);
    int (*count)(Map *map);
    int (*reset)(Map *map);

    /*attribs*/
    rbtree_map_t *rbmap;
    void *key_cmp_cb;
};

#endif
