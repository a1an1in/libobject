#ifndef __ZIP_H__
#define __ZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>

typedef struct Zip_s Zip;

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

    zip_central_directory_end_header_t central_directory_end_header;
};


#endif
