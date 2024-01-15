#ifndef __TGZ_H__
#define __TGZ_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/File.h>
#include <libobject/archive/Archive.h>
#include <libobject/archive/Tar.h>

typedef struct Tgz_s Tgz;

struct Tgz_s {
    Tar parent;

    int (*construct)(Tgz *, char *);
    int (*deconstruct)(Tgz *);

    /*virtual methods reimplement*/
    int (*set)(Tgz *, char *attrib, void *value);
    void *(*get)(Tgz *, char *attrib);
    char *(*to_json)(Tgz *);
	int (*add_file)(Tgz *, char *file_name);
	int (*extract)(Tgz *);
	int (*compress)(Tgz *, char *file_in, char *file_out);
	int (*uncompress)(Tgz *, char *file_in, char *file_out);

    File *file;
	String *file_name;
};

#endif
