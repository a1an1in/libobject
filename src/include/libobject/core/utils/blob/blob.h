#ifndef __BLOB_H__
#define __BLOB_H__

#include <libobject/basic_types.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/dbg/debug.h>

#define blob_for_each_attr(pos, head, len) \
    for(pos = head; (uint8_t *)pos < (uint8_t *)head + len; pos = blob_next((blob_attr_t *)pos))

#define BLOB_MAX_SIZE 1024 * 10
#define BLOB_BUFFER_MAX_SIZE 2048 * 2

typedef struct blob_s{
    allocator_t *allocator;
    uint8_t *head;
    uint8_t *tail;
    uint16_t len;
    array_stack_t *tbl_stack;
}blob_t;

typedef struct blob_attr_s{
    uint8_t type;
    uint16_t len;
    uint16_t name_len;
    uint8_t value[0];
}__attribute__((packed)) blob_attr_t;

enum msgblob_type {       
    BLOB_TYPE_UNSPEC,  
    BLOB_TYPE_ARRAY,
    BLOB_TYPE_TABLE,
    BLOB_TYPE_STRING,
    BLOB_TYPE_BUFFER,
    BLOB_TYPE_UINT64,
    BLOB_TYPE_UINT32,
    BLOB_TYPE_UINT16,
    BLOB_TYPE_UINT8,
    BLOB_TYPE_INT32,
    BLOB_TYPE_LAST,
}; 

typedef struct blob_policy_s{
    uint8_t type;
    char *name;
}blob_policy_t;

blob_t *blob_create(allocator_t *allocator);
int blob_init(blob_t *blob);
int blob_reset(blob_t *blob);
int blob_destroy(blob_t *blob);
int blob_add(blob_t *blob, uint8_t type, char *name, void *value, uint16_t value_len);
int blob_add_uint8(blob_t *blob, char *name, uint8_t val);
int blob_add_uint16(blob_t *blob, char *name, uint16_t val);
int blob_add_uint32(blob_t *blob, char *name, uint32_t val);
int blob_add_int32(blob_t *blob, char *name, int32_t val);
/*
 *int blob_add_uint64(blob_t *blob, char *name, uint64_t val);
 */
int blob_add_string(blob_t *blob, char *name, char *str);
int blob_add_buffer(blob_t *blob, char *name, uint8_t *buf, int len);
int blob_add_table_start(blob_t *blob, char *name);
int blob_add_table_end(blob_t *blob);
int blob_catenate(blob_t *blob, blob_attr_t *attr);
blob_attr_t *blob_next(blob_attr_t *attr);
char *blob_get_name(blob_attr_t *attr);
uint8_t *blob_get_data(blob_attr_t *attr);
uint32_t blob_get_data_len(blob_attr_t *attr);
uint8_t blob_get_uint8(blob_attr_t *attr);
uint16_t blob_get_uint16(blob_attr_t *attr);
uint32_t blob_get_uint32(blob_attr_t *attr);
uint64_t blob_get_uint64(blob_attr_t *attr);
int32_t blob_get_int32(blob_attr_t *attr);
int blob_get_buffer(blob_attr_t *attr,uint8_t **out);
char * blob_get_string(blob_attr_t *attr);
uint16_t blob_get_len(blob_attr_t *attr);

int blob_parse_to_attr(const struct blob_policy_s *policy,                               
                       uint8_t policy_count,
                       blob_attr_t **tb,
                       void *data,
                       unsigned int len);



#endif
