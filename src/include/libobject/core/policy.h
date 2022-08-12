#ifndef __POLICY_H__
#define __POLICY_H__

#include <stdio.h>
#include <libobject/basic_types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/core/String.h>
#include <libobject/core/Number.h>
#include <libobject/core/try.h>
#include <libobject/argument/Command.h>

typedef struct vector_to_json_policy_s {
    int (*policy)(cjson_t *root, void *element);
} vector_to_json_policy_t;

typedef struct obj_set_policy_s {
    int (*policy)(Obj *obj, class_info_entry_t *entry, void *value);
} obj_set_policy_t;

typedef struct obj_to_json_policy_s {
    int (*policy)(cjson_t *json, char *name, void *value);
} obj_to_json_policy_t;

typedef struct number_policy_s {
    int (*set_type)(Number *number, enum number_type_e type);
    int (*add)(Number *number, enum number_type_e type, void *value, int len);
    int (*sub)(Number *number, enum number_type_e type, void *value, int len);
    int (*mul)(Number *number, enum number_type_e a1_type, void *a1_value, int a1_len, enum number_type_e a2_type, void *a2_value, int a2_len);
} number_policy_t;

extern obj_to_json_policy_t g_obj_to_json_policy[ENTRY_TYPE_MAX_TYPE];
extern obj_set_policy_t g_obj_set_policy[ENTRY_TYPE_MAX_TYPE];
extern vector_to_json_policy_t g_vector_to_json_policy[ENTRY_TYPE_MAX_TYPE];
extern number_policy_t g_number_policies[NUMBER_TYPE_MAX];

#endif
