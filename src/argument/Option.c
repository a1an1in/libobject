/**
 * @file Option.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
#include <libobject/argument/Option.h>

static int __construct(Option *option, char *init_str)
{
    option->set_flag = 0;
    option->multi_value_flag = 0;

    return 0;
}

static int __deconstruct(Option *option)
{
    if (option->name != NULL) {
        object_destroy(option->name);
    }

    if (option->alias != NULL) {
        object_destroy(option->alias);
    }

    if (option->usage != NULL) {
        object_destroy(option->usage);
    }

    if (option->value != NULL) {
        object_destroy(option->value);
    }

    return 0;
}

static class_info_entry_t option_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Option, construct, __construct),
    Init_Nfunc_Entry(2 , Option, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Option, get, NULL),
    Init_Vfunc_Entry(4 , Option, set, NULL),
    Init_Vfunc_Entry(5 , Option, to_json, NULL),
    Init_Str___Entry(6 , Option, name, NULL),
    Init_Str___Entry(7 , Option, alias, NULL),
    Init_Str___Entry(8 , Option, usage, NULL),
    Init_Str___Entry(9 , Option, value, NULL),
    Init_Point_Entry(10, Option, action, NULL),
    Init_Point_Entry(11, Option, opaque, NULL),
    Init_Point_Entry(12, Option, command, NULL),
    Init_U8____Entry(13, Option, set_flag, NULL),
    Init_U8____Entry(14, Option, multi_value_flag, NULL),
    Init_End___Entry(15, Option),
};
REGISTER_CLASS(Option, option_class_info);
