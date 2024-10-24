#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/core/Obj.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/utils/dbg/debug.h>

int busd_object_struct_custom_new(allocator_t *allocator, cjson_t *c, void **value)
{
    busd_object_t *v;
    int ret;

    TRY {
        v = allocator_mem_alloc(allocator, sizeof(busd_object_t));
        *value = v;
        while (c) {
            if (strcmp(c->string, "fd") == 0) {
                v->fd = c->valueint;
            } else if (strcmp(c->string, "id") == 0) {
                strcpy(v->id, c->valuestring);
            } else {
                dbg_str(DBG_VIP, "wrong value name:%s", c->string);
            }

            c = c->next;
        }
    } CATCH (ret) {}

    return ret;
}

int busd_object_struct_custom_free(allocator_t *allocator, busd_object_t *element)
{
    allocator_mem_free(allocator, element);

    return 1;
}

int busd_object_struct_custom_to_json(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    busd_object_t *o = (busd_object_t *)element;

    item = cjson_create_object();

    cjson_add_string_to_object(item, "id", o->id);
    // cjson_add_string_to_object(item, "infos", o->infos);
    cjson_add_number_to_object(item, "fd", o->fd);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

int busd_object_struct_custom_print(int index, busd_object_t *element)
{
    printf("index:%d, id:%s, fd:%d\n", index, element->id, element->fd);

    return 1;
}

static class_info_entry_t busd_object_stuct_adapter[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, Struct_Adapter, new, busd_object_struct_custom_new),
    Init_Point_Entry(2, Struct_Adapter, free, busd_object_struct_custom_free),
    Init_Point_Entry(3, Struct_Adapter, to_json, busd_object_struct_custom_to_json),
    Init_Point_Entry(4, Struct_Adapter, print, busd_object_struct_custom_print),
    Init_End___Entry(5, Struct_Adapter),
};
REGISTER_CLASS(Busd_Object_Struct_Adapter, busd_object_stuct_adapter)