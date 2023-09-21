/**
 * @file Zip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Zip.h"

static int __construct(Zip *zip, char *init_str)
{
    return 0;
}

static int __deconstruct(Zip *zip)
{
    return 0;
}

static class_info_entry_t zip_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Zip, construct, __construct),
    Init_Nfunc_Entry(2, Zip, deconstruct, __deconstruct),
    Init_End___Entry(3, Zip),
};
REGISTER_CLASS("Zip", zip_class_info);

