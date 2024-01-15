/**
 * @file Tbz.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include <unistd.h>
#include "Tbz.h"

static int __compress(Tbz *tar, char *file_in, char *file_out)
{

}

static int __uncompress(Tbz *tar, char *file_in, char *file_out)
{

}

static class_info_entry_t tgz_class_info[] = {
    Init_Obj___Entry(0, Tar, parent),
    Init_Vfunc_Entry(1, Tbz, compress, __compress),
    Init_Vfunc_Entry(2, Tbz, uncompress, __uncompress),
    Init_End___Entry(3, Tbz),
};
REGISTER_CLASS("Tbz", tgz_class_info);