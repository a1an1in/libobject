#ifndef __OBJ_VECTOR_H__
#define __OBJ_VECTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct _vector_s Vector;

enum vector_sort_type_e {
    VECTOR_SORT_TYPE_BUBBLE = 0,
    VECTOR_SORT_MAX_TYPE,
};

struct _vector_s{
    Obj obj;

    int (*construct)(Vector *vector,char *init_str);
    int (*deconstruct)(Vector *vector);

    /*virtual methods reimplement*/
    int (*set)(Vector *vector, char *attrib, void *value);
    void *(*get)(Vector *vector, char *attrib);
    char *(*to_json)(Vector *vector);
    int (*add)(Vector *vector,void *value);
    int (*add_at)(Vector *vector,int index, void *value);
    int (*add_vector)(Vector *vector, Vector *src);
    int (*add_back)(Vector *vector,void *value);
    int (*remove)(Vector *vector, int index, void **value);
    int (*remove_back)(Vector *vector,void **value);
    int (*peek_at)(Vector *vector,int index, void **value);
    int (*for_each)(Vector *vector, int (*func)(int index, void *element));
    int (*for_each_arg)(Vector *vector, int (*func)(int index, void *element, void *arg), void *arg);
    void (*free_vector_elements)(Vector *vector);
    uint32_t (*count)(Vector *vector);
    int (*empty)(Vector *vector);
    int (*reset)(Vector *vector);
    int (*assign)(Vector *vector, char *value);
    int (*search)(Vector *vector, int (*cmp)(void *element, void *key), void *key, void **out, int *index);
    int (*get_end_index)(Vector *vector);
    int (*sort)(Vector *vector, enum vector_sort_type_e type, int (*cmp)(void *e1, void *e2));
    int (*reset_from)(Vector *vector, int index);
    int (*filter)(Vector *vector, int (*condition)(void *element, void *cond), void *cond, Vector *out);
    int (*copy)(Vector *vector, Vector *out);
    int (*customize)(Vector *tree, int value_type, int (*value_free_callback)(allocator_t *allocator, void *value));

    /*attribs*/
    vector_t *vector;
    uint32_t value_size;
    uint8_t value_type;
    uint32_t capacity;
    String *class_name; //只有object类型时使用。
    uint8_t trustee_flag;

    /* custom methods for struct */
    int (*value_free_callback)(allocator_t *allocator, void *value);
    int (*value_to_json_callback)(cjson_t *root, void *element);
    int (*value_new_callback)(allocator_t *allocator, void *json, void **value);
};

#endif
