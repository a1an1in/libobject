/**
 * @file policy.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2021-10-03
 */

#include <libobject/core/policy.h>

static int __to_json_int8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int8_t num = (int8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_uint8_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint8_t num = (uint8_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_int16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int16_t num = (int16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_uint16_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint16_t num = (uint16_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_int32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    int32_t num = (int32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_uint32_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    uint32_t num = (uint32_t)element;

    item = cjson_create_number(num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_float_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    float *num = (float *)element;

    item = cjson_create_number(*num);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_string_policy(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    String *s = (String *)element;

    item = cjson_create_string(s->get_cstr(s));
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int __to_json_object_pointer_policy(cjson_t *root, void *element)
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
    [ENTRY_TYPE_INT8_T]      = {.policy = __to_json_int8_policy},
    [ENTRY_TYPE_UINT8_T]     = {.policy = __to_json_uint8_policy},
    [ENTRY_TYPE_INT16_T]     = {.policy = __to_json_int16_policy},
    [ENTRY_TYPE_UINT16_T]    = {.policy = __to_json_uint16_policy},
    [ENTRY_TYPE_INT32_T]     = {.policy = __to_json_int32_policy},
    [ENTRY_TYPE_UINT32_T]    = {.policy = __to_json_uint32_policy},
    [VALUE_TYPE_FLOAT_T]     = {.policy = __to_json_float_policy},
    [VALUE_TYPE_STRING]      = {.policy = __to_json_string_policy},
    [VALUE_TYPE_OBJ_POINTER] = {.policy = __to_json_object_pointer_policy},
};
