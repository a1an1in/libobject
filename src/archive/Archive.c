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
    allocator_t *allocator = archive->parent.allocator;
    
    archive->file = object_new(allocator, "File", NULL);
    archive->wildchard = object_new(allocator, "String", NULL);
    archive->extracting_path = object_new(allocator, "String", NULL);
    archive->extracting_path->assign(archive->extracting_path, "./");
    archive->adding_path = object_new(allocator, "String", NULL);

    return 0;
}

static int __deconstruct(Archive *archive)
{
    object_destroy(archive->file);
    object_destroy(archive->wildchard);
    object_destroy(archive->extracting_path);
    object_destroy(archive->adding_path);
    return 0;
}

static int __open(Archive *archive, char *archive_name, char *mode)
{
    File *file = archive->file;
    int (*func)(Archive *archive, char *archive_name, char *mode);
    int ret;

    TRY {
        dbg_str(DBG_VIP, "Archive open file %s", archive_name);
        EXEC(file->open(file, archive_name, mode));
        func = object_get_progeny_class_first_normal_func(((Obj *)archive)->target_name, "Archive", "open");
        if (func != NULL) {
            EXEC(func(archive, archive_name, mode));
        }
    } CATCH (ret) {}

    return ret;
}

static int __close(Archive *archive)
{
    File *file = archive->file;

    return TRY_EXEC(file->close(file));
}

static int __set_wildchard(Archive *archive, char *wildchard)
{
    String *w = archive->wildchard; 

    return TRY_EXEC(w->assign(w, wildchard));
}

static int __set_extracting_path(Archive *archive, char *path)
{
    String *p = archive->extracting_path;

    p->assign(p, path);

    return 0;
}

static int __set_adding_path(Archive *archive, char *path)
{
    String *p = archive->adding_path;

    p->assign(p, path);

    return 0;
}

static int __get_file_list(Archive *archive, char *file_list, int num)
{
    String *p = archive->adding_path;
}

static int __extract_files(Archive *a, String *files)
{

}

static int __extract(Archive *a)
{

}

static int __add_files(Archive *a, String *files)
{

}

static int __add(Archive *a)
{

}

static class_info_entry_t archive_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Archive, construct, __construct),
    Init_Nfunc_Entry(2 , Archive, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3 , Archive, open, __open),
    Init_Nfunc_Entry(4 , Archive, close, __close),
    Init_Vfunc_Entry(5 , Archive, set_wildchard, __set_wildchard),
    Init_Vfunc_Entry(6 , Archive, set_extracting_path, __set_extracting_path),
    Init_Vfunc_Entry(7 , Archive, set_adding_path, __set_adding_path),
    Init_Vfunc_Entry(8 , Archive, extract_file, NULL),
    Init_Vfunc_Entry(9 , Archive, extract_files, __extract_files),
    Init_Vfunc_Entry(10, Archive, extract, __extract),
    Init_Vfunc_Entry(11, Archive, add_file, NULL),
    Init_Vfunc_Entry(12, Archive, add_files, __add_files),
    Init_Vfunc_Entry(13, Archive, add, __add),
    Init_End___Entry(14, Archive),
};
REGISTER_CLASS("Archive", archive_class_info);

