/**
 * @file SevenZip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-12-19
 */

#include "SevenZip.h"

static int __construct(SevenZip *fs, char *init_str)
{
    allocator_t *allocator = fs->parent.parent.allocator;
    Vector *headers;

    return 0;
}

static int __deconstruct(SevenZip *fs)
{
    return 0;
}

static int __open(SevenZip *fs, char *archive_name, char *mode)
{
    int ret, size;

    TRY {
        dbg_str(DBG_VIP, "SevenZip open archive %s", archive_name);
    } CATCH (ret) {}

    return ret;
}

static int __extract_file(SevenZip *fs, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&fs->parent;
    File *a = archive->file, *file = fs->file;
    allocator_t *allocator = fs->parent.parent.allocator;
    int ret;
    char buf[512] = {0};

    TRY {
        dbg_str(DBG_VIP, "fs extract_file, name:%s", info->file_name);

    } CATCH (ret) {} FINALLY {
    }

    return ret;
}

static int __add_file(SevenZip *fs, archive_file_info_t *info)
{
    uint32_t extra_field_length = 0;
    allocator_t *allocator = fs->parent.parent.allocator;
    int ret;

    TRY {

    } CATCH (ret) {} FINALLY {
    }

    return ret;
}

static int __list(SevenZip *fs)
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
    Init_Nfunc_Entry(1, SevenZip, construct, __construct),
    Init_Nfunc_Entry(2, SevenZip, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, SevenZip, open, __open),
    Init_Vfunc_Entry(4, SevenZip, extract_file, __extract_file),
    Init_Vfunc_Entry(5, SevenZip, add_file, __add_file),
    Init_Vfunc_Entry(6, SevenZip, list, __list),
    Init_End___Entry(7, SevenZip),
};
REGISTER_CLASS(SevenZip, fs_class_info);

