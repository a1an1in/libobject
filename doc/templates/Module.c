/**
 * @file Module.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Module.h"

static int __construct(Module *module, char *init_str)
{
    return 0;
}

static int __deconstruct(Module *module)
{
    return 0;
}

static class_info_entry_t module_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Module, construct, __construct),
    Init_Nfunc_Entry(2, Module, deconstruct, __deconstruct),
    Init_End___Entry(3, Module),
};
REGISTER_CLASS(Module, module_class_info);