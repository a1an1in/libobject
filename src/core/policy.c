/**
 * @file policy.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2021-10-03
 */

#include <libobject/core/policy.h>

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
    }

    (*addr)->set_value((*addr), NUMBER_TYPE_SIGNED_INT, value);

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
    int d;
    Number *number = *(Number **)value;

    if (number == NULL) {
        return 0;
    }
    number->get_value(number, NUMBER_TYPE_SIGNED_INT, &d);

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
__number_set_value_of_signed_or_unsigned_short(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_SHORT) {
        short v = *((short *)value);
        number->data.short_data = v;
    } else if (type = NUMBER_TYPE_UNSIGNED_SHORT) {
        unsigned short v = *((unsigned short *)value);
        number->data.unsigned_short_data = v;
    } else {
        return -1;
    }
    
    return 0;
}

static int 
__number_set_value_of_signed_or_unsigned_int(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_INT) {
        int v = *((int *)value);
        number->data.int_data = v;
    } else if (type = NUMBER_TYPE_UNSIGNED_INT) {
        unsigned int v = *((unsigned int *)value);
        number->data.unsigned_int_data = v;
    } else {
        return -1;
    }
    
    return 0;
}

static int 
__number_set_value_of_signed_or_unsigned_long(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_LONG) {
        long v = *((int *)value);
        number->data.long_data = v;
    } else if (type = NUMBER_TYPE_UNSIGNED_LONG) {
        unsigned long v = *((unsigned long *)value);
        number->data.unsigned_long_data = v;
    } else {
        return -1;
    }
    
    return 0;
}

static int 
__number_set_value_of_float(Number *number, enum number_type_e type, void *value)
{
    float v = *((float *)value);
    number->data.float_data = v;
    
    return 0;
}

static int 
__number_set_value_of_double(Number *number, enum number_type_e type, void *value)
{
    double v = *((double *)value);
    number->data.double_data = v;
    
    return 0;
}


static int 
__number_get_value_of_signed_or_unsigned_short(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_SHORT) {
        *((short *)value) = number->data.short_data;
    } else if (type = NUMBER_TYPE_UNSIGNED_SHORT) {
        *((unsigned short *)value) = number->data.unsigned_short_data;
    } else {
        return -1;
    }
    
    return 0;
}


static int 
__number_get_value_of_signed_or_unsigned_int(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_INT) {
        *((int *)value) = number->data.int_data;
    } else if (type = NUMBER_TYPE_UNSIGNED_INT) {
        *((unsigned int *)value) = number->data.unsigned_int_data;
    } else {
        return -1;
    }
    
    return 0;
}

static int 
__number_get_value_of_signed_or_unsigned_long(Number *number, enum number_type_e type, void *value)
{
    if (type == NUMBER_TYPE_SIGNED_LONG) {
        *((long *)value) = number->data.long_data;
    } else if (type = NUMBER_TYPE_UNSIGNED_LONG) {
        *((unsigned long *)value) = number->data.unsigned_long_data;
    } else {
        return -1;
    }
    
    return 0;
}

static int 
__number_get_value_of_float(Number *number, enum number_type_e type, void *value)
{
    *((float *)value) = number->data.float_data;
    
    return 0;
}

static int 
__number_get_value_of_double(Number *number, enum number_type_e type, void *value)
{
    *((double *)value) = number->data.double_data;
    
    return 0;
}

static int __number_add_to_short(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.short_data += add->data.short_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_unsigned_short(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.unsigned_short_data += add->data.unsigned_short_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_int(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.int_data += add->data.int_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_unsigned_int(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.unsigned_int_data += add->data.unsigned_int_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_long(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.long_data += add->data.long_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_unsigned_long(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.unsigned_long_data += add->data.unsigned_long_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_float(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.float_data += add->data.float_data;
    } CATCH (ret) {
    }

    return ret;
}

static int __number_add_to_double(Number *number, Number *add)
{
    int ret;

    TRY {
        THROW_IF(number->type != add->type, -1);
        number->data.double_data += add->data.double_data;
    } CATCH (ret) {
    }

    return ret;
}

number_policy_t g_number_policies[NUMBER_TYPE_MAX] = {
    [NUMBER_TYPE_SIGNED_SHORT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_short,
        .set_value = __number_set_value_of_signed_or_unsigned_short,
        .get_value = __number_get_value_of_signed_or_unsigned_short,
        .add       = __number_add_to_short,
    },
    [NUMBER_TYPE_UNSIGNED_SHORT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_short,
        .set_value = __number_set_value_of_signed_or_unsigned_short,
        .get_value = __number_get_value_of_signed_or_unsigned_short,
        .add       = __number_add_to_unsigned_short,
    },
    [NUMBER_TYPE_SIGNED_INT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_int,
        .set_value = __number_set_value_of_signed_or_unsigned_int,
        .get_value = __number_get_value_of_signed_or_unsigned_int,
        .add       = __number_add_to_int,
    },
    [NUMBER_TYPE_UNSIGNED_INT] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_int,
        .set_value = __number_set_value_of_signed_or_unsigned_int,
        .get_value = __number_get_value_of_signed_or_unsigned_int,
        .add       = __number_add_to_unsigned_int,
    },
    [NUMBER_TYPE_SIGNED_LONG] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_long,
        .set_value = __number_set_value_of_signed_or_unsigned_long,
        .get_value = __number_get_value_of_signed_or_unsigned_long,
        .add       = __number_add_to_long,
    },
    [NUMBER_TYPE_UNSIGNED_LONG] = {
        .set_type  = __number_set_type_of_signed_or_unsigned_long,
        .set_value = __number_set_value_of_signed_or_unsigned_long,
        .get_value = __number_get_value_of_signed_or_unsigned_long,
        .add       = __number_add_to_unsigned_long,
    },
    [NUMBER_TYPE_FLOAT] = {
        .set_type  = __number_set_type_of_float,
        .set_value = __number_set_value_of_float,
        .get_value = __number_get_value_of_float,
        .add       = __number_add_to_float,
    },
    [NUMBER_TYPE_DOUBLE] = {
        .set_type  = __number_set_type_of_double,
        .set_value = __number_set_value_of_double,
        .get_value = __number_get_value_of_double,
        .add       = __number_add_to_double,
    }
};