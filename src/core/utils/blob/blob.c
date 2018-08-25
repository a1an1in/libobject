/**
 * @file blob.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-09
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/blob/utils.h>
#include <libobject/core/utils/blob/blob.h>

blob_t *blob_create(allocator_t *allocator)
{
    blob_t *b;

    if((b = (blob_t *)allocator_mem_alloc(allocator, sizeof(blob_t))) == NULL){
        dbg_str(DBG_DETAIL, "allocator_mem_alloc");
        return NULL;
    }

    memset(b, 0, sizeof(blob_t));

    b->allocator = allocator;

    return b;
}

int blob_init(blob_t *blob)
{
    allocator_t *allocator = blob->allocator;

    if(blob->len == 0) {
        blob->len = 512;
        blob->head = (uint8_t *) allocator_mem_alloc(allocator, blob->len);
        if(blob->head == NULL) {
            dbg_str(DBG_WARNNING, "allocator_mem_alloc");
            return -1;
        }
        memset(blob->head, 0 , blob->len);
        blob->tail = blob->head;
    }

    blob->tbl_stack = array_stack_alloc(allocator);
    blob->tbl_stack->step = sizeof(void *);
    array_stack_init(blob->tbl_stack);

    return 0;
}

int blob_reset(blob_t *blob)
{
    memset(blob->head, 0 , blob->len);
    blob->tail = blob->head;

    return 0;
}

int blob_destroy(blob_t *blob)
{
    dbg_str(DBG_DETAIL, "blob_destroy");
    array_stack_destroy(blob->tbl_stack);
    allocator_mem_free(blob->allocator, blob->head);
    allocator_mem_free(blob->allocator, blob);

    return 0;
}

int blob_add(blob_t *blob, uint8_t type, char *name, void *value, uint16_t value_len)
{
    blob_attr_t *new_attrib = (blob_attr_t *)blob->tail;
    uint16_t name_len, attr_len;

    new_attrib->type = type;
    name_len = strlen(name) + 1;
    new_attrib->name_len = cpu_to_be16(name_len);

    if(name != NULL){
        strncpy((char *)(new_attrib->value), (char *) name, name_len);
        /*
         *dbg_buf(DBG_DETAIL, "add name:", (char *)(new_attrib->value), name_len);
         */
    }

    if(value != NULL)
        memcpy(new_attrib->value + name_len, value, value_len);


    attr_len = sizeof(new_attrib->type) + sizeof(new_attrib->len) +
                      sizeof(new_attrib->name_len) + name_len + value_len;
    new_attrib->len = cpu_to_be16(attr_len);

    blob->tail += attr_len;

    return attr_len;
}

int blob_add_u8(blob_t *blob, char *name, uint8_t val)
{
    return blob_add(blob, BLOB_TYPE_INT8, name, &val, sizeof(val));
}

int blob_add_u16(blob_t *blob, char *name, uint16_t val)
{
    val = cpu_to_be16(val);
    return blob_add(blob, BLOB_TYPE_INT16, name, &val, sizeof(val));
}

int blob_add_u32(blob_t *blob, char *name, uint32_t val)
{
    val = cpu_to_be32(val);
    return blob_add(blob, BLOB_TYPE_INT32, name, &val, sizeof(val));
}

int blob_add_u64(blob_t *blob, char *name, uint64_t val)
{
    val = cpu_to_be64(val);
    return blob_add(blob, BLOB_TYPE_INT64, name, &val, sizeof(val));
}

int blob_add_string(blob_t *blob, char *name, char *str)
{
    return blob_add(blob, BLOB_TYPE_STRING, name, str, strlen(str) + 1);
}

int blob_add_buffer(blob_t *blob, char *name, uint8_t *buf, int len)
{
    return blob_add(blob, BLOB_TYPE_BUFFER, name, buf, len);
}

int blob_add_table_start(blob_t *blob, char *name)
{
    blob_attr_t *new_table = (blob_attr_t *)blob->tail;

    dbg_str(DBG_DETAIL, "blob_add_table_start, push addr:%p", new_table);
    /*
     *array_stack_push(blob->tbl_stack, &new_table);
     */
    array_stack_push(blob->tbl_stack, new_table);

    return blob_add(blob, BLOB_TYPE_TABLE, name, NULL, 0);
}

int blob_add_table_end(blob_t *blob)
{
    uint8_t *table_start = NULL;
    uint16_t table_len;

    array_stack_pop(blob->tbl_stack, (void **)&table_start);

    dbg_str(DBG_DETAIL, "blob_add_table_end, pop addr:%p", table_start);

    table_len = blob->tail - table_start;

    ((blob_attr_t *)table_start)->len = cpu_to_be16(table_len);

    return 0;
}

int blob_catenate(blob_t *blob, blob_attr_t *attr)
{
    uint8_t *joint = (uint8_t *)blob->tail;

    memcpy(joint, attr, blob_get_len(attr));

    blob->tail += blob_get_len(attr);

    return 0;
}
blob_attr_t *blob_next(blob_attr_t *attr)
{
    blob_attr_t *ret =(blob_attr_t *)((uint8_t *)attr + be16_to_cpu(attr->len));

    return ret;
}

char *blob_get_name(blob_attr_t *attr)
{
    return (char *)(attr->value);
}

uint8_t *blob_get_data(blob_attr_t *attr)
{
    return attr->value + be16_to_cpu(attr->name_len); 
}

uint16_t blob_get_len(blob_attr_t *attr)
{
    return be16_to_cpu(attr->len); 
}

uint32_t blob_get_data_len(blob_attr_t *attr)
{
    uint8_t * data_addr = blob_get_data(attr);
    return be16_to_cpu(attr->len) - (uint8_t)(data_addr - (uint8_t *)attr); 
}

uint8_t blob_get_u8(blob_attr_t *attr)
{
    uint8_t *body_addr = blob_get_data(attr);

    return body_addr[0];
}

uint16_t blob_get_u16(blob_attr_t *attr)
{
    uint16_t *body_addr = (uint16_t*)blob_get_data(attr);

    return be16_to_cpu(*body_addr);
}

uint32_t blob_get_u32(blob_attr_t *attr)
{
    uint32_t *body_addr = (uint32_t*)blob_get_data(attr);

    return be32_to_cpu(*body_addr);
}

char * blob_get_string(blob_attr_t *attr)
{
    char *body_addr = (char *) blob_get_data(attr);

    return body_addr;
}

int blob_get_buffer(blob_attr_t *attr, uint8_t **out)
{
    /*
     *memcpy(out, blob_get_data(attr), blob_get_data_len(attr));
     */
    *out = blob_get_data(attr);

    return blob_get_data_len(attr);
}

int blob_parse_to_attr(const struct blob_policy_s *policy,                             
                       uint8_t policy_count, 
                       blob_attr_t **tb, 
                       void *data, 
                       unsigned int len) 
{
    int i;
    blob_attr_t *pos, *head = (blob_attr_t *)data;

    dbg_str(DBG_DETAIL, "policy count=%d", policy_count);
    blob_for_each_attr(pos, head, len){
        for(i = 0; i < policy_count; i++) {
            if(!strncmp(blob_get_name((blob_attr_t *)pos), 
               policy[i].name, strlen(policy[i].name))) 
            {
                tb[i] = (blob_attr_t *)pos;
                dbg_str(DBG_DETAIL, "found policy %d, name=%s", i, policy[i].name);
            }
        }
    }

    return 0;
}

enum {                    
    FOO_U8,        
    FOO_STRING, 
    FOO_BUFFER, 
    FOO_TABLE, 
    FOO_TABLE2, 
    FOO_U32
}; 

static const struct blob_policy_s pol[] = {
    [FOO_U8] = {     
        .name = (char *)"u8", 
        .type = BLOB_TYPE_STRING, 
    }, 
    [FOO_STRING] = {
        .name = (char *)"value", 
        .type = BLOB_TYPE_ARRAY,  
    }, 
    [FOO_BUFFER] = {
        .name = (char *)"buffer", 
        .type = BLOB_TYPE_ARRAY,  
    }, 
    [FOO_TABLE] = {
        .name = (char *)"table1", 
        .type = BLOB_TYPE_TABLE,  
    }, 
    [FOO_TABLE2] = {
        .name = (char *)"table2", 
        .type = BLOB_TYPE_TABLE,  
    }, 
    [FOO_U32] = {     
        .name = (char *)"u32", 
        .type = BLOB_TYPE_INT32, 
    }, 
};

enum {                    
    TBL_U8,        
    TBL_U16,        
    TBL_U32,        
    TBL_TABLE2, 
}; 
static const struct blob_policy_s table[] = {
    [TBL_U8] = {     
        .name = (char *)"u8", 
        .type = BLOB_TYPE_INT8, 
    }, 
    [TBL_U16] = {     
        .name = (char *)"u16", 
        .type = BLOB_TYPE_INT16, 
    }, 
    [TBL_U32] = {     
        .name = (char *)"u32", 
        .type = BLOB_TYPE_INT32, 
    }, 
    [TBL_TABLE2] = {
        .name = (char *)"table2", 
        .type = BLOB_TYPE_TABLE,
    }, 
};
void test_blob()
{
    blob_t *blob;
    blob_attr_t *tb[ARRAY_SIZE(pol)] = {NULL};
    allocator_t *allocator = allocator_get_default_alloc();
    char buffer[] = "01234234214234234234234";
    
    blob = blob_create(allocator);
    blob_init(blob);

    blob_add_u8(blob, (char *)"u8", 8);
    blob_add_string(blob, (char *)"value", (char *)"hello world!");

    blob_add_table_start(blob, (char *)"table1");{
        blob_add_u8(blob, (char *)"u8", 8);
        blob_add_u16(blob, (char *)"u16", 16);
        blob_add_u32(blob, (char *)"u32", 32);

        blob_add_table_start(blob, (char *)"table2");{
            blob_add_u8(blob, (char *)"u8", 8);
            blob_add_u16(blob, (char *)"u16", 16);
            blob_add_u32(blob, (char *)"u32", 32);
        }
        blob_add_table_end(blob);
    }
    blob_add_table_end(blob);

    blob_add_u32(blob, (char *)"u32", 32);

    blob_add_buffer(blob, (char *)"buffer", (uint8_t *)buffer, sizeof(buffer));

    blob_parse_to_attr(pol, ARRAY_SIZE(pol), tb,
                       (void *)blob->head, 
                       (uint32_t)(blob->tail - blob->head)) ;

    if (tb[FOO_TABLE] != NULL) {
        dbg_str(DBG_DETAIL, "name :%s", blob_get_name(tb[FOO_TABLE]));
        blob_attr_t *tb2[10];
        blob_attr_t *attr = (blob_attr_t *)blob_get_data(tb[FOO_TABLE]);
        int len = blob_get_data_len(tb[FOO_TABLE]);
        blob_parse_to_attr(table, ARRAY_SIZE(table), tb2, 
                           attr, len) ;
    }
    blob_destroy(blob);
}

