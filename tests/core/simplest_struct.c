#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/try.h>
#include <libobject/core/Vector.h>
#include "simplest_struct.h"

int simplest_struct_custom_to_json(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    simplest_struct *o = (simplest_struct *)element;

    item = cjson_create_object();
    cjson_add_number_to_object(item, "type", o->type);
    cjson_add_number_to_object(item, "len", o->len);
    cjson_add_string_to_object(item, "name", o->name);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

int simplest_struct_cumtom_new(allocator_t *allocator, cjson_t *c, void **value)
{
    simplest_struct *v;
    int ret;

    TRY {
        v = allocator_mem_alloc(allocator, sizeof(simplest_struct));
        *value = v;
        while (c) {
            if (strcmp(c->string, "type") == 0) {
                v->type = c->valueint;
            } else if (strcmp(c->string, "len") == 0) {
                v->len = c->valueint;
            } else if (strcmp(c->string, "name") == 0) {
                v->name = allocator_mem_alloc(allocator, strlen(c->string) + 1);
                strcpy(v->name, c->valuestring);
            } else {
                dbg_str(DBG_VIP, "wrong value name:%s", c->string);
            }

            c = c->next;
        }
    } CATCH (ret) {}

    return ret;
}

int simplest_struct_cumtom_free(allocator_t *allocator, simplest_struct *value)
{
    allocator_mem_free(allocator, value->name);
    allocator_mem_free(allocator, value);

    return 1;
}

static class_info_entry_t simplest_sturct_adapter[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, Simplest_Struct_Adapter, new, simplest_struct_cumtom_new),
    Init_Point_Entry(2, Simplest_Struct_Adapter, free, simplest_struct_cumtom_free),
    Init_Point_Entry(3, Simplest_Struct_Adapter, to_json, simplest_struct_custom_to_json),
    Init_End___Entry(4, Simplest_Struct_Adapter),
};
REGISTER_CLASS(Simplest_Struct_Adapter, simplest_sturct_adapter);
