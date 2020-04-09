#ifndef __OBJ_VECTOR_H__
#define __OBJ_VECTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct _vector_s Vector;

struct _vector_s{
    Obj obj;

    int (*construct)(Vector *vector,char *init_str);
    int (*deconstruct)(Vector *vector);

    /*virtual methods reimplement*/
    int (*reconstruct)(Vector *vector);
    int (*set)(Vector *vector, char *attrib, void *value);
    void *(*get)(Vector *vector, char *attrib);
    char *(*to_json)(Vector *vector);
    int (*add)(Vector *vector,void *value);
    int (*add_at)(Vector *vector,int index, void *value);
    int (*add_back)(Vector *vector,void *value);
    int (*remove)(Vector *vector, int index, void **value);
    int (*remove_back)(Vector *vector,void **value);
    int (*peek_at)(Vector *vector,int index, void **value);
    void (*for_each)(Vector *vector,void (*func)(int index, void *element));
    void (*free_vector_elements)(Vector *vector);
    uint32_t (*count)(Vector *vector);
    int (*empty)(Vector *vector);
    void (*reset)(Vector *vector);

    /*attribs*/
    vector_t *vector;
    uint32_t value_size;
    uint8_t value_type;
    uint32_t capacity;
    String *init_data;
    String *class_name;
    uint8_t trustee_flag;
};

#endif
