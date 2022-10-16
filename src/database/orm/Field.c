/**
 * @file Field.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/database/orm/Field.h>

static int __construct(Field *field, char *init_str)
{
    return 0;
}

static int __deconstruct(Field *field)
{
    if (field->name)
        object_destroy(field->name);

    if (field->type)
        object_destroy(field->type);

    if (field->constraint)
        object_destroy(field->constraint);

    return 0;
}

static class_info_entry_t field_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Field, construct, __construct),
    Init_Nfunc_Entry(2, Field, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Field, set, NULL),
    Init_Vfunc_Entry(4, Field, get, NULL),
    Init_Str___Entry(5, Field, name, NULL),
    Init_Str___Entry(6, Field, type, NULL),
    Init_Str___Entry(7, Field, constraint, NULL),
    Init_End___Entry(8, Field),
};
REGISTER_CLASS("Field", field_class_info);

