#ifndef __OBJ_H__
#define __OBJ_H__

#include <libobject/core/class_deamon.h>
#include <libobject/core/object.h>
#include <libobject/core/value_type.h>

typedef struct obj_s Obj;
struct obj_s{
	allocator_t *allocator;
	int (*construct)(Obj *obj,char *init_str);
	int (*deconstruct)(Obj *obj);
	int (*set)(Obj *obj, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 
    int (*set_target_name)(Obj *obj, char *);

#define MAX_CLASS_NAME_LEN 20
    char name[MAX_CLASS_NAME_LEN];
    char target_name[MAX_CLASS_NAME_LEN];
#undef MAX_CLASS_NAME_LEN
    void *cache;
    void *json;
};

#endif
