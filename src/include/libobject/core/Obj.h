#ifndef __OBJ_H__
#define __OBJ_H__

#include <libobject/core/class_deamon.h>
#include <libobject/core/object.h>
#include <libobject/core/value_type.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/json/cjson.h>

#define MAX_CLASS_NAME_LEN 25

typedef struct obj_s Obj;
struct obj_s {
    allocator_t *allocator;

    int (*construct)(Obj *obj,char *init_str);
    int (*deconstruct)(Obj *obj);

    int (*set)(Obj *obj, char *attrib, void *value);
    void *(*get)(Obj *obj, char *attrib);
    char *(*to_json)(Obj *obj); 
    int (*assign)(Obj *obj, char *value); 
    int (*reset)(Obj *obj); 
    int (*set_target_name)(Obj *obj, char *);

    char name[MAX_CLASS_NAME_LEN];
    char target_name[MAX_CLASS_NAME_LEN];
    void *cache;
    void *json;
};

typedef struct obj_set_policy_s {
    int (*policy)(Obj *obj, class_info_entry_t *entry, void *value);
} obj_set_policy_t;

typedef struct obj_to_json_policy_s {
    int (*policy)(cjson_t *json, char *name, void *value);
} obj_to_json_policy_t;

extern obj_to_json_policy_t g_obj_to_json_policy[ENTRY_TYPE_MAX_TYPE];
extern obj_set_policy_t g_obj_set_policy[ENTRY_TYPE_MAX_TYPE];

#endif
