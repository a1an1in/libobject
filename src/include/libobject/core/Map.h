#ifndef __OBJ_MAP_H__
#define __OBJ_MAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Iterator.h>
#include <libobject/core/List.h>

typedef struct _map_s Map;

/**
 * map key type can be value and address, and the default compare function 
 * only compare the value, if you key is address, you must set your function.
 * we support string_key_cmp_func().
 */
struct _map_s{
	Obj obj;

	int (*construct)(Map *map,char *init_str);
	int (*deconstruct)(Map *map);

	/*virtual methods reimplement*/
	int (*reconstruct)(Map *map);
	int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*set_cmp_func)(Map *map, void *func);
    int (*add)(Map *map,void *key,void *value);
    int (*search)(Map *map,void *key,void **element);
    int (*contain_key)(Map *map,void *key);
    int (*contain_value)(Map *map,void *value);
    int (*contain_key_and_value)(Map *map,void *key, void *value);
    int (*search_all_same_key)(Map *map,void *key, List *list);
    int (*remove)(Map *map,void *key,void **element);
    int (*del)(Map *map, void *key);
    void (*for_each)(Map *map,void (*func)(void *key, void *element));
    void (*for_each_arg)(Map *map,void (*func)(void *key, void *element, void *arg),void *arg);
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);
    int (*destroy)(Map *map);

    /*inherit methods*/
    int (*set_target_name)(Obj *obj, char *);

    Iterator *b, *e;
};

#endif
