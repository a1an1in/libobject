/**
 * @file Uio.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-03-26
 */

#include <libobject/drivers/uio/Uio.h>

static int __construct(Uio *module, char *init_str)
{
    return 0;
}

static int __deconstruct(Uio *module)
{
    return 0;
}

static class_info_entry_t module_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Uio, construct, __construct),
    Init_Nfunc_Entry(2, Uio, deconstruct, __deconstruct),
    Init_End___Entry(3, Uio),
};
REGISTER_CLASS("Uio", module_class_info);