#ifndef __OBJECT_DEAMON_H__
#define __OBJECT_DEAMON_H__

#include <libobject/core/Obj.h>
#include <libobject/core/Object_Cache.h>

typedef struct Object_Deamon_s Object_Deamon;

struct Object_Deamon_s{
	Obj obj;

	int (*construct)(Object_Deamon *,char *init_str);
	int (*deconstruct)(Object_Deamon *);

	/*virtual methods reimplement*/
	int (*set)(Object_Deamon *, char *attrib, void *value);
    void *(*get)(Object_Deamon *, char *attrib);
    Object_Cache *(*get_cache)(Object_Deamon *, char *cache_name);
};

#endif
