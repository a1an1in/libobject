#ifndef __MODEL_H__
#define __MODEL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct Model_s Model;

struct Model_s{
    Obj parent;

    int (*construct)(Model *,char *);
    int (*deconstruct)(Model *);

    /*virtual methods reimplement*/
    int (*set)(Model *model, char *attrib, void *value);
    void *(*get)(Model *, char *attrib);
    char *(*to_json)(Model *); 
    int (*add)(Model *dst, Model *src);
    char *(*get_table_class_name)(Model *model);
    int (*set_table_class_name)(Model *model, char *table_class_name);
    int (*cut)(Model *model);

    /*attribs*/
    String *table_class_name;
};

#endif
