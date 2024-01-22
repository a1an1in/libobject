/**
 * @file Tbz.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include <unistd.h>
#include "Tbz.h"

static int __compress(Tbz *tar, char *file_in, char **file_out)
{
    dbg_str(DBG_VIP, "tbz compress file:%s", file_in);
}

static int __uncompress(Tbz *tar, char *file_in, char **file_out)
{
    char *name;
    int ret;

    TRY {
        // name->assign(name, STR2A(archive->extracting_path));
        EXEC(fs_get_path_and_name(file_in, NULL, &name))
        dbg_str(DBG_VIP, "tbz uncompress file:%s, name:%s", file_in, name);
    } CATCH (ret) {}
    
    return ret;
}

static class_info_entry_t tgz_class_info[] = {
    Init_Obj___Entry(0, Tar, parent),
    Init_Vfunc_Entry(1, Tbz, compress, __compress),
    Init_Vfunc_Entry(2, Tbz, uncompress, __uncompress),
    Init_End___Entry(3, Tbz),
};
REGISTER_CLASS("Tbz", tgz_class_info);