/**
 * @file Tgz.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include <unistd.h>
#include "Tgz.h"

static int __compress(Tgz *tar, char *file_in, char *file_out)
{

}

static int __uncompress(Tgz *tar, char *file_in, char *file_out)
{

}

static class_info_entry_t tgz_class_info[] = {
    Init_Obj___Entry(0, Tar, parent),
    Init_Vfunc_Entry(1, Tgz, compress, __compress),
    Init_Vfunc_Entry(2, Tgz, uncompress, __uncompress),
    Init_End___Entry(3, Tgz),
};
REGISTER_CLASS("Tgz", tgz_class_info);