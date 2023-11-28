#ifndef __ZIP_H__
#define __ZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/archive/Archive.h>

typedef struct Zip_s Zip;

struct Zip_s {
    Archive parent;

    int (*construct)(Zip *, char *);
    int (*deconstruct)(Zip *);

    /*virtual methods reimplement*/
    int (*set)(Zip *, char *attrib, void *value);
    void *(*get)(Zip *, char *attrib);
    char *(*to_json)(Zip *); 
    int (*extract_file)(Zip *, char *file_name);
    int (*add_file)(Zip *, char *file_name);

    File *file;
	String *file_name;
    uint64_t end_of_central_dir_position;
    uint64_t central_dir_position;
};

#endif
