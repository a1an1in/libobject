/**
 * @file Tbz2.c
 * @Synopsis 
 *  haven't tested.
 * @author alan lin
 * @version 
 * @date 2024-01-22
 */
#include <unistd.h>
#include "Tbz2.h"

static int __construct(Tbz2 *Tbz2, char *init_str)
{
    allocator_t *allocator = Tbz2->parent.parent.parent.allocator;

    Tbz2->c = object_new(allocator, "Bz2Compress", NULL);
    Tbz2->file_name = object_new(allocator, "String", NULL);
    return 0;
}

static int __deconstruct(Tbz2 *Tbz2)
{
    object_destroy(Tbz2->c);
    object_destroy(Tbz2->file_name);

    return 0;
}

static int __compress(Tbz2 *Tbz2, char *file_in, char **file_out)
{
    char *name, *fmt, *str;
    Archive *archive = &Tbz2->parent.parent;
    char in_tmp[1024] = {0};
    String *file_name;
    Compress *c;
    int ret, len;

    TRY {
        THROW_IF(file_out == NULL, -1);

        strcpy(in_tmp, file_in);
        EXEC(fs_get_path_and_name(in_tmp, NULL, &name));

        len = strlen(STR2A(archive->extracting_path));
        if (STR2A(archive->extracting_path)[len -1] == '/') {
            fmt = "%s%s.bz2";
        } else { fmt = "%s/%s.bz2"; }

        file_name = Tbz2->file_name;
        file_name->format(file_name, 1024, fmt, STR2A(archive->extracting_path), name);
        dbg_str(DBG_VIP, "tbz2 compress file:%s, file_out:%s", file_in, STR2A(file_name));
        *file_out = STR2A(file_name);

        c = Tbz2->c;
        EXEC(c->compress_file(c, file_in, *file_out));
    } CATCH (ret) {}
    
    return ret;
}

static int __uncompress(Tbz2 *Tbz2, char *file_in, char **file_out)
{
    char *name, *fmt, *str;
    Archive *archive = &Tbz2->parent.parent;
    char in_tmp[1024] = {0};
    String *file_name;
    Compress *c;
    int ret, len;

    TRY {
        THROW_IF(file_out == NULL, -1);

        strcpy(in_tmp, file_in);
        EXEC(fs_get_path_and_name(in_tmp, NULL, &name));
        str = strstr(name, ".gz2");
        THROW_IF(str == NULL, -1);
        memset(str, 0, strlen(str));

        len = strlen(STR2A(archive->extracting_path));
        if (STR2A(archive->extracting_path)[len -1] == '/') {
            fmt = "%s%s";
        } else { fmt = "%s/%s"; }

        file_name = Tbz2->file_name;
        file_name->format(file_name, 1024, fmt, STR2A(archive->extracting_path), name);
        dbg_str(DBG_VIP, "Tbz2 uncompress file:%s, file_out:%s", file_in, STR2A(file_name));
        *file_out = STR2A(file_name);

        c = Tbz2->c;
        EXEC(c->uncompress_file(c, file_in, *file_out));
    } CATCH (ret) {}
    
    return ret;
}

static class_info_entry_t tbz_class_info[] = {
    Init_Obj___Entry(0, Tar, parent),
    Init_Nfunc_Entry(1, Tbz2, construct, __construct),
    Init_Nfunc_Entry(2, Tbz2, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Tbz2, compress, __compress),
    Init_Vfunc_Entry(4, Tbz2, uncompress, __uncompress),
    Init_End___Entry(5, Tbz2),
};
REGISTER_CLASS(Tbz2, tbz_class_info);