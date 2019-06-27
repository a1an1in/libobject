/**
 * @file Argument.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
#include <libobject/argument/Argument.h>

static int __construct(Argument *arg, char *init_str)
{
    return 0;
}

static int __deconstruct(Argument *arg)
{
    if (arg->usage != NULL) {
        object_destroy(arg->usage);
    }

    if (arg->value != NULL) {
        object_destroy(arg->value);
    }

    return 0;
}

static class_info_entry_t arg_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Argument, construct, __construct),
    Init_Nfunc_Entry(2 , Argument, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Argument, get, NULL),
    Init_Vfunc_Entry(4 , Argument, set, NULL),
    Init_Vfunc_Entry(5 , Argument, to_json, NULL),
    Init_Str___Entry(6 , Argument, usage, NULL),
    Init_Str___Entry(7 , Argument, value, NULL),
    Init_End___Entry(8 , Argument),
};
REGISTER_CLASS("Argument", arg_class_info);
