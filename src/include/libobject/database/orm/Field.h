#ifndef __ORM_FIELD_H__
#define __ORM_FIELD_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct Field_s Field;

struct Field_s{
    Obj parent;

    int (*construct)(Field *,char *);
    int (*deconstruct)(Field *);

    /*virtual methods reimplement*/
    int (*set)(Field *field, char *attrib, void *value);
    void *(*get)(Field *, char *attrib);

    /*attribs*/
    String *name, *type, *constraint;
    int set_flag:1;
};

#endif
