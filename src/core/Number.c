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
    return 1;
}

static int __deconstruct(Number *number)
{
    return 1;
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
__set_value(Number *number, enum number_type_e type, void *value)
{
    int ret = 1;

    number->set_type(number, type);

    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT: {
            short v = *((short *)value);
            number->data.short_data = v;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_SHORT: {
            unsigned short v = *((unsigned short *)value);
            number->data.unsigned_short_data = v;
            break;
        }
        case NUMBER_TYPE_SIGNED_INT: {
            int v = *((int *)value);
            number->data.int_data = v;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_INT: {
            unsigned int v = *((unsigned int *)value);
            number->data.unsigned_int_data = v;
            break;
        }
        case NUMBER_TYPE_SIGNED_LONG: {
            long v = *((long *)value);
            number->data.long_data = v;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_LONG: {
            unsigned long v = *((unsigned long *)value);
            number->data.unsigned_short_data = v;
            break;
        }
        case NUMBER_TYPE_FLOAT: {
            float v = *((float *)value);
            number->data.float_data = v;
            break;
        }
        case NUMBER_TYPE_DOUBLE: {
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
__get_value(Number *number, enum number_type_e type, void *value)
{
    int ret = 1;

    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT: {
            *((short *)value) = number->data.short_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_SHORT: {
            *((unsigned short *)value) = number->data.unsigned_short_data;
            break;
        }
        case NUMBER_TYPE_SIGNED_INT: {
            *((int *)value) = number->data.int_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_INT: {
            *((unsigned int *)value) = number->data.unsigned_int_data;
            break;
        }
        case NUMBER_TYPE_SIGNED_LONG: {
            *((long *)value) = number->data.long_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_LONG: {
            *((unsigned long *)value) = number->data.unsigned_short_data;
            break;
        }
        case NUMBER_TYPE_FLOAT: {
            *((float *)value) = number->data.float_data;
            break;
        }
        case NUMBER_TYPE_DOUBLE: {
            *((double *)value) = number->data.float_data;
            break;
        }
        default:
            ret = -1;
            break;
    }

    return ret;
}

static int __clear(Number *number)
{
    number->size = 0;
    number->type = 0;

    return 0;
}

static int __add(Number *number, Number *add)
{
    enum number_type_e type;
    int ret = 1;

    type = number->get_type(number);

    switch(type) {
        case NUMBER_TYPE_SIGNED_SHORT: {
            number->data.short_data += add->data.short_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_SHORT: {
            number->data.unsigned_short_data += add->data.unsigned_short_data;
            break;
        }
        case NUMBER_TYPE_SIGNED_INT: {
            number->data.int_data += add->data.int_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_INT: {
            number->data.unsigned_int_data += add->data.unsigned_int_data;
            break;
        }
        case NUMBER_TYPE_SIGNED_LONG: {
            number->data.long_data += add->data.long_data;
            break;
        }
        case NUMBER_TYPE_UNSIGNED_LONG: {
            number->data.unsigned_short_data += add->data.unsigned_short_data;
            break;
        }
        case NUMBER_TYPE_FLOAT: {
            number->data.float_data += add->data.float_data;
            break;
        }
        case NUMBER_TYPE_DOUBLE: {
            number->data.double_data += add->data.double_data;
            break;
        }
        default:
            ret = -1;
            break;
    }

    return ret;
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
    Init_Vfunc_Entry(10, Number, get_value, __get_value),
    Init_Vfunc_Entry(11, Number, clear, __clear),
    Init_Vfunc_Entry(12, Number, add, __add),
    Init_End___Entry(13, Number),
};
REGISTER_CLASS("Number", number_class_info);

