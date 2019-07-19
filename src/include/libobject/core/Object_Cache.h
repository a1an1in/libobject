#ifndef __OBJECT_CACHE_H__
#define __OBJECT_CACHE_H__

#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/List.h>

typedef struct Object_Cache_s Object_Cache;

struct Object_Cache_s{
	Obj obj;

	int (*construct)(Object_Cache *,char *init_str);
	int (*deconstruct)(Object_Cache *);

	/*virtual methods reimplement*/
	int (*set)(Object_Cache *, char *attrib, void *value);
    void *(*get)(Object_Cache *, char *attrib);
    void *(*new)(Object_Cache *cache, char *class_name);

    Map *class_map;
    List *object_list;
};


#endif
