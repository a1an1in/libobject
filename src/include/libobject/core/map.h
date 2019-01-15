#ifndef __OBJ_MAP_H__
#define __OBJ_MAP_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/iterator.h>
#include <libobject/core/list.h>

typedef struct _map_s Map;

struct _map_s{
	Obj obj;

	int (*construct)(Map *map,char *init_str);
	int (*deconstruct)(Map *map);
	int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

    int (*set_cmp_func)(Map *map, void *func);
    int (*add)(Map *map,void *key,void *value);
    int (*search)(Map *map,void *key,void **element);
    int (*search_all_same_key)(Map *map,void *key, List *list);
    int (*remove)(Map *map,void *key,void **element);
    int (*del)(Map *map, void *key);
    void (*for_each)(Map *map,void (*func)(void *key, void *element));
    void (*for_each_arg)(Map *map,void (*func)(void *key, void *element, void *arg),void *arg);
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);
    int (*destroy)(Map *map);
    int (*size)(Map*);
    int (*is_empty)(Map *);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    Iterator *b, *e;
};

#endif
