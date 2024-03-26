#ifndef __UIO_H__
#define __UIO_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Uio_s Uio;

struct Uio_s {
    Obj parent;

    int (*construct)(Uio *,char *);
    int (*deconstruct)(Uio *);

    /*virtual methods reimplement*/
    int (*set)(Uio *module, char *attrib, void *value);
    void *(*get)(Uio *, char *attrib);
    char *(*to_json)(Uio *); 
};

#endif
