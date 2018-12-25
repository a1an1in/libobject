#ifndef __OBJ_VECTOR_H__
#define __OBJ_VECTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/obj.h>

typedef struct _vector_s Vector;

struct _vector_s{
	Obj obj;

	int (*construct)(Vector *vector,char *init_str);
	int (*deconstruct)(Vector *vector);
	int (*set)(Vector *vector, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Vector *vector,void *value);
    int (*add_at)(Vector *vector,int index, void *value);
    int (*add_back)(Vector *vector,void *value);
    int (*remove)(Vector *vector, int index, void **value);
    int (*remove_back)(Vector *vector,void **value);
    int (*peek_at)(Vector *vector,int index, void **value);
    void (*for_each)(Vector *vector,void (*func)(int index, void *element));
    void (*free_vector_elements)(Vector *vector);
    uint32_t (*size)(Vector *vector);
    int    (*empty)(Vector *vector);

    void (*clear)(Vector *vector);

    vector_t *vector;
	uint32_t value_size;
    uint32_t capacity;
};

#endif
