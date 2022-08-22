/**
 * @file Number.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-08-04
 */

#include <libobject/core/Number.h>
#include <libobject/core/policy.h>

static int __deconstrcut(Number *number)
{
    allocator_t *allocator = number->parent.allocator;

    allocator_mem_free(allocator, number->big_number_data);

    return 1;
}

static int 
__set_type(Number *number, enum number_type_e type)
{
    int ret = 0;

    TRY {
        THROW_IF(type >= NUMBER_TYPE_MAX,  -1);
        THROW_IF(g_number_policies[type].set_type == NULL,  -1);
        number->type = type;
        EXEC(g_number_policies[type].set_type(number, type));
    } CATCH (ret) {
    }

    return ret;
}

static enum number_type_e __get_type(Number *number)
{
    return number->type;
}

static int __get_size(Number *number)
{
    return number->size;
}

static int 
__set_value(Number *number, void *value, int len)
{
    allocator_t *allocator = number->parent.allocator;
    int ret = 1;
    int type;

    TRY {
        type = number->get_type(number);

        if (type == NUMBER_TYPE_OBJ_BIG_NUMBER) {
            if (len < 0) {
                number->big_number_neg_flag = 1;
                len *= -1;
            }
            
            if (number->big_number_data == NULL) {
                number->big_number_data = allocator_mem_alloc(allocator, len * 2);
                THROW_IF(number->big_number_data == NULL, -1);
                number->capacity = 2 * len;
            } else if (number->capacity < len) {
                allocator_mem_free(allocator, number->big_number_data);
                number->big_number_data = allocator_mem_alloc(allocator, len * 2);
                THROW_IF(number->big_number_data == NULL, -1);
                number->capacity = 2 * len;
            }
            memcpy(number->big_number_data, value, len);
            number->size = len;
        } else {
            memcpy(&number->data, value, number->size);
            //size have been set in set_type policy.
        }
    } CATCH (ret) {
    }

    return ret;
}

static int 
__get_value(Number *number, void *value, int *len)
{
    int ret = 0;
    int type;

    TRY {
        THROW_IF(*len < number->size || value == NULL || number == NULL, -1);
        type = number->get_type(number);

        if (type == NUMBER_TYPE_OBJ_BIG_NUMBER) {
            THROW_IF(number->big_number_data == NULL, -1);
            memcpy(value, number->big_number_data, number->size);
            *len = number->size;
        } else {
            memcpy(value, &number->data, number->size);
        }
    } CATCH (ret) {
        dbg_str(DBG_DETAIL, "get_value, len:%d, size:%d", *len, number->size);
    }

    return ret;
}

static int __clear(Number *number)
{
    memset(number->big_number_data, 0, number->capacity);
    number->size = 0;
    number->type = 0;

    return 0;
}

static int __add(Number *number, enum number_type_e type, void *value, int len)
{
    enum number_type_e number_type;
    int ret = 1;

    TRY {
        THROW_IF(type >= NUMBER_TYPE_MAX,  -1);
        number_type = number->get_type(number);
        THROW_IF(g_number_policies[number_type].add == NULL,  -1);

        EXEC(g_number_policies[number_type].add(number, type, value, len));
    } CATCH (ret) {
    }

    return ret;
}

static int __sub(Number *number, enum number_type_e type, void *value, int len)
{
    enum number_type_e number_type;
    int ret = 1;

    TRY {
        THROW_IF(type >= NUMBER_TYPE_MAX,  -1);
        number_type = number->get_type(number);
        THROW_IF(g_number_policies[number_type].sub == NULL,  -1);

        EXEC(g_number_policies[number_type].sub(number, type, value, len));
    } CATCH (ret) {
    }

    return ret;
}

static int __mul(Number *number, 
                 enum number_type_e a1_type, void *a1_value, int a1_len, 
                 enum number_type_e a2_type, void *a2_value, int a2_len)
{
    enum number_type_e number_type;
    int ret = 1;

    TRY {
        number_type = number->get_type(number);
        THROW_IF(g_number_policies[number_type].mul == NULL,  -1);

        EXEC(g_number_policies[number_type].mul(number, a1_type, a1_value, a1_len, a2_type, a2_value, a2_len));
    } CATCH (ret) {
    }

    return ret;

}

static int __div(Number *number, 
                 enum number_type_e a1_type, void *a1_value, int a1_len, 
                 enum number_type_e a2_type, void *a2_value, int a2_len)
{
    enum number_type_e number_type;
    int ret = 1;

    TRY {
        number_type = number->get_type(number);
        THROW_IF(g_number_policies[number_type].div == NULL,  -1);

        EXEC(g_number_policies[number_type].div(number, a1_type, a1_value, a1_len, a2_type, a2_value, a2_len));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "div, number_type:%d", number_type);
    }

    return ret;
}

static int __mod(Number *number, enum number_type_e type, void *value, int len)
{

}

static int __exp(Number *number, enum number_type_e type, void *value, int len)
{

}

static int __gcd(Number *a, Number *b)
{

}

static int __rand(Number *number, int bits, int top, int bottom)
{

}

static int __prime(Number *number, int bits)
{

}

static class_info_entry_t number_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Number, construct, NULL),
    Init_Nfunc_Entry(2 , Number, deconstruct, __deconstrcut),
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
    Init_Vfunc_Entry(13, Number, sub, __sub),
    Init_Vfunc_Entry(14, Number, mul, __mul),
    Init_Vfunc_Entry(15, Number, div, __div),
    Init_Vfunc_Entry(16, Number, mod, __mod),
    Init_Vfunc_Entry(17, Number, gcd, __gcd),
    Init_Vfunc_Entry(18, Number, rand, __rand),
    Init_Vfunc_Entry(19, Number, prime, __prime),
    Init_End___Entry(20, Number),
};
REGISTER_CLASS("Number", number_class_info);

