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
    archive->path = object_new(allocator, "String", NULL);
    return 0;
}

static int __deconstruct(Archive *archive)
{
    object_destroy(archive->file);
    object_destroy(archive->wildchard);
    object_destroy(archive->path);
    return 0;
}

static int __open(Archive *archive, char *archive_name)
{
    File *file = archive->file;

    return TRY_EXEC(file->open(file, archive_name, "wb+"));
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

static int __set_path(Archive *archive, char *path)
{
    String *p = archive->path;

    p->assign(p, path);

    return 0;
}

static int __get_file_list(Archive *archive, char *file_list, int num)
{
    String *p = archive->path;
}

static int __put(Archive *a)
{

}

static class_info_entry_t archive_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Archive, construct, __construct),
    Init_Nfunc_Entry(2 , Archive, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Archive, open, __open),
    Init_Vfunc_Entry(4 , Archive, close, __close),
    Init_Vfunc_Entry(5 , Archive, set_wildchard, __set_wildchard),
    Init_Vfunc_Entry(6 , Archive, set_path, __set_path),
    Init_Vfunc_Entry(7 , Archive, extract, NULL),
    Init_Vfunc_Entry(8 , Archive, add_file, NULL),
    Init_Vfunc_Entry(9 , Archive, put, __put),
    Init_End___Entry(10, Archive),
};
REGISTER_CLASS("Archive", archive_class_info);

