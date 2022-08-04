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

        if (type == NUMBER_TYPE_BIG_NUMBER) {
            if (number->big_number_data == NULL) {
                number->big_number_data = allocator_mem_alloc(allocator, len);
                THROW_IF(number->big_number_data == NULL, -1);
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
        } else {
            memcpy(value, &number->data, number->size);
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __clear(Number *number)
{
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

static class_info_entry_t number_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Number, construct, NULL),
    Init_Nfunc_Entry(2 , Number, deconstruct, NULL),
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

