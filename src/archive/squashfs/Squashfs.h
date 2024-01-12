#ifndef __Squashfs_H__
#define __Squashfs_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct Squashfs_s Squashfs;

struct Squashfs_s {
    Archive parent;

    int (*construct)(Squashfs *, char *);
    int (*deconstruct)(Squashfs *);
    int (*open)(Squashfs *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(Squashfs *, char *attrib, void *value);
    void *(*get)(Squashfs *, char *attrib);
    char *(*to_json)(Squashfs *); 
    int (*extract_file)(Squashfs *, char *file_name);
    int (*add_file)(Squashfs *, char *file_name);
    int (*list)(Squashfs *zip, Vector **infos);

    File *file;
	String *file_name;
};

#endif
