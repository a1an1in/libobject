#ifndef __Explorer_H__
#define __Explorer_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Explorer_s Explorer;

struct Explorer_s {
    Obj parent;

    int (*construct)(Explorer *,char *);
    int (*deconstruct)(Explorer *);

    /*virtual methods reimplement*/
    int (*set)(Explorer *Explorer, char *attrib, void *value);
    void *(*get)(Explorer *, char *attrib);
    char *(*to_json)(Explorer *); 
};

#endif
