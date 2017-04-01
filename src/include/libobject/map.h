#ifndef __OBJ_MAP_H__
#define __OBJ_MAP_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/obj.h>
#include <libobject/iterator.h>

typedef struct _map_s Map;

struct _map_s{
	Obj obj;

	int (*construct)(Map *map,char *init_str);
	int (*deconstruct)(Map *map);
	int (*set)(Map *map, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*insert)(Map *map,void *key,void *value);
    int (*insert_wb)(Map *map,void *key,void *value,Iterator *iter);
    int (*search)(Map *map,void *key,Iterator *iter);
    int (*del)(Map *map,Iterator *iter);
    /*
     *void (*for_each)(Map *map,void (*func)(Map *map, char *key, void *value));
     */
    void (*for_each)(Map *map,void (*func)(Iterator *iter));
    void (*for_each_arg2)(Map *map,void (*func)(Iterator *iter, void *arg),void *arg);
    Iterator *(*begin)(Map *map);
    Iterator *(*end)(Map *map);
    int (*destroy)(Map *map);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    Iterator *b, *e;
};

#endif
