/**
 * @file Zip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Zip.h"

static int __construct(Zip *zip, char *init_str)
{
    allocator_t *allocator = zip->parent.parent.allocator;

    zip->file = object_new(allocator, "File", NULL);
    zip->file_name = object_new(allocator, "String", NULL);
    zip->central_dir_position = 0;

    return 0;
}

static int __deconstruct(Zip *zip)
{
    object_destroy(zip->file);
    object_destroy(zip->file_name);

    return 0;
}

static int __search_end_of_central_dir_position(Zip *zip, uint64_t *offset)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file;
    int ret;
    int size;

    TRY {
        size = a->get_size(a);
        dbg_str(DBG_VIP, "search_central_dir_position, file size:%d", size);
    } CATCH (ret) {}

    return ret;
}

static int __extract_file(Zip *zip, char *file_name)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file, *file = zip->file;
    int ret;
    char buf[512] = {0};

    TRY {
        dbg_str(DBG_VIP, "zip extract_file, name:%s", file_name);
        if (zip->end_of_central_dir_position == 0) {
            EXEC(__search_end_of_central_dir_position(zip, &zip->end_of_central_dir_position));
        }
        
        // a->seek(a, -1024, SEEK_END);
    } CATCH (ret) {}

    return ret;
}

static int __add_file(Zip *zip, char *file_name)
{

}

static class_info_entry_t zip_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Zip, construct, __construct),
    Init_Nfunc_Entry(2, Zip, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Zip, extract_file, __extract_file),
    Init_Vfunc_Entry(4, Zip, add_file, __add_file),
    Init_End___Entry(5, Zip),
};
REGISTER_CLASS("Zip", zip_class_info);

