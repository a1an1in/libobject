#ifndef __ZIP_H__
#define __ZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct Zip_s Zip;

#define ZIP_FILE_HEADER_SIZE 30
#define ZIP_CENTROL_DIR_HEADER_SIZE 46
#define ZIP_CENTROL_DIR_END_HEADER_SIZE 22

typedef enum zip_compression_method_enum {
    ZIP_COMPRESSION_METHOD_STORED = 0,
    ZIP_COMPRESSION_METHOD_DEFLATED = 8,
    ZIP_COMPRESSION_METHOD_MAX
} zip_compression_method_e;

/*
 * The End of Central Directory Structure header has following format :
 * end of central dir signature    4 bytes  (0x06054b50)
 * number of this disk             2 bytes
 * number of the disk with the
 * start of the central directory  2 bytes
 * total number of entries in the
 * central directory on this disk  2 bytes
 * total number of entries in
 * the central directory           2 bytes
 * size of the central directory   4 bytes
 * offset of start of central
 * directory with respect to
 * the starting disk number        4 bytes
 * .ZIP file comment length        2 bytes
 * .ZIP file comment       (variable size)
*/

typedef struct zip_the_end_of_central_directory_header_s {
    uint32_t signature;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
    uint16_t central_directory_total_number;
    uint32_t central_directory_size;
    uint32_t central_directory_start_offset;
    uint16_t comment_length;
    uint8_t comment[];
} zip_central_directory_end_header_t;

/*
 *  Central directory structure:
 *    [central directory header 1]
 *    ...
 *    [central directory header n]
 *    [digital signature] 
 *  File header:
 *    central file header signature   4 bytes  (0x02014b50)
 *    version made by                 2 bytes
 *    version needed to extract       2 bytes
 *    general purpose bit flag        2 bytes
 *    compression method              2 bytes
 *    last mod file time              2 bytes
 *    last mod file date              2 bytes
 *    crc-32                          4 bytes
 *    compressed size                 4 bytes
 *    uncompressed size               4 bytes
 *    file name length                2 bytes
 *    extra field length              2 bytes
 *    file comment length             2 bytes
 *    disk number start               2 bytes
 *    internal file attributes        2 bytes
 *    external file attributes        4 bytes
 *    relative offset of local header 4 bytes
 *
 *    file name (variable size)
 *    extra field (variable size)
 *    file comment (variable size)
 */

typedef struct zip_central_directory_header_s {
    uint32_t signature;
    uint16_t create_version;
    uint16_t extract_version;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mode_time;
    uint16_t last_mode_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t start_disk_number;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t offset;
    uint8_t *file_name;
    uint8_t *extra_field;
    uint8_t *comment;
    void *opaque;
} __attribute__ ((packed)) zip_central_directory_header_t;

/*
 * file header
 * local file header signature     4 bytes  (0x04034b50)
 * version needed to extract       2 bytes
 * general purpose bit flag        2 bytes
 * compression method              2 bytes
 * last mod file time              2 bytes
 * last mod file date              2 bytes
 * crc-32                          4 bytes
 * compressed size                 4 bytes
 * uncompressed size               4 bytes
 * file name length                2 bytes
 * extra field length              2 bytes
 * file name (variable size)
 * extra field (variable size)
 */
typedef struct zip_file_header_s {
    uint32_t signature;
    uint16_t extract_version;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mode_time;
    uint16_t last_mode_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint8_t *file_name;
    uint8_t *extra_field;
    uint32_t data_offset;
} __attribute__ ((packed)) zip_file_header_t;

struct Zip_s {
    Archive parent;

    int (*construct)(Zip *, char *);
    int (*deconstruct)(Zip *);
    int (*open)(Zip *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(Zip *, char *attrib, void *value);
    void *(*get)(Zip *, char *attrib);
    char *(*to_json)(Zip *); 
    int (*extract_file)(Zip *, char *file_name);
    int (*add_file)(Zip *, char *file_name);

    File *file;
	String *file_name;
    Buffer *buffer;
    uint64_t central_dir_end_header_position;
    uint64_t central_dir_position;
    uint64_t last_file_position;
    uint8_t write_central_dir_flag;

    zip_central_directory_end_header_t central_directory_end_header;
    Vector *headers;
    Vector *compressors;
};

#endif
