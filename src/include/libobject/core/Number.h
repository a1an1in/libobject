#ifndef __NUMBER_H__
#define __NUMBER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Number_s Number;

enum number_type_e {
    NUMBER_TYPE_SIGNED_SHORT = 1,
    NUMBER_TYPE_UNSIGNED_SHORT,
    NUMBER_TYPE_SIGNED_INT,
    NUMBER_TYPE_UNSIGNED_INT,
    NUMBER_TYPE_SIGNED_LONG,
    NUMBER_TYPE_UNSIGNED_LONG,
    NUMBER_TYPE_SIGNED_LONG_LONG,
    NUMBER_TYPE_UNSIGNED_LONG_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE,
    NUMBER_TYPE_BIG_NUMBER,
    NUMBER_TYPE_OBJ_SIGNED_SHORT,
    NUMBER_TYPE_OBJ_UNSIGNED_SHORT,
    NUMBER_TYPE_OBJ_SIGNED_INT,
    NUMBER_TYPE_OBJ_UNSIGNED_INT,
    NUMBER_TYPE_OBJ_SIGNED_LONG,
    NUMBER_TYPE_OBJ_UNSIGNED_LONG,
    NUMBER_TYPE_OBJ_SIGNED_LONG_LONG,
    NUMBER_TYPE_OBJ_UNSIGNED_LONG_LONG,
    NUMBER_TYPE_OBJ_FLOAT,
    NUMBER_TYPE_OBJ_DOUBLE,
    NUMBER_TYPE_OBJ_BIG_NUMBER,
    NUMBER_TYPE_MAX,
};

struct Number_s{
    Obj parent;

    int (*construct)(Number *,char *);
    int (*deconstruct)(Number *);

    /*virtual methods reimplement*/
    int (*set)(Number *number, char *attrib, void *value);
    void *(*get)(Number *, char *attrib);
    char *(*to_json)(Number *); 
    int (*set_type)(Number *number,enum number_type_e type);
    enum number_type_e (*get_type)(Number *number);
    int (*get_size)(Number *number);
    int (*set_value)(Number *number, void *value, int len);
    int (*get_value)(Number *number, void *value, int *len);
    int (*clear)(Number *number);
    int (*add)(Number *number, enum number_type_e type, void *value, int len);

    /*attribs*/
    union {
        short short_data;
        int int_data;
        long long_data;
        unsigned short unsigned_short_data;
        unsigned int unsigned_int_data;
        unsigned long unsigned_long_data;
        float float_data;
        double double_data;
    } data;
    
    void *big_number_data;
    int size;
    int capacity;
    enum number_type_e type;
};


#define NUM2S32(number)                                                \
({                                                                     \
    int value;                                                         \
    int len = sizeof(int);                                             \
    number->get_value(number, &value, &len);                           \
    value;                                                             \
})

#define NUM2U32(number)                                                \
({                                                                     \
    unsigned int value;                                                \
    int len = sizeof(int);                                             \
    number->get_value(number, &value, &len);                           \
    value;                                                             \
})

#define MAX_BIG_NUMBER_LEN  2048

#endif
