#ifndef __X_7Z_H__
#define __X_7Z_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct X_7Z_s X_7Z;

struct X_7Z_s {
    Archive parent;

    int (*construct)(X_7Z *, char *);
    int (*deconstruct)(X_7Z *);
    int (*open)(X_7Z *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(X_7Z *, char *attrib, void *value);
    void *(*get)(X_7Z *, char *attrib);
    char *(*to_json)(X_7Z *); 
    int (*extract_file)(X_7Z *, char *file_name);
    int (*add_file)(X_7Z *, char *file_name);
    int (*get_file_infos)(X_7Z *zip);

    File *file;
	String *file_name;
};

#endif
