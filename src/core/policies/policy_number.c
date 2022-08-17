/**
 * @file policy.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2021-10-03
 */

#include <libobject/core/policy.h>
#include <libobject/core/utils/bn/big_number.h>

static int 
__number_signed_or_unsigned_short_set_type(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(short);

    return 0;
}

static int __number_short_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_SIGNED_SHORT, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_SIGNED_SHORT && type != NUMBER_TYPE_SIGNED_SHORT , -1);

        switch (type) {
            case NUMBER_TYPE_SIGNED_SHORT:
                number->data.short_data += *(short *)value;
                break;
            case NUMBER_TYPE_OBJ_SIGNED_SHORT:
                value = &((Number *)value)->data.short_data;
                number->data.short_data += *(short *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_unsigned_short_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_UNSIGNED_SHORT, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_UNSIGNED_SHORT && type != NUMBER_TYPE_UNSIGNED_SHORT , -1);

        switch (type) {
            case NUMBER_TYPE_UNSIGNED_SHORT:
                number->data.unsigned_short_data += *(unsigned short *)value;
                break;
            case NUMBER_TYPE_OBJ_UNSIGNED_SHORT:
                value = &((Number *)value)->data.unsigned_short_data;
                number->data.unsigned_short_data += *(unsigned short *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_signed_or_unsigned_int_set_type(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(int);

    return 0;
}

static int __number_int_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_SIGNED_INT, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_SIGNED_INT && type != NUMBER_TYPE_SIGNED_INT , -1);

        switch (type) {
            case NUMBER_TYPE_SIGNED_INT:
                number->data.int_data += *(int *)value;
                break;
            case NUMBER_TYPE_OBJ_SIGNED_INT:
                value = &((Number *)value)->data.int_data;
                number->data.int_data += *(int *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_unsigned_int_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_UNSIGNED_INT, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_UNSIGNED_INT && type != NUMBER_TYPE_UNSIGNED_INT , -1);

        switch (type) {
            case NUMBER_TYPE_UNSIGNED_INT:
                number->data.unsigned_int_data += *(unsigned int *)value;
                break;
            case NUMBER_TYPE_OBJ_UNSIGNED_INT:
                value = &((Number *)value)->data.int_data;
                number->data.unsigned_int_data += *(unsigned int *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_signed_or_unsigned_long_set_type(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(long);

    return 0;
}

static int __number_long_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_SIGNED_LONG, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_SIGNED_LONG && type != NUMBER_TYPE_SIGNED_LONG , -1);

        switch (type) {
            case NUMBER_TYPE_SIGNED_LONG:
                number->data.long_data += *(long *)value;
                break;
            case NUMBER_TYPE_OBJ_SIGNED_LONG:
                value = &((Number *)value)->data.long_data;
                number->data.long_data += *(long *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_unsigned_long_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_UNSIGNED_LONG, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_UNSIGNED_LONG && type != NUMBER_TYPE_UNSIGNED_LONG , -1);

        switch (type) {
            case NUMBER_TYPE_UNSIGNED_LONG:
                number->data.unsigned_long_data += *(unsigned long *)value;
                break;
            case NUMBER_TYPE_OBJ_UNSIGNED_LONG:
                value = &((Number *)value)->data.unsigned_long_data;
                number->data.unsigned_long_data += *(unsigned long *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_float_set_type(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(float);

    return 0;
}

static int __number_float_add(Number *number,  enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_FLOAT, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_FLOAT && type != NUMBER_TYPE_FLOAT , -1);

        switch (type) {
            case NUMBER_TYPE_FLOAT:
                number->data.float_data += *(float *)value;
                break;
            case NUMBER_TYPE_OBJ_FLOAT:
                value = &((Number *)value)->data.float_data;
                number->data.float_data += *(float *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_double_set_type(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(double);

    return 0;
}

static int __number_double_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_DOUBLE, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_DOUBLE && type != NUMBER_TYPE_DOUBLE, -1);

        switch (type) {
            case NUMBER_TYPE_DOUBLE:
                number->data.double_data += *(double *)value;
                break;
            case NUMBER_TYPE_OBJ_DOUBLE:
                value = &((Number *)value)->data.double_data;
                number->data.double_data += *(double *)value;
                break;
            default:
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_big_set_type(Number *number, enum number_type_e type)
{
    number->type = type;

    return 0;
}

static int __number_big_add(Number *number, enum number_type_e type, void *value, int len)
{
    int ret, l, i, carry = 0, tmp1, tmp2, diff;
    uint8_t *dest, *n1, *n2;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);

        dbg_str(DBG_DETAIL, "run at here, size:%d, cap:%d", number->size, number->capacity);
        switch (type) {
            case NUMBER_TYPE_OBJ_BIG_NUMBER: {
                Number *add = (Number *)value;
                if (number->big_number_neg_flag == add->big_number_neg_flag) {
                    EXEC(bn_add(number->big_number_data, number->capacity, &number->size, 
                                add->big_number_data, add->size)); 
                } else {
                    THROW(-1); //not support now!
                }
                
                break;
            }
            default:
                THROW(-1);
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}

static int __number_big_sub(Number *number, enum number_type_e type, void *value, int len)
{
    int ret, l, i, carry = 0, tmp1, tmp2, diff;
    uint8_t *dest, *n1, *n2;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);

        switch (type) {
            case NUMBER_TYPE_OBJ_BIG_NUMBER: {
                Number *sub = (Number *)value;
                if (number->big_number_neg_flag == sub->big_number_neg_flag) {
                    EXEC(bn_sub(number->big_number_data, number->capacity, &number->size, &number->big_number_neg_flag,
                                sub->big_number_data, sub->size)); 
                } else {
                    THROW(-1); //not support now!
                }
                
                break;
            }
            default:
                THROW(-1);
                break;
        }
        
    } CATCH (ret) {
    }

    return ret;
}


static int __number_big_mul(Number *number, enum number_type_e a1_type, void *a1_value, int a1_len, enum number_type_e a2_type, void *a2_value, int a2_len)
{
    int ret, l, i, carry = 0, tmp1, tmp2, diff;
    uint8_t *dest, *n1, *n2;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);
        THROW_IF(a1_type != NUMBER_TYPE_OBJ_BIG_NUMBER || a2_type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);

        dbg_str(DBG_DETAIL, "run at here, size:%d, cap:%d", number->size, number->capacity);
        if ( a1_type == NUMBER_TYPE_OBJ_BIG_NUMBER && a2_type == NUMBER_TYPE_OBJ_BIG_NUMBER) {
            Number *a1 = (Number *)a1_value;
            Number *a2 = (Number *)a2_value;
            if (a1->big_number_neg_flag == a2->big_number_neg_flag) {
                EXEC(bn_mul(number->big_number_data, number->capacity, &number->size,
                            a1->big_number_data, a1->size, 
                            a2->big_number_data, a2->size)); 
            } else {
                THROW(-1); //not support now!
            }
        }
        
    } CATCH (ret) {
    }

    return ret;
}

number_policy_t g_number_policies[NUMBER_TYPE_MAX] = {
    [NUMBER_TYPE_OBJ_SIGNED_SHORT] = {
        .set_type  = __number_signed_or_unsigned_short_set_type,
        .add       = __number_short_add,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_SHORT] = {
        .set_type  = __number_signed_or_unsigned_short_set_type,
        .add       = __number_unsigned_short_add,
    },
    [NUMBER_TYPE_OBJ_SIGNED_INT] = {
        .set_type  = __number_signed_or_unsigned_int_set_type,
        .add       = __number_int_add,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_INT] = {
        .set_type  = __number_signed_or_unsigned_int_set_type,
        .add       = __number_unsigned_int_add,
    },
    [NUMBER_TYPE_OBJ_SIGNED_LONG] = {
        .set_type  = __number_signed_or_unsigned_long_set_type,
        .add       = __number_long_add,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_LONG] = {
        .set_type  = __number_signed_or_unsigned_long_set_type,
        .add       = __number_unsigned_long_add,
    },
    [NUMBER_TYPE_OBJ_FLOAT] = {
        .set_type  = __number_float_set_type,
        .add       = __number_float_add,
    },
    [NUMBER_TYPE_OBJ_DOUBLE] = {
        .set_type  = __number_double_set_type,
        .add       = __number_double_add,
    },
    [NUMBER_TYPE_OBJ_BIG_NUMBER] = {
        .set_type  = __number_big_set_type,
        .add       = __number_big_add,
        .sub       = __number_big_sub,
        .mul       = __number_big_mul,
    },
};