/**
 * @file X_7Z.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-12-19
 */

#include "X_7Z.h"

static int __construct(X_7Z *fs, char *init_str)
{
    allocator_t *allocator = fs->parent.parent.allocator;
    Vector *headers;

    return 0;
}

static int __deconstruct(X_7Z *fs)
{
    return 0;
}

static int __open(X_7Z *fs, char *archive_name, char *mode)
{
    int ret, size;

    TRY {
        dbg_str(DBG_VIP, "X_7Z open archive %s", archive_name);
    } CATCH (ret) {}

    return ret;
}

static int __extract_file(X_7Z *fs, char *file_name)
{
    Archive *archive = (Archive *)&fs->parent;
    File *a = archive->file, *file = fs->file;
    allocator_t *allocator = fs->parent.parent.allocator;
    int ret;
    char buf[512] = {0};

    TRY {
        dbg_str(DBG_VIP, "fs extract_file, name:%s", file_name);

    } CATCH (ret) {} FINALLY {
    }

    return ret;
}

static int __add_file(X_7Z *fs, char *file_name)
{
    uint32_t extra_field_length = 0;
    allocator_t *allocator = fs->parent.parent.allocator;
    int ret;

    TRY {

    } CATCH (ret) {} FINALLY {
    }

    return ret;
}

static int __get_file_infos(X_7Z *fs)
{
    Archive *archive = (Archive *)&fs->parent;
    Vector *files = archive->extracting_file_infos;
    int ret;

    TRY {
        THROW_IF(files == NULL, -1);
        files->reset(files);
    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t fs_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, X_7Z, construct, __construct),
    Init_Nfunc_Entry(2, X_7Z, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, X_7Z, open, __open),
    Init_Vfunc_Entry(4, X_7Z, extract_file, __extract_file),
    Init_Vfunc_Entry(5, X_7Z, add_file, __add_file),
    Init_Vfunc_Entry(6, X_7Z, get_file_infos, __get_file_infos),
    Init_End___Entry(7, X_7Z),
};
REGISTER_CLASS("X_7Z", fs_class_info);

