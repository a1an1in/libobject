/**
 * @file Gzip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Gzip.h"

static int __construct(Gzip *gzip, char *init_str)
{
    return 0;
}

static int __deconstruct(Gzip *gzip)
{
    return 0;
}

static class_info_entry_t gzip_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Gzip, construct, __construct),
    Init_Nfunc_Entry(2, Gzip, deconstruct, __deconstruct),
    Init_End___Entry(3, Gzip),
};
REGISTER_CLASS("Gzip", gzip_class_info);

