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
    return 0;
}

static int __deconstruct(Option *option)
{
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
    Init_Point_Entry(9 , Option, action, NULL),
    Init_U32___Entry(10, Option, value_type, 0),
    Init_U32___Entry(11, Option, int_value, 0),
    Init_Str___Entry(12, Option, string_value, NULL),
    Init_End___Entry(13, Option),
};
REGISTER_CLASS("Option", option_class_info);
