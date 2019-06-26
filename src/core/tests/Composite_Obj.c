/**
 * @file Obj.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/core/tests/Composite_Obj.h>

static int __construct(Obj *obj, char *init_str)
{
    return 0;
}

static int __deconstruct(Obj *obj)
{
    Composite_Obj *test_obj = (Composite_Obj *)obj;

    if (test_obj->option != NULL)
        object_destroy(test_obj->option);

    return 0;
}

static class_info_entry_t test_obj_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Composite_Obj, construct, __construct),
    Init_Nfunc_Entry(2, Composite_Obj, deconstruct, __deconstruct),
    Init_Str___Entry(3, Composite_Obj, option, NULL),
    Init_U32___Entry(4, Composite_Obj, help, 0),
    Init_End___Entry(5, Composite_Obj),
};
REGISTER_CLASS("Composite_Obj", test_obj_class_info);
