/**
 * @file Zip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Zip.h"

int __centrol_header_free_callback(allocator_t *allocator, zip_central_directory_header_t *header)
{
    allocator_mem_free(allocator, header->file_name);
    allocator_mem_free(allocator, header);

    return 1;
}

static int __construct(Zip *zip, char *init_str)
{
    allocator_t *allocator = zip->parent.parent.allocator;
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    uint8_t trustee_flag = 1;
    Vector *headers;

    zip->file = object_new(allocator, "File", NULL);
    zip->file_name = object_new(allocator, "String", NULL);
    zip->buffer = object_new(allocator, "Buffer", NULL);

    zip->headers = object_new(allocator, "Vector", NULL);
    headers = zip->headers;
    headers->set_trustee(headers, value_type, __centrol_header_free_callback);

    zip->central_dir_position = 0;
    zip->central_dir_end_header_position = 0;

    return 0;
}

static int __deconstruct(Zip *zip)
{
    object_destroy(zip->headers);
    object_destroy(zip->buffer);
    object_destroy(zip->file_name);
    object_destroy(zip->file);

    return 0;
}

static int __search_central_directory_end_header_position(Zip *zip, uint64_t *offset)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file;
    uint8_t buf[512]; // TODO: we assume the comment is less than 512 bytes.
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
    zip_central_directory_end_header_t *header = &zip->central_directory_end_header;
    uint8_t buf[512];
    int ret;

    TRY {
        dbg_str(DBG_VIP, "central_dir_end_header_position:%lld", zip->central_dir_end_header_position);
        a->seek(a, zip->central_dir_end_header_position, SEEK_SET);
        a->read(a, (uint8_t *)header, sizeof(zip->central_directory_end_header));
        dbg_buf(DBG_VIP, "offset:", header, 22);

        LE32_TO_CPU(header->central_directory_total_number);
        LE32_TO_CPU(header->central_directory_size);
        LE32_TO_CPU(header->central_directory_start_offset);
        dbg_str(DBG_VIP, "central_directory_start_offset:%ld", header->central_directory_start_offset);
    } CATCH (ret) {}
    
    return ret;
}

static int __get_central_directory_headers(Zip *zip)
{
    Archive *archive = (Archive *)&zip->parent;
    allocator_t *allocator = zip->parent.parent.allocator;
    File *a = archive->file;
    zip_central_directory_end_header_t *end_header = &zip->central_directory_end_header;
    Vector *headers = zip->headers;
    zip_central_directory_header_t *header;
    uint8_t buf[512];
    int ret, i, read_size = 0;

    TRY {
        dbg_str(DBG_VIP, "dir num %d", end_header->central_directory_total_number);
        for (i = 0; i < end_header->central_directory_total_number; i++) {
            header = allocator_mem_zalloc(allocator, sizeof(zip_central_directory_header_t));
            a->seek(a, end_header->central_directory_start_offset + read_size, SEEK_SET);
            a->read(a, (uint8_t *)header, sizeof(zip_central_directory_header_t) - 3 * sizeof(void *));
            LE32_TO_CPU(header->signature);
            LE16_TO_CPU(header->create_version);
            LE16_TO_CPU(header->extract_version);
            LE16_TO_CPU(header->general_purpose_bit_flag);
            LE16_TO_CPU(header->compression_method);
            LE16_TO_CPU(header->last_mode_time);
            LE16_TO_CPU(header->last_mode_date);
            LE32_TO_CPU(header->crc32);
            LE32_TO_CPU(header->compressed_size);
            LE32_TO_CPU(header->uncompressed_size);
            LE16_TO_CPU(header->file_name_length);
            LE16_TO_CPU(header->extra_field_length);
            LE16_TO_CPU(header->file_comment_length);
            LE16_TO_CPU(header->start_disk_number);
            LE16_TO_CPU(header->internal_file_attributes);
            LE32_TO_CPU(header->external_file_attributes);
            LE32_TO_CPU(header->offset);

            if (header->file_name_length) {
                header->file_name = allocator_mem_zalloc(allocator, header->file_name_length + 1);
                a->seek(a, end_header->central_directory_start_offset + read_size + sizeof(zip_central_directory_header_t) - 3 * sizeof(void *), SEEK_SET);
                a->read(a, (uint8_t *)header->file_name, header->file_name_length);
            }
            printf("\n");
            dbg_str(DBG_VIP, "central dir, name:%s", header->file_name);
            dbg_str(DBG_VIP, "central dir, offset:%x", header->offset);
            dbg_str(DBG_VIP, "central dir, compression_method:%d", header->compression_method);
            dbg_str(DBG_VIP, "central dir, compressed_size:%d", header->compressed_size);
            dbg_str(DBG_VIP, "central dir, uncompressed_size:%d", header->uncompressed_size);
            dbg_str(DBG_VIP, "central dir, general_purpose_bit_flag:%d", header->general_purpose_bit_flag);
            dbg_str(DBG_VIP, "central dir, extract_version:%d", header->extract_version);
            dbg_str(DBG_VIP, "central dir, create_version:%d", header->create_version);
            dbg_str(DBG_VIP, "central dir, last_mode_time:%x", header->last_mode_time);
            dbg_str(DBG_VIP, "central dir, last_mode_date:%x", header->last_mode_date);
            dbg_str(DBG_VIP, "central dir, extra_field_length:%x", header->extra_field_length);
            dbg_str(DBG_VIP, "central dir, file_comment_length:%x", header->file_comment_length);
            dbg_str(DBG_VIP, "central dir, crc32:%x", header->crc32);
    
            EXEC(headers->add(headers, header));
            read_size += sizeof(zip_central_directory_header_t) - 3 * sizeof(void *)
                         + header->file_name_length
                         + header->extra_field_length + header->file_comment_length;
        }
    } CATCH (ret) {}
    
    return ret;
}

static int __open(Zip *zip, char *archive_name, char *mode)
{
    int ret;

    TRY {
        dbg_str(DBG_VIP, "Zip open archive %s", archive_name);

        EXEC(__search_central_directory_end_header_position(zip, &zip->central_dir_end_header_position));
        EXEC(__get_central_directory_end_header(zip));
        EXEC(__get_central_directory_headers(zip));
    } CATCH (ret) {}

    return ret;
}

static int __c_string_vector_element_cmp(zip_central_directory_header_t *element, void *key)
{
    if (strcmp(element->file_name, key) == 0) {
        return 1;
    }

    return 0;
}

static zip_central_directory_header_t *
__search_central_directory_header(Zip *zip, char *file_name)
{
    Vector *headers = zip->headers;
    void *element = NULL;
    int ret;

    TRY {
        EXEC(headers->search(headers, __c_string_vector_element_cmp, file_name, &element, NULL));
    } CATCH (ret) {
        element = NULL;
    }

    return element;
}

static int __get_file_header(Zip *zip, zip_central_directory_header_t *record, zip_file_header_t *header)
{
    Archive *archive = (Archive *)&zip->parent;
    allocator_t *allocator = zip->parent.parent.allocator;
    File *a = archive->file;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "get_file_header, offset:%d", record->offset);
        a->seek(a, record->offset, SEEK_SET);
        a->read(a, (uint8_t *)header, sizeof(zip_file_header_t));
        if (header->file_name_length) {
            // header->file_name = allocator_mem_zalloc(allocator, header->file_name_length + 1);
            // a->seek(a, record->offset + sizeof(zip_file_header_t) - 2 * sizeof(void *), SEEK_SET);
            // a->read(a, (uint8_t *)header->file_name, header->file_name_length);
        }
        THROW_IF(header->crc32 != record->crc32, -1);
        THROW_IF(header->compression_method != record->compression_method, -1);
        THROW_IF(header->compressed_size != record->compressed_size, -1);
        THROW_IF(header->uncompressed_size != record->uncompressed_size, -1);
        THROW_IF(header->extract_version != record->extract_version, -1);
        THROW_IF(header->general_purpose_bit_flag != record->general_purpose_bit_flag, -1);
        

    } CATCH (ret) {}

    return ret;
}

static int __extract_file(Zip *zip, char *file_name)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file, *file = zip->file;
    zip_central_directory_header_t *element;
    zip_file_header_t header;
    int ret;
    char buf[512] = {0};

    TRY {
        element = __search_central_directory_header(zip, file_name);
        THROW_IF(element == NULL, -1);

        dbg_str(DBG_VIP, "zip extract_file, name:%s", element->file_name);
        dbg_str(DBG_VIP, "zip extract_file, offset:%x", element->offset);
        dbg_str(DBG_VIP, "zip extract_file, compression_method:%d", element->compression_method);
        dbg_str(DBG_VIP, "zip extract_file, compressed_size:%d", element->compressed_size);
        dbg_str(DBG_VIP, "zip extract_file, uncompressed_size:%d", element->uncompressed_size);
        dbg_str(DBG_VIP, "zip extract_file, general_purpose_bit_flag:%d", element->general_purpose_bit_flag);
        dbg_str(DBG_VIP, "zip extract_file, extract_version:%d", element->extract_version);
        EXEC(__get_file_header(zip, element, &header));
        
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
    Init_Nfunc_Entry(3, Zip, open, __open),
    Init_Vfunc_Entry(4, Zip, extract_file, __extract_file),
    Init_Vfunc_Entry(5, Zip, add_file, __add_file),
    Init_End___Entry(6, Zip),
};
REGISTER_CLASS("Zip", zip_class_info);

