/**
 * @file Tgz.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-01-22
 */
#include <unistd.h>
#include "Tgz.h"

static int __construct(Tgz *tgz, char *init_str)
{
    allocator_t *allocator = tgz->parent.parent.parent.allocator;

    tgz->c = object_new(allocator, "GZCompress", NULL);
    tgz->file_name = object_new(allocator, "String", NULL);
    return 0;
}

static int __deconstruct(Tgz *tgz)
{
    object_destroy(tgz->c);
    object_destroy(tgz->file_name);

    return 0;
}

static int __compress(Tgz *tgz, char *file_in, char **file_out)
{
    char *name, *fmt, *str;
    Archive *archive = &tgz->parent.parent;
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
            fmt = "%s%s.gz";
        } else { fmt = "%s/%s.gz"; }

        file_name = tgz->file_name;
        file_name->format(file_name, 1024, fmt, STR2A(archive->extracting_path), name);
        dbg_str(DBG_INFO, "tgz compress file:%s, file_out:%s", file_in, STR2A(file_name));
        *file_out = STR2A(file_name);

        c = tgz->c;
        EXEC(c->compress_file(c, file_in, *file_out));
    } CATCH (ret) {}
    
    return ret;
}

static int __uncompress(Tgz *tgz, char *file_in, char **file_out)
{
    char *name, *fmt, *str;
    Archive *archive = &tgz->parent.parent;
    char in_tmp[1024] = {0};
    String *file_name;
    Compress *c;
    int ret, len;

    TRY {
        THROW_IF(file_out == NULL, -1);

        strcpy(in_tmp, file_in);
        EXEC(fs_get_path_and_name(in_tmp, NULL, &name));
        str = strstr(name, ".gz");
        THROW_IF(str == NULL, -1);
        memset(str, 0, strlen(str));

        len = strlen(STR2A(archive->extracting_path));
        if (STR2A(archive->extracting_path)[len -1] == '/') {
            fmt = "%s%s";
        } else { fmt = "%s/%s"; }

        file_name = tgz->file_name;
        file_name->format(file_name, 1024, fmt, STR2A(archive->extracting_path), name);
        dbg_str(DBG_INFO, "tgz uncompress file:%s, file_out:%s", file_in, STR2A(file_name));
        *file_out = STR2A(file_name);

        c = tgz->c;
        EXEC(c->uncompress_file(c, file_in, *file_out));
    } CATCH (ret) {}
    
    return ret;
}

static class_info_entry_t tgz_class_info[] = {
    Init_Obj___Entry(0, Tar, parent),
    Init_Nfunc_Entry(1, Tgz, construct, __construct),
    Init_Nfunc_Entry(2, Tgz, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Tgz, compress, __compress),
    Init_Vfunc_Entry(4, Tgz, uncompress, __uncompress),
    Init_End___Entry(5, Tgz),
};
REGISTER_CLASS(Tgz, tgz_class_info);