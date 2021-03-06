#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/hash_list.h>
#include <libobject/core/Map.h>
#include <libobject/core/Hmap_Iterator.h>

typedef struct Hash_Map_s Hash_Map;

struct Hash_Map_s{
    Map map;

    int (*construct)(Map *map,char *init_str);
    int (*deconstruct)(Map *map);

    /*virtual methods reimplement*/
    int (*reconstruct)(Map *map);
    int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add)(Map *map,void *key,void *value);
    int (*search)(Map *map,void *key,void **element);
    int (*remove)(Map *map,void *key,void **element);
    int (*del)(Map *map, void *key);
    /*
     *void (*for_each)(Map *map,void (*func)(Map *map, char *key, void *value));
     */
    void (*for_each)(Map *map,void (*func)(void *key, void *element));
    void (*for_each_arg)(Map *map,void (*func)(void *key, void *element, void *arg),void *arg);
    int (*destroy)(Map *map);
    int (*count)(Map *map);
    int (*reset)(Map *map);
    int (*set_cmp_func)(Map *map, void *func);
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);

    /*attribs*/
    hash_map_t *hmap;
    uint16_t bucket_size;
};

#endif
