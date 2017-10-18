#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/map.h>
#include <libobject/utils/data_structure/hash_list.h>
#include <libobject/core/hmap_iterator.h>

typedef struct Hash_Map_s Hash_Map;

struct Hash_Map_s{
	Map map;

	int (*construct)(Map *map,char *init_str);
	int (*deconstruct)(Map *map);
	int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Map *map,void *key,void *value);
    int (*search)(Map *map,void *key,void **element);
    int (*remove)(Map *map,void *key,void **element);
    int (*del)(Map *map, void *key);
    /*
     *void (*for_each)(Map *map,void (*func)(Map *map, char *key, void *value));
     */
    void (*for_each)(Map *map,void (*func)(void *key, void *element));
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);
    int (*destroy)(Map *map);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    hash_map_t *hmap;
    uint16_t key_size;
    uint16_t bucket_size;
    uint16_t value_size;
    uint8_t key_type;
};

#endif
