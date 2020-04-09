#ifndef __OBJECT_CHAIN_H__
#define __OBJECT_CHAIN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/List.h>

typedef struct Object_Chain_s Object_Chain;

struct Object_Chain_s{
    Obj parent;

    int (*construct)(Object_Chain *,char *);
    int (*deconstruct)(Object_Chain *);

    /*virtual methods reimplement*/
    int (*set)(Object_Chain *list, char *attrib, void *value);
    void *(*get)(Object_Chain *, char *attrib);
    void *(*new)(Object_Chain *, char *class_name, char *data);

    List *list;
};

#endif
