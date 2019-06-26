/**
 * @file Obj.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/core/tests/Composite_Obj.h>

static int __construct(Composite_Obj *obj, char *init_str)
{
    if (obj->vector == NULL) {
        obj->vector = object_new(obj->parent.allocator, "Vector", NULL);
    }
    return 0;
}

static int __deconstruct(Composite_Obj *obj)
{

    if (obj->name != NULL)
        object_destroy(obj->name);
    if (obj->vector != NULL) {
        object_destroy(obj->vector);
    }

    return 0;
}

static class_info_entry_t composite_obj_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Composite_Obj, construct, __construct),
    Init_Nfunc_Entry(2, Composite_Obj, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Composite_Obj, set, NULL),
    Init_Vfunc_Entry(4, Composite_Obj, get, NULL),
    Init_Vfunc_Entry(5, Composite_Obj, to_json, NULL),
    Init_Str___Entry(6, Composite_Obj, name, NULL),
    Init_U32___Entry(7, Composite_Obj, help, 0),
    Init_Vec___Entry(8, Composite_Obj, vector, NULL, "Simplest_Obj"),
    Init_End___Entry(9, Composite_Obj),
};
REGISTER_CLASS("Composite_Obj", composite_obj_class_info);
