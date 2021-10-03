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
        (*addr)->set_type((*addr), NUMBER_TYPE_SIGNED_INT);
    }

    (*addr)->set_value((*addr), value);

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

        v->set(v, "/Vector/init_data", value);
        v->set(v, "/Vector/class_name", entry->type_name);
        v->set(v, "/Vector/trustee_flag", &trustee_flag);
        v->reconstruct(v);
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
    if (s != NULL)
        cjson_add_string_to_object(json, name, s->value);

    return 1;
}

static int __obj_to_json_sn32_policy(cjson_t *json, char *name, void *value)
{
    double d;
    Number *number = *(Number **)value;

    if (number == NULL) {
        dbg_str(DBG_WARNNING, "Number to json, but point is null, offset:%p", value);
        return -1;
    }
    d = number->get_signed_int_value(number);
    cjson_add_number_to_object(json, name, d);

    return 1;
}

static int __obj_to_json_vector_policy(cjson_t *json, char *name, void *value)
{
    Vector *v = *((Vector **)value);
    cjson_t *item;

    if (v != NULL) {
        dbg_str(DBG_DETAIL, "Vector json: %s", v->to_json(v));
        item = cjson_parse(v->to_json(v));
        cjson_add_item_to_object(json, name, item);
    } else {
        dbg_str(DBG_WARNNING, "Vector to json, but content is null, offset:%p", value);
    }

    return 1;
}

static int __obj_to_json_object_pointer_policy(cjson_t *json, char *name, void *value)
{
    Obj *o = *(Obj **)value;
    cjson_t *item;

    if (o != NULL) {
        item = cjson_parse(o->to_json(o));
        cjson_add_item_to_object(json, o->name, item);
    }

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
