/**
 * @file Tar.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Tar.h"

static int __construct(Tar *tar, char *init_str)
{
    return 0;
}

static int __deconstruct(Tar *tar)
{
    return 0;
}

static class_info_entry_t tar_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Tar, construct, __construct),
    Init_Nfunc_Entry(2, Tar, deconstruct, __deconstruct),
    Init_End___Entry(3, Tar),
};
REGISTER_CLASS("Tar", tar_class_info);

