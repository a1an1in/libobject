/**
 * @file Number.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-08-04
 */

#include <libobject/core/Number.h>

static int __construct(Number *number, char *init_str)
{
    return 0;
}

static int __deconstruct(Number *number)
{
    return 0;
}

static int 
__set_type(Number *number, enum number_type_e type)
{
    allocator_t *allocator;
    int size;
    int ret = 0;

    allocator = number->parent.allocator;

    number->type = type;
    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT:
        case NUMBER_TYPE_UNSIGNED_SHORT:
            size = sizeof(short);
            break;
        case NUMBER_TYPE_SIGNED_INT:
        case NUMBER_TYPE_UNSIGNED_INT:
            size = sizeof(int);
            break;
        case NUMBER_TYPE_SIGNED_LONG:
        case NUMBER_TYPE_UNSIGNED_LONG:
            size = sizeof(long);
            break;
        case NUMBER_TYPE_SIGNED_LONG_LONG:
        case NUMBER_TYPE_UNSIGNED_LONG_LONG:
            size = sizeof(long long);
            break;
        case NUMBER_TYPE_FLOAT:
            size = sizeof(float);
            break;
        case NUMBER_TYPE_DOUBLE:
            size = sizeof(double);
            break;
        default:
            ret = -1;
            goto end;
            break;
    }

    number->size = size;

end:
    return ret;
}

static enum number_type_e 
__get_type(Number *number)
{
    return number->type;
}

static int 
__get_size(Number *number)
{
    return number->size;
}

static int 
__set_value(Number *number, void *value)
{
    enum number_type_e type;
    double d;
    int ret;

    type = number->type;
    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT:
            {
                short v = *((short *)value);
                number->data.short_data = v;
                break;
            }
        case NUMBER_TYPE_UNSIGNED_SHORT:
            {
                unsigned short v = *((unsigned short *)value);
                number->data.unsigned_short_data = v;
                break;
            }
        case NUMBER_TYPE_SIGNED_INT:
            {
                int v = *((int *)value);
                number->data.int_data = v;
                break;
            }
        case NUMBER_TYPE_UNSIGNED_INT:
            {
                unsigned int v = *((unsigned int *)value);
                number->data.unsigned_int_data = v;
                break;
            }
        case NUMBER_TYPE_SIGNED_LONG:
            {
                long v = *((long *)value);
                number->data.long_data = v;
                break;
            }
        case NUMBER_TYPE_UNSIGNED_LONG:
            {
                unsigned long v = *((unsigned long *)value);
                number->data.unsigned_short_data = v;
                break;
            }
        case NUMBER_TYPE_FLOAT:
            {
                float v = *((float *)value);
                number->data.float_data = v;
                break;
            }
        case NUMBER_TYPE_DOUBLE:
            {
                double v = *((double *)value);
                number->data.float_data = v;
                break;
            }
        default:
            ret = -1;
            break;
    }

    return ret;
}

static int 
__set_format_value(Number *number, char *fmt, ...)
{
    va_list ap;
    int max_len = 100;
    char buf_addr[max_len];
    enum number_type_e type;
    int ret = 0;

    va_start(ap, fmt);
    ret = vsnprintf(buf_addr, max_len, fmt, ap);
    va_end(ap);

    type = number->type;
    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT:
        case NUMBER_TYPE_UNSIGNED_SHORT:
            break;
        case NUMBER_TYPE_SIGNED_INT:
            {
                number->data.int_data = strtol(buf_addr, NULL, 10);;
                break;
            }
        case NUMBER_TYPE_UNSIGNED_INT:
            {
                number->data.int_data = strtoul(buf_addr, NULL, 10);;
                break;
            }
        case NUMBER_TYPE_SIGNED_LONG:
            {
                number->data.int_data = strtol(buf_addr, NULL, 10);;
                break;
            }
        case NUMBER_TYPE_UNSIGNED_LONG:
            {
                number->data.int_data = strtoul(buf_addr, NULL, 10);;
                break;
            }
        case NUMBER_TYPE_FLOAT:
            {
                number->data.float_data = strtof(buf_addr, NULL);
                break;
            }
        case NUMBER_TYPE_DOUBLE:
            {
                number->data.double_data = strtod(buf_addr, NULL);
                break;
            }
        default:
            ret = -1;
            break;
    }

    return ret;
}

static short 
__get_signed_short_value(Number *number)
{
    return number->data.short_data;
}

static unsigned short 
__get_unsigned_short_value(Number *number)
{
    return number->data.unsigned_short_data;
}

static int 
__get_signed_int_value(Number *number)
{
    return number->data.int_data;
}

static unsigned int 
__get_unsigned_int_value(Number *number)
{
    return number->data.unsigned_int_data;
}

static long 
__get_signed_long_value(Number *number)
{
    return number->data.long_data;
}

static unsigned long 
__get_unsigned_long_value(Number *number)
{
    return number->data.unsigned_long_data;
}

static float 
__get_float_value(Number *number)
{
    return number->data.float_data;
}

static double 
__get_double_value(Number *number)
{
    return number->data.double_data;
}

static int __clear(Number *number)
{
    number->size = 0;
    number->type = 0;
}

static class_info_entry_t number_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Number, construct, __construct),
    Init_Nfunc_Entry(2 , Number, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Number, set, NULL),
    Init_Vfunc_Entry(4 , Number, get, NULL),
    Init_Vfunc_Entry(5 , Number, to_json, NULL),
    Init_Vfunc_Entry(6 , Number, set_type, __set_type),
    Init_Vfunc_Entry(7 , Number, get_type, __get_type),
    Init_Vfunc_Entry(8 , Number, get_size, __get_size),
    Init_Vfunc_Entry(9 , Number, set_value, __set_value),
    Init_Vfunc_Entry(10, Number, set_format_value, __set_format_value),
    Init_Vfunc_Entry(11, Number, get_signed_short_value, __get_signed_short_value),
    Init_Vfunc_Entry(12, Number, get_unsigned_short_value, __get_unsigned_short_value),
    Init_Vfunc_Entry(13, Number, get_signed_int_value, __get_signed_int_value),
    Init_Vfunc_Entry(14, Number, get_unsigned_int_value, __get_unsigned_int_value),
    Init_Vfunc_Entry(15, Number, get_signed_long_value, __get_signed_long_value),
    Init_Vfunc_Entry(16, Number, get_unsigned_long_value, __get_unsigned_long_value),
    Init_Vfunc_Entry(17, Number, get_float_value, __get_float_value),
    Init_Vfunc_Entry(18, Number, get_double_value, __get_double_value),
    Init_Vfunc_Entry(19, Number, clear, __clear),
    Init_End___Entry(20, Number),
};
REGISTER_CLASS("Number", number_class_info);

