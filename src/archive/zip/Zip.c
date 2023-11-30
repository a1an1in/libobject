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
    zip->buffer = object_new(allocator, "Buffer", NULL);

    zip->central_dir_position = 0;
    zip->end_of_central_dir_position = 0;

    return 0;
}

static int __deconstruct(Zip *zip)
{
    object_destroy(zip->file);
    object_destroy(zip->file_name);
    object_destroy(zip->buffer);

    return 0;
}

static int __search_central_directory_end_header_position(Zip *zip, uint64_t *offset)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file;
    uint8_t buf[512]; // we assume the comment is less than 512 bytes.
    Buffer *buffer = zip->buffer;
    uint8_t needle[4] = {0x50, 0x4b, 0x5, 0x6};
    uint8_t *addr;
    uint64_t size, read_size;
    int ret;

    TRY {
        size = a->get_size(a); 
        //the file size must greater than sizeof end of centrol dir.
        THROW_IF(size < 22, -1);
        dbg_str(DBG_VIP, "search_central_dir_position, file size:%d", size);

        read_size = size > 512 ? 512 : size;
        a->read(a, buf, read_size);
        buffer->set_capacity(buffer, read_size);
        buffer->write(buffer, buf, read_size);
        addr = buffer->rfind(buffer, needle, sizeof(needle));
        THROW_IF(addr == NULL, -1);

        *offset = size - read_size + (uint64_t)(addr - (uint8_t *)buffer->addr);
        dbg_str(DBG_VIP, "search_central_dir_position, address:%p, offset:%lld", addr, *offset);
        dbg_buf(DBG_VIP, "content:", addr, 4);
    } CATCH (ret) {}

    return ret;
}

static int __get_central_directory_end_header(Zip *zip)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file;
    zip_central_directory_end_header_t *header = &zip->central_directory_end_header;;
    uint8_t buf[512];
    int ret;

    TRY {
        dbg_str(DBG_VIP, "end_of_central_dir_position:%lld", zip->end_of_central_dir_position);
        a->seek(a, zip->end_of_central_dir_position, SEEK_SET);
        a->read(a, (uint8_t *)header, sizeof(zip->central_directory_end_header));
        dbg_buf(DBG_VIP, "offset:", header, 22);
        LE32_TO_CPU(header->central_directory_start_offset);
        dbg_str(DBG_VIP, "central_directory_start_offset:%ld", header->central_directory_start_offset);
        // a->read(a, (uint8_t *)buf, sizeof(zip->central_directory_end_header));
        // dbg_buf(DBG_VIP, "offset:", buf, 22);
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
            EXEC(__search_central_directory_end_header_position(zip, &zip->end_of_central_dir_position));
            EXEC(__get_central_directory_end_header(zip));
        }
        
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

