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
    void *(*get)(Obj *obj, char *attrib);
    char *(*to_json)(Obj *obj); 
    int (*assign)(Obj *obj, char *value); 
    char *(*reset)(Obj *obj); 
    int (*set_target_name)(Obj *obj, char *);
    int (*override_virtual_funcs)(Obj *obj, char *func_name, void *value);

#define MAX_CLASS_NAME_LEN 20
    char name[MAX_CLASS_NAME_LEN];
    char target_name[MAX_CLASS_NAME_LEN];
#undef MAX_CLASS_NAME_LEN
    void *cache;
    void *json;
};

#endif
