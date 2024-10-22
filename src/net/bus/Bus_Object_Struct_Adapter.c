#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/core/Obj.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/utils/dbg/debug.h>

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

static class_info_entry_t busd_object_stuct_adapter[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, Struct_Adapter, new, NULL),
    Init_Point_Entry(2, Struct_Adapter, free, NULL),
    Init_Point_Entry(3, Struct_Adapter, to_json, busd_object_struct_custom_to_json),
    Init_End___Entry(4, Struct_Adapter),
};
REGISTER_CLASS(Busd_Object_Struct_Adapter, busd_object_stuct_adapter)