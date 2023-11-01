#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/io/File.h>

typedef struct Archive_s Archive;

struct Archive_s{
    Obj parent;

    int (*construct)(Archive *, char *);
    int (*deconstruct)(Archive *);

    /*virtual methods reimplement*/
    int (*set)(Archive *archive, char *attrib, void *value);
    void *(*get)(Archive *, char *attrib);
    char *(*to_json)(Archive *); 
    int (*compress)(Archive *c, char *file_in, char *file_out);
    int (*uncompress)(Archive *c, char *file_in, char *file_out);
    int (*open)(Archive *a, char *archive_name);
	int (*close)(Archive *a);
    int (*extract)(Archive *a);
    int (*add_file)(Archive *a, char *file_name);
    int (*put)(Archive *a);
    int (*set_path)(Archive *a, char *path);
    int (*set_wildchard)(Archive *archive, char *wildchard);

    String *path;
    String *wildchard; 
    File *file;
    char **file_list;
    int file_count;
};

#endif
