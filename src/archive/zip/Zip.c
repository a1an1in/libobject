/**
 * @file Zip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include "Zip.h"

static int __free_centrol_header_callback(allocator_t *allocator, zip_central_directory_header_t *header)
{
    allocator_mem_free(allocator, header->file_name);
    allocator_mem_free(allocator, header->extra_field);
    allocator_mem_free(allocator, header->comment);
    allocator_mem_free(allocator, header);

    return 1;
}

static int __free_file_header_callback(allocator_t *allocator, zip_file_header_t *header)
{
    if (header == NULL) return 0;

    allocator_mem_free(allocator, header->file_name);
    allocator_mem_free(allocator, header->extra_field);
    allocator_mem_free(allocator, header);

    return 1;
}

static int __config_default_compressors(Zip *zip)
{
    Vector *headers = zip->compressors;
    allocator_t *allocator = zip->parent.parent.allocator;
    Compress *c;
    int ret;

    TRY {
        c = object_new(allocator, "DeflateCompress", NULL);
        THROW_IF(c == NULL, -1);
        dbg_str(DBG_VIP, "decompress compressor:%p", c);
        headers->add_at(headers, ZIP_COMPRESSION_METHOD_STORED, NULL);
        headers->add_at(headers, ZIP_COMPRESSION_METHOD_DEFLATED, c);
    } CATCH (ret) {}
}

static int __construct(Zip *zip, char *init_str)
{
    allocator_t *allocator = zip->parent.parent.allocator;
    Vector *headers;

    zip->file = object_new(allocator, "File", NULL);
    zip->file_name = object_new(allocator, "String", NULL);
    zip->buffer = object_new(allocator, "Buffer", NULL);

    zip->headers = object_new(allocator, "Vector", NULL);
    headers = zip->headers;
    headers->set_trustee(headers, VALUE_TYPE_STRUCT_POINTER, __free_centrol_header_callback);

    zip->compressors = object_new(allocator, "Vector", NULL);
    headers = zip->compressors;
    headers->set_trustee(headers, VALUE_TYPE_OBJ_POINTER, NULL);
    __config_default_compressors(zip);

    zip->central_dir_position = 0;
    zip->central_dir_end_header_position = 0;

    return 0;
}

static int __deconstruct(Zip *zip)
{
    object_destroy(zip->compressors);
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

static int __read_central_directory_end_header(Zip *zip)
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

static int __read_central_directory_headers(Zip *zip)
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
            dbg_str(DBG_VIP, "central dir, internal_file_attributes:%x", header->internal_file_attributes);
            dbg_str(DBG_VIP, "central dir, external_file_attributes:%x", header->external_file_attributes);
            dbg_str(DBG_VIP, "central dir, start_disk_number:%x", header->start_disk_number);
            dbg_str(DBG_VIP, "central dir, create_version:%x", header->create_version);
            dbg_str(DBG_VIP, "central dir, extract_version:%x", header->extract_version);
    
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
    int ret, size;

    TRY {
        dbg_str(DBG_VIP, "Zip open archive %s", archive_name);
        size = fs_get_size(archive_name);
        if (size == 0) return 0;

        EXEC(__search_central_directory_end_header_position(zip, &zip->central_dir_end_header_position));
        EXEC(__read_central_directory_end_header(zip));
        EXEC(__read_central_directory_headers(zip));
    } CATCH (ret) {}

    return ret;
}

static int __vector_cstr_element_cmp(zip_central_directory_header_t *element, void *key)
{
    if (strcmp(element->file_name, key) == 0) {
        return 1;
    }

    return 0;
}

static zip_central_directory_header_t *__search_central_directory_header(Zip *zip, char *file_name)
{
    Vector *headers = zip->headers;
    void *element = NULL;
    int ret;

    TRY {
        EXEC(headers->search(headers, __vector_cstr_element_cmp, file_name, &element, NULL));
    } CATCH (ret) {
        element = NULL;
    }

    return element;
}

static int __read_file_header(Zip *zip, zip_central_directory_header_t *record, zip_file_header_t *header)
{
    Archive *archive = (Archive *)&zip->parent;
    allocator_t *allocator = zip->parent.parent.allocator;
    File *a = archive->file;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "get_file_header, offset:%d", record->offset);
        a->seek(a, record->offset, SEEK_SET);
        a->read(a, (uint8_t *)header, ZIP_FILE_HEADER_SIZE);
        if (header->file_name_length) {
            header->file_name = allocator_mem_zalloc(allocator, header->file_name_length + 1);
            a->seek(a, record->offset + ZIP_FILE_HEADER_SIZE, SEEK_SET);
            a->read(a, (uint8_t *)header->file_name, header->file_name_length);
        }
        header->data_offset = ZIP_FILE_HEADER_SIZE + record->offset + 
                              header->file_name_length + 
                              header->extra_field_length;
        dbg_str(DBG_VIP, "file_header, file data offset:%d", header->data_offset);
        dbg_str(DBG_VIP, "file_header, file name in file header:%s", header->file_name);
        dbg_str(DBG_VIP, "file_header, crc32:%x", header->crc32);
        dbg_str(DBG_VIP, "file_header, file_name_length:%x", header->file_name_length);
        dbg_str(DBG_VIP, "file_header, extra_field_length:%x", header->extra_field_length);
        dbg_str(DBG_VIP, "file_header, last_mode_time:%x", header->last_mode_time);
        dbg_str(DBG_VIP, "file_header, last_mode_date:%x", header->last_mode_date);
        dbg_str(DBG_VIP, "file_header, compression_method:%x", header->compression_method);
        dbg_str(DBG_VIP, "file_header, compressed_size:%x", header->compressed_size);
        dbg_str(DBG_VIP, "file_header, uncompressed_size:%x", header->uncompressed_size);
        dbg_str(DBG_VIP, "file_header, extract_version:%x", header->extract_version);
        dbg_str(DBG_VIP, "file_header, general_purpose_bit_flag:%x", header->general_purpose_bit_flag);
        THROW_IF(header->crc32 != record->crc32, -1);
        THROW_IF(header->last_mode_time != record->last_mode_time, -1);
        THROW_IF(header->last_mode_date != record->last_mode_date, -1);
        THROW_IF(header->compression_method != record->compression_method, -1);
        THROW_IF(header->compressed_size != record->compressed_size, -1);
        THROW_IF(header->uncompressed_size != record->uncompressed_size, -1);
        THROW_IF(header->extract_version != record->extract_version, -1);
        THROW_IF(header->general_purpose_bit_flag != record->general_purpose_bit_flag, -1);
    } CATCH (ret) {}

    return ret;
}

static int __store_file(Zip *zip, File *a, long size, File *out, long *out_size)
{
    int ret, read_len;
    unsigned char buffer[1024];

    TRY {
        while (size > 0) {
            read_len = size > 1024 ? 1024 : size;
            EXEC(a->read(a, buffer, read_len));
            EXEC(out->write(out, buffer, read_len));
            size -= read_len;
        }
        *out_size = size;
    } CATCH (ret) {}

    return ret;
}

static int __decompress_file(Zip *zip, zip_file_header_t *header)
{
    Compress *c = NULL;
    Vector *headers = zip->compressors;
    Archive *archive = (Archive *)&zip->parent;
    allocator_t *allocator = zip->parent.parent.allocator;
    File *a = archive->file, *out = zip->file;
    char file_name[1024];
    int ret, out_len;

    TRY {
        THROW_IF(header->compression_method >= ZIP_COMPRESSION_METHOD_MAX, -1);
        dbg_str(DBG_VIP, "decompress file:%s", header->file_name);
        dbg_str(DBG_VIP, "decompress method:%x", header->compression_method);
        dbg_str(DBG_VIP, "decompress file data_offset:%x", header->data_offset);
        
        strcpy(file_name, STR2A(archive->path));
        strcat(file_name, header->file_name);
        EXEC(fs_mkfile(file_name, 0777));
        EXEC(a->seek(a, header->data_offset, SEEK_SET));
        EXEC(out->open(out, file_name, "wb+"));
        if (header->compression_method == ZIP_COMPRESSION_METHOD_STORED) {
            EXEC(__store_file(zip, a, header->compressed_size, out, &out_len));
        } else {
            headers->peek_at(headers, header->compression_method, &c);
            THROW_IF(c == NULL, -1);
            EXEC(c->uncompress(c, a, header->compressed_size, out, &out_len));
            dbg_str(DBG_VIP, "decompress out len:%x", out_len);
        }
    } CATCH (ret) {} FINALLY {
        out->close(out);
    }

    return ret;
}

static int __extract_file(Zip *zip, char *file_name)
{
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file, *file = zip->file;
    allocator_t *allocator = zip->parent.parent.allocator;
    zip_central_directory_header_t *element;
    zip_file_header_t *header;
    int ret;
    char buf[512] = {0};

    TRY {
        printf("\n");
        dbg_str(DBG_VIP, "zip extract_file, name:%s", file_name);
        element = __search_central_directory_header(zip, file_name);
        THROW_IF(element == NULL, -1);

        dbg_str(DBG_VIP, "zip extract_file, offset:%x", element->offset);
        dbg_str(DBG_VIP, "zip extract_file, compression_method:%x", element->compression_method);
        dbg_str(DBG_VIP, "zip extract_file, compressed_size:%x", element->compressed_size);
        dbg_str(DBG_VIP, "zip extract_file, uncompressed_size:%x", element->uncompressed_size);
        dbg_str(DBG_VIP, "zip extract_file, general_purpose_bit_flag:%x", element->general_purpose_bit_flag);
        dbg_str(DBG_VIP, "zip extract_file, extract_version:%x", element->extract_version);
        header = allocator_mem_zalloc(allocator, sizeof(zip_file_header_t));
        EXEC(__read_file_header(zip, element, header));
        EXEC(__decompress_file(zip, header));
    } CATCH (ret) {} FINALLY {
        __free_file_header_callback(allocator, header);
    }

    return ret;
}

static int __write_file_header(Zip *zip, char *file_name, zip_file_header_t *header)
{
    zip_central_directory_end_header_t *end_header = &zip->central_directory_end_header;
    uint32_t uncompressed_size;
    int ret;

    TRY {
        header->signature = 0x04034b50;
        header->extract_version = 0x14;
        header->general_purpose_bit_flag = 0;
        header->compression_method = ZIP_COMPRESSION_METHOD_DEFLATED;
        header->last_mode_time = 0;
        header->last_mode_date = 0;
        header->file_name_length = strlen(file_name);
        header->extra_field_length = 0;
        header->data_offset = zip->central_dir_position 
                              + header->file_name_length 
                              + header->extra_field_length;

        header->signature = CPU_TO_LE32(header->signature);
        header->extract_version = CPU_TO_LE16(header->extract_version);
        header->general_purpose_bit_flag = CPU_TO_LE16(header->general_purpose_bit_flag);
        header->compression_method = CPU_TO_LE16(header->compression_method);
        header->last_mode_time = CPU_TO_LE16(header->last_mode_time);
        header->last_mode_date = CPU_TO_LE16(header->last_mode_date);
        header->crc32 = CPU_TO_LE32(header->crc32);
        header->compressed_size = CPU_TO_LE32(header->compressed_size);
        header->uncompressed_size = CPU_TO_LE32(header->uncompressed_size);
        header->file_name_length = CPU_TO_LE16(header->file_name_length);
        header->extra_field_length = CPU_TO_LE16(header->extra_field_length);
    } CATCH (ret) {}
    
    return ret;
}

static int __write_file(Zip *zip, char *file_name, zip_file_header_t *header)
{
    Compress *c = NULL;
    Archive *archive = (Archive *)&zip->parent;
    File *a = archive->file, *file = zip->file;
    uint32_t write_len = 0;
    Vector *headers = zip->compressors;
    allocator_t *allocator = zip->parent.parent.allocator;
    int ret;

    TRY {
        /* prepare */
        EXEC(file->open(file, file_name, "r"));
        header->uncompressed_size = fs_get_size(file_name);

        /* compute crc32 */
        EXEC(file_compute_crc32(file_name, &header->crc32));

        /* write file */
        EXEC(a->seek(a, header->data_offset, SEEK_SET));
        EXEC(file->seek(file, 0, SEEK_SET));
        if (header->compression_method == ZIP_COMPRESSION_METHOD_STORED) {
            EXEC(__store_file(zip, file, header->uncompressed_size, a, &write_len));
            header->compressed_size = write_len;
            THROW_IF(write_len != header->uncompressed_size, -1);
        } else {
            headers->peek_at(headers, header->compression_method, &c);
            THROW_IF(c == NULL, -1);
            EXEC(c->compress(c, file, header->uncompressed_size, a, &write_len));
            dbg_str(DBG_VIP, "compress file len:%x", write_len);
            header->compressed_size = write_len;
        }
    } CATCH (ret) {}
    
    return ret;
}

static int __write_central_directory_header(Zip *zip, zip_file_header_t *file_header)
{
    zip_central_directory_header_t *dir_header;
    allocator_t *allocator = zip->parent.parent.allocator;
    Vector *dir_headers = zip->headers;
    uint32_t signature = 0x02014b50;
    int ret;

    TRY {
        dir_header = allocator_mem_zalloc(allocator, sizeof(zip_central_directory_header_t));
        dir_header->signature = CPU_TO_LE32(signature);
        dir_header->create_version = 2;
        dir_header->extract_version = 2;
        dir_header->general_purpose_bit_flag = file_header->general_purpose_bit_flag;
        dir_header->compression_method = file_header->compression_method;
        dir_header->last_mode_time = file_header->last_mode_time;
        dir_header->last_mode_date = file_header->last_mode_date;
        dir_header->crc32 = file_header->crc32;
        dir_header->compressed_size = file_header->compressed_size;
        dir_header->uncompressed_size = file_header->uncompressed_size;
        dir_header->file_name_length = file_header->file_name_length;
        dir_header->extra_field_length = file_header->extra_field_length;
        dir_header->file_comment_length = 0;
        dir_header->start_disk_number = 0;
        dir_header->internal_file_attributes = 0;
        dir_header->external_file_attributes = 0;
        dir_header->offset = CPU_TO_LE32(zip->central_dir_position);
        zip->central_dir_position += (ZIP_FILE_HEADER_SIZE + 
                                      LE32_TO_CPU(file_header->file_name_length) + 
                                      LE32_TO_CPU(file_header->extra_field_length) +
                                      LE32_TO_CPU(file_header->compressed_size));
        dir_headers->add(dir_headers, dir_header);
    } CATCH (ret) {}
    
    return ret;
}

static int __write_central_directory_end_header(Zip *zip)
{
    uint32_t signature = 0x06054b50;
    zip_central_directory_end_header_t *end_header = &zip->central_directory_end_header;
    int ret;

    TRY {
        end_header->signature = CPU_TO_LE32(signature);

    } CATCH (ret) {}

    return ret;
}

static int __add_file(Zip *zip, char *file_name)
{
    zip_file_header_t file_header;
    uint32_t extra_field_length = 0;
    int ret;

    TRY {
        file_header.data_offset = zip->central_dir_position 
                                  + strlen(file_name)
                                  + extra_field_length;

        EXEC(__write_file(zip, file_name, &file_header));
        EXEC(__write_file_header(zip, file_name, &file_header));
        EXEC(__write_central_directory_header(zip, &file_header));
        if (zip->central_dir_end_header_position == 0) {
        }
    } CATCH (ret) {}

    return ret;
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

