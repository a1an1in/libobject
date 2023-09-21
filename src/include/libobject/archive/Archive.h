#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Archive_s Archive;

struct Archive_s{
    Obj parent;

    int (*construct)(Archive *, char *);
    int (*deconstruct)(Archive *);

    /*virtual methods reimplement*/
    int (*set)(Archive *archive, char *attrib, void *value);
    void *(*get)(Archive *, char *attrib);
    char *(*to_json)(Archive *); 
};

#endif
