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
    int (*open)(Archive *a, char *archive_name, char *mode);
	int (*close)(Archive *a);

    /*virtual methods reimplement*/
    int (*set)(Archive *archive, char *attrib, void *value);
    void *(*get)(Archive *, char *attrib);
    char *(*to_json)(Archive *); 
    int (*compress)(Archive *c, char *file_in, char *file_out);
    int (*uncompress)(Archive *c, char *file_in, char *file_out);
    int (*extract_file)(Archive *a, char *file_name);
    /* extract designated files */
    int (*extract_files)(Archive *a, char *file_list, int num);
    int (*extract)(Archive *a);
    int (*add_file)(Archive *a, char *file_name);
    int (*add_files)(Archive *a, char *file_list, int num);
    int (*add)(Archive *a);
    int (*set_path)(Archive *a, char *path);
    int (*set_wildchard)(Archive *archive, char *wildchard);

    String *path;
    String *wildchard; 
    File *file;
    char **file_list;
    int file_count;
};

#endif
