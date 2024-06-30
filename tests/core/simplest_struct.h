#ifndef __SIMPLEST_STR_H__
#define __SIMPLEST_STR_H__

#include <stdio.h>
#include <libobject/core/String.h>
#include <libobject/core/Struct_Adapter.h>

typedef struct simplest_struct_s simplest_struct;
typedef Struct_Adapter Simplest_Struct_Adapter;

struct simplest_struct_s {
    int type;
    int len;
    char *name;
};

int simplest_struct_custom_to_json(cjson_t *root, void *element);
int simplest_struct_cumtom_new(allocator_t *allocator, cjson_t *c, void **value);
int simplest_struct_cumtom_free(allocator_t *allocator, simplest_struct *value);


#endif