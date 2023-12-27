#ifndef __X_7Z_H__
#define __X_7Z_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct X_7Z_s SevenZip;

struct X_7Z_s {
    Archive parent;

    int (*construct)(SevenZip *, char *);
    int (*deconstruct)(SevenZip *);
    int (*open)(SevenZip *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(SevenZip *, char *attrib, void *value);
    void *(*get)(SevenZip *, char *attrib);
    char *(*to_json)(SevenZip *); 
    int (*extract_file)(SevenZip *, char *file_name);
    int (*add_file)(SevenZip *, char *file_name);
    int (*get_extracting_file_infos)(SevenZip *zip, Vector **infos);

    File *file;
	String *file_name;
};

#endif
