#ifndef __OBJ_VECTOR_H__
#define __OBJ_VECTOR_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/utils/data_structure/vector.h>

typedef struct _vector_s Vector;

struct _vector_s{
	Obj obj;

	int (*construct)(Vector *vector,char *init_str);
	int (*deconstruct)(Vector *vector);
	int (*set)(Vector *vector, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*set_data)(Vector *vector,int index, void *value);
    int (*get_data)(Vector *vector,int index, void **value);
    void (*for_each_by_index)(Vector *vector,void (*func)(Vector *vector,int index));

    vector_t *vector;
	uint32_t value_size;
    uint32_t capacity;
};

#endif
