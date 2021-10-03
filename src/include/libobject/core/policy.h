#ifndef __POLICY_H__
#define __POLICY_H__

#include <stdio.h>
#include <libobject/basic_types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/core/String.h>
#include <libobject/core/try.h>
#include <libobject/argument/Command.h>

typedef struct vector_to_json_policy_s {
    int (*policy)(cjson_t *root, void *element);
} vector_to_json_policy_t;

extern vector_to_json_policy_t g_vector_to_json_policy[ENTRY_TYPE_MAX_TYPE];
#endif
