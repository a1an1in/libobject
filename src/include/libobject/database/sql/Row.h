#ifndef __SQL_ROW_H__
#define __SQL_ROW_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>

typedef struct Row_s Row;

struct Row_s{
    Obj parent;

    int (*construct)(Row *,char *);
    int (*deconstruct)(Row *);

    /*virtual methods reimplement*/
    int (*set)(Row *row, char *attrib, void *value);
    void *(*get)(Row *, char *attrib);
    String *(*get_column)(Row *, int index);
    int (*set_column)(Row *, int index, String *value);
    int (*get_column_count)(Row *row);
    int (*reset)(Row *row);

    Vector *data;
};

#endif
