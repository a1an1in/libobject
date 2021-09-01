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
    int (*set_value)(Number *number, void *value);
    int (*set_format_value)(Number *number, const char *fmt, ...);
    short (*get_signed_short_value)(Number *number);
    unsigned short (*get_unsigned_short_value)(Number *number);
    int (*get_signed_int_value)(Number *number);
    unsigned int (*get_unsigned_int_value)(Number *number);
    long (*get_signed_long_value)(Number *number);
    unsigned long (*get_unsigned_long_value)(Number *number);
    long long (*get_signed_long_long_value)(Number *number);
    unsigned long long (*get_unsigned_long_long_value)(Number *number);
    float (*get_float_value)(Number *number);
    double (*get_double_value)(Number *number);
    int (*clear)(Number *number);

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
    int size;
    enum number_type_e type;
};

#define NUM2S32(number) number->get_signed_int_value(number)  
#define NUM2U32(number) number->get_unsigned_int_value(number)  

#endif
