#ifndef __STRUCT_ADAPTER_H__
#define __STRUCT_ADAPTER_H__

#include <stdio.h>
#include <libobject/core/String.h>

typedef struct Struct_Adapter_s Struct_Adapter;
struct Struct_Adapter_s {
    Obj parent;

    int (*construct)(Struct_Adapter *,char *);
    int (*deconstruct)(Struct_Adapter *);

    int (*set)(Struct_Adapter *, char *attrib, void *value);
    void *(*get)(Struct_Adapter *, char *attrib);

    /* operation attribs */
    void *new;
    void *free;
    void *to_json;
    void *print;
};

#endif