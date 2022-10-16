/**
 * @file Stub.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-08-29
 */

#include "libobject/stub/Stub.h"

static int __construct(Stub *stub, char *init_str)
{
    return 0;
}

static int __deconstruct(Stub *stub)
{
    return 0;
}

static class_info_entry_t stub_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stub, construct, __construct),
    Init_Nfunc_Entry(2, Stub, deconstruct, __deconstruct),
    Init_End___Entry(3, Stub),
};
REGISTER_CLASS("Stub", stub_class_info);

