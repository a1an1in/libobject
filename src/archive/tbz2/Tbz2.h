#ifndef __TBZ_H__
#define __TBZ_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/File.h>
#include <libobject/archive/Archive.h>
#include <libobject/archive/Tar.h>
#include <libobject/compress/Compress.h>

typedef struct Tbz_s Tbz2;

struct Tbz_s {
    Tar parent;

    int (*construct)(Tbz2 *, char *);
    int (*deconstruct)(Tbz2 *);

    /*virtual methods reimplement*/
    int (*set)(Tbz2 *, char *attrib, void *value);
    void *(*get)(Tbz2 *, char *attrib);
    char *(*to_json)(Tbz2 *);
	int (*add_file)(Tbz2 *, archive_file_info_t *info);
	int (*extract_file)(Tbz2 *, archive_file_info_t *info);
	int (*compress)(Tbz2 *, char *file_in, char *file_out, int out_len);
	int (*uncompress)(Tbz2 *, char *file_in, char *file_out, int out_len);

    Compress *c;
    File *file;
	String *file_name;
};

#endif
