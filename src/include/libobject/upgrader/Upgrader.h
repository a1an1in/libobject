#ifndef __UPGRADER_H__
#define __UPGRADER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Upgrader_s Upgrader;

struct Upgrader_s {
    Obj parent;

    int (*construct)(Upgrader *,char *);
    int (*deconstruct)(Upgrader *);

    /*virtual methods reimplement*/
    int (*set)(Upgrader *upgrader, char *attrib, void *value);
    void *(*get)(Upgrader *, char *attrib);
    char *(*to_json)(Upgrader *); 
};

#endif
