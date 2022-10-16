#ifndef __SQL_RESULT_H__
#define __SQL_RESULT_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>
#include <libobject/database/sql/Row.h>

typedef struct Result_s Result;

struct Result_s{
    Obj parent;

    int (*construct)(Result *,char *);
    int (*deconstruct)(Result *);

    /*virtual methods reimplement*/
    int (*set)(Result *result, char *attrib, void *value);
    void *(*get)(Result *, char *attrib);
    Row *(*get_row)(Result *, int index);
    int (*set_row)(Result *, int index, Row *row);
    int (*set_row_at_back)(Result *, Row *row);
    int (*get_row_count)(Result *);
    int (*get_column_count)(Result *);
    int (*set_column)(Result *result, int index, String *data);
    String *(*get_column)(Result *result, int index);
    int (*reset)(Result *);

    /*attribs*/
    Vector *rows;
    Vector *columns;
};

#endif
