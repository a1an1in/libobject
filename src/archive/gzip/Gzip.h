#ifndef __GZIP_H__
#define __GZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/archive/Archive.h>

typedef struct Gzip_s Gzip;

struct Gzip_s {
    Archive parent;

    int (*construct)(Gzip *, char *);
    int (*deconstruct)(Gzip *);

    /*virtual methods reimplement*/
    int (*set)(Gzip *, char *attrib, void *value);
    void *(*get)(Gzip *, char *attrib);
    char *(*to_json)(Gzip *); 
};

#endif
