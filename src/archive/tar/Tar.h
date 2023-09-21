#ifndef __GZIP_H__
#define __GZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/archive/Archive.h>

typedef struct Tar_s Tar;

struct Tar_s {
    Archive parent;

    int (*construct)(Tar *, char *);
    int (*deconstruct)(Tar *);

    /*virtual methods reimplement*/
    int (*set)(Tar *tar, char *attrib, void *value);
    void *(*get)(Tar *, char *attrib);
    char *(*to_json)(Tar *); 
};

#endif
