#ifndef __TBZ_H__
#define __TBZ_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/File.h>
#include <libobject/archive/Archive.h>
#include <libobject/archive/Tar.h>

typedef struct Tbz_s Tbz;

struct Tbz_s {
    Tar parent;

    int (*construct)(Tbz *, char *);
    int (*deconstruct)(Tbz *);

    /*virtual methods reimplement*/
    int (*set)(Tbz *, char *attrib, void *value);
    void *(*get)(Tbz *, char *attrib);
    char *(*to_json)(Tbz *);
	int (*add_file)(Tbz *, archive_file_info_t *info);
	int (*extract_file)(Tbz *, archive_file_info_t *info);
	int (*compress)(Tbz *, char *file_in, char *file_out, int out_len);
	int (*uncompress)(Tbz *, char *file_in, char *file_out, int out_len);

    File *file;
	String *file_name;
};

#endif