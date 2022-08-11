/**
 * @file policy.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2021-10-03
 */

#include <libobject/core/policy.h>
#include <libobject/core/utils/bn/big_number.h>

int __obj_set_int8_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int8_t t = (int8_t)(*((int *)value));
    int8_t *addr = (int8_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_uint8_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint8_t t = (uint8_t)(*((int *)value));
    uint8_t *addr = (uint8_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_int16_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int16_t t = (int16_t)(*((int *)value));
    int16_t *addr = (int16_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_uint16_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint16_t t = (uint16_t)(*((int *)value));
    uint16_t *addr = (uint16_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_int32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    int32_t t = (int32_t)(*((int *)value));
    int32_t *addr = (int32_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_uint32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    uint32_t t = (uint32_t)(*((int *)value));
    uint32_t *addr = (uint32_t *)(base + entry->offset);
    *addr = t;

    return 1;
}

int __obj_set_float_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    double t1 = *((double *)value);
    float t2 = (double)t1;
    float *addr = (float *)(base + entry->offset);
    *addr = t2;

    return 1;
}

int __obj_set_double_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    float *addr = (float *)(base + entry->offset);
    *addr = *((double *)value);

    return 1;
}

int __obj_set_sn32_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    Number **addr = (Number **)(base + entry->offset);

    if (*addr == NULL) {
        *addr = object_new(allocator, "Number", NULL);
        (*addr)->set_type((*addr), NUMBER_TYPE_OBJ_SIGNED_INT);
    }

    (*addr)->set_value((*addr), value, -1);

    return 1;
}

int __obj_set_string_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    String **addr = (String **)(base + entry->offset);

    if (*addr != NULL)
        (*addr)->assign((*addr), (char *)value);
    else {
        *addr = object_new(allocator, "String", NULL);
        (*addr)->assign((*addr), (char *)value);
    }

    return 1;
}

int __obj_set_vector_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    Vector **addr = (Vector **)(base + entry->offset);
    Vector *v;
    uint8_t trustee_flag = 1;
    char *p = (char *)value;
    int ret = 1;

    TRY {
        if (*addr != NULL) {
            v = *addr;
        } else {
            v = object_new(allocator, "Vector", NULL);
        }
        THROW_IF(p[0] != '[', -1);
        THROW_IF(p[1] == ']', 0);

        if (entry->type_name != NULL) {
            v->set(v, "/Vector/class_name", entry->type_name);
        }
        v->set(v, "/Vector/trustee_flag", &trustee_flag);
        v->assign(v, value);
        *addr = v;
        Vector **addr = (Vector **)(base + entry->offset);
    } CATCH (ret) {
    }

    return ret;
}

int __obj_set_pointer_policy(Obj *obj, class_info_entry_t *entry, void *value)
{
    uint8_t *base = (uint8_t *)obj;
    void **addr = (void **)(base + entry->offset);
    *addr = value;

    return 1;
}

obj_set_policy_t g_obj_set_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]         = {.policy = __obj_set_int8_policy},
    [ENTRY_TYPE_UINT8_T]        = {.policy = __obj_set_uint8_policy},
    [ENTRY_TYPE_INT16_T]        = {.policy = __obj_set_int16_policy},
    [ENTRY_TYPE_UINT16_T]       = {.policy = __obj_set_uint16_policy},
    [ENTRY_TYPE_INT32_T]        = {.policy = __obj_set_int32_policy},
    [ENTRY_TYPE_UINT32_T]       = {.policy = __obj_set_uint32_policy},
    [ENTRY_TYPE_FLOAT_T]        = {.policy = __obj_set_float_policy},
    [ENTRY_TYPE_DOUBLE_T]       = {.policy = __obj_set_double_policy},
    [ENTRY_TYPE_SN32]           = {.policy = __obj_set_sn32_policy},
    [ENTRY_TYPE_STRING]         = {.policy = __obj_set_string_policy},
    [ENTRY_TYPE_VECTOR]         = {.policy = __obj_set_vector_policy},
    [ENTRY_TYPE_NORMAL_POINTER] = {.policy = __obj_set_pointer_policy},
    [ENTRY_TYPE_FUNC_POINTER]   = {.policy = __obj_set_pointer_policy},
    [ENTRY_TYPE_VFUNC_POINTER]  = {.policy = __obj_set_pointer_policy},
    [ENTRY_TYPE_IFUNC_POINTER]  = {.policy = __obj_set_pointer_policy},
    [ENTRY_TYPE_OBJ_POINTER]    = {.policy = __obj_set_pointer_policy},
};

static int __obj_to_json_int8_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int8_t *)value));

    return 1;
}

static int __obj_to_json_uint8_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint8_t *)value));

    return 1;
}

static int __obj_to_json_int16_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int16_t *)value));

    return 1;
}

static int __obj_to_json_uint16_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint16_t *)value));

    return 1;
}

static int __obj_to_json_int32_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((int32_t *)value));

    return 1;
}

static int __obj_to_json_uint32_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((uint32_t *)value));

    return 1;
}

static int __obj_to_json_float_policy(cjson_t *json, char *name, void *value)
{
    cjson_add_number_to_object(json, name, *((float *)value));

    return 1;
}

static int __obj_to_json_string_policy(cjson_t *json, char *name, void *value)
{
    String *s = *(String **)value;

    if (s == NULL) {
        return 0;
    }

    cjson_add_string_to_object(json, name, s->value);

    return 1;
}

static int __obj_to_json_sn32_policy(cjson_t *json, char *name, void *value)
{
    int d, len;
    Number *number = *(Number **)value;

    if (number == NULL) {
        return 0;
    }
    len = sizeof(d);
    number->get_value(number, &d, &len);

    cjson_add_number_to_object(json, name, d);

    return 1;
}

static int __obj_to_json_vector_policy(cjson_t *json, char *name, void *value)
{
    Vector *v = *((Vector **)value);
    cjson_t *item;

    if (v == NULL) {
        return 0;
    }
    dbg_str(DBG_DETAIL, "Vector json: %s", v->to_json(v));
    item = cjson_parse(v->to_json(v));
    cjson_add_item_to_object(json, name, item);

    return 1;
}

static int __obj_to_json_object_pointer_policy(cjson_t *json, char *name, void *value)
{
    Obj *o = *(Obj **)value;
    cjson_t *item;

    if (o == NULL) {
        return 0;
    }

    item = cjson_parse(o->to_json(o));
    cjson_add_item_to_object(json, name, item);

    return 1;
}

obj_to_json_policy_t g_obj_to_json_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]      = {.policy = __obj_to_json_int8_policy},
    [ENTRY_TYPE_UINT8_T]     = {.policy = __obj_to_json_uint8_policy},
    [ENTRY_TYPE_INT16_T]     = {.policy = __obj_to_json_int16_policy},
    [ENTRY_TYPE_UINT16_T]    = {.policy = __obj_to_json_uint16_policy},
    [ENTRY_TYPE_INT32_T]     = {.policy = __obj_to_json_int32_policy},
    [ENTRY_TYPE_UINT32_T]    = {.policy = __obj_to_json_uint32_policy},
    [ENTRY_TYPE_FLOAT_T]     = {.policy = __obj_to_json_float_policy},
    [ENTRY_TYPE_STRING]      = {.policy = __obj_to_json_string_policy},
    [ENTRY_TYPE_SN32]        = {.policy = __obj_to_json_sn32_policy},
    [ENTRY_TYPE_VECTOR]      = {.policy = __obj_to_json_vector_policy},
    [ENTRY_TYPE_OBJ_POINTER] = {.policy = __obj_to_json_object_pointer_policy},
};


static int __vector_to_json_int8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int8_t num = (int8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint8_t num = (uint8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_int16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int16_t num = (int16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint16_t num = (uint16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_int32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int32_t num = (int32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_uint32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint32_t num = (uint32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_float_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    float *num = (float *)element;

    item = cjson_create_number(*num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_string_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    String *s = (String *)element;

    item = cjson_create_string(s->get_cstr(s));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __vector_to_json_object_pointer_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    Obj *o = (Obj *)element;

    item = cjson_parse(o->to_json(o));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

vector_to_json_policy_t g_vector_to_json_policy[ENTRY_TYPE_MAX_TYPE] = {
    [ENTRY_TYPE_INT8_T]      = {.policy = __vector_to_json_int8_policy},
    [ENTRY_TYPE_UINT8_T]     = {.policy = __vector_to_json_uint8_policy},
    [ENTRY_TYPE_INT16_T]     = {.policy = __vector_to_json_int16_policy},
    [ENTRY_TYPE_UINT16_T]    = {.policy = __vector_to_json_uint16_policy},
    [ENTRY_TYPE_INT32_T]     = {.policy = __vector_to_json_int32_policy},
    [ENTRY_TYPE_UINT32_T]    = {.policy = __vector_to_json_uint32_policy},
    [VALUE_TYPE_FLOAT_T]     = {.policy = __vector_to_json_float_policy},
    [VALUE_TYPE_STRING]      = {.policy = __vector_to_json_string_policy},
    [VALUE_TYPE_OBJ_POINTER] = {.policy = __vector_to_json_object_pointer_policy},
};

static int 
__number_set_type_of_signed_or_unsigned_short(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(short);

    return 0;
}

static int 
__number_set_type_of_signed_or_unsigned_int(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(int);

    return 0;
}

static int 
__number_set_type_of_signed_or_unsigned_long(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(long);

    return 0;
}

static int 
__number_set_type_of_float(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(float);

    return 0;
}

static int 
__number_set_type_of_double(Number *number, enum number_type_e type)
{
    number->type = type;
    number->size = sizeof(double);

    return 0;
}

static int 
__number_set_type_of_big_number(Number *number, enum number_type_e type)
{
    number->type = type;

    return 0;
}

static int __number_add_to_short(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_unsigned_short(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_int(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_unsigned_int(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_long(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_unsigned_long(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_float(Number *number,  enum number_type_e type, void *value, int len)
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

static int __number_add_to_double(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_add_to_big_number(Number *number, enum number_type_e type, void *value, int len)
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

static int __number_sub_to_big_number(Number *number, enum number_type_e type, void *value, int len)
{
    int ret, l, i, carry = 0, tmp1, tmp2, diff;
    uint8_t *dest, *n1, *n2;

    TRY {
        THROW_IF(number->type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);
        THROW_IF(type != NUMBER_TYPE_OBJ_BIG_NUMBER, -1);

        dbg_str(DBG_DETAIL, "run at here, size:%d, cap:%d", number->size, number->capacity);
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


number_policy_t g_number_policies[NUMBER_TYPE_MAX] = {
    [NUMBER_TYPE_OBJ_SIGNED_SHORT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_short,
        .add       = __number_add_to_short,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_SHORT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_short,
        .add       = __number_add_to_unsigned_short,
    },
    [NUMBER_TYPE_OBJ_SIGNED_INT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_int,
        .add       = __number_add_to_int,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_INT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_int,
        .add       = __number_add_to_unsigned_int,
    },
    [NUMBER_TYPE_OBJ_SIGNED_LONG] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_long,
        .add       = __number_add_to_long,
    },
    [NUMBER_TYPE_OBJ_UNSIGNED_LONG] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_long,
        .add       = __number_add_to_unsigned_long,
    },
    [NUMBER_TYPE_OBJ_FLOAT] = {
        .set_type  = __number_set_type_of_float,
        .add       = __number_add_to_float,
    },
    [NUMBER_TYPE_OBJ_DOUBLE] = {
        .set_type  = __number_set_type_of_double,
        .add       = __number_add_to_double,
    },
    [NUMBER_TYPE_OBJ_BIG_NUMBER] = {
        .set_type  = __number_set_type_of_big_number,
        .add       = __number_add_to_big_number,
        .sub       = __number_sub_to_big_number,
    },
};