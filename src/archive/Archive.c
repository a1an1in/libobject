/**
 * @file Archive.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include <libobject/archive/Archive.h>

static int __construct(Archive *archive, char *init_str)
{
    return 0;
}

static int __deconstruct(Archive *archive)
{
    return 0;
}

static class_info_entry_t archive_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Archive, construct, __construct),
    Init_Nfunc_Entry(2, Archive, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Archive, compress, NULL),
    Init_Vfunc_Entry(4, Archive, uncompress, NULL),
    Init_End___Entry(5, Archive),
};
REGISTER_CLASS("Archive", archive_class_info);

