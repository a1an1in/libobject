/**
 * @file ring_buffer.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-10-27
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
#include <libobject/core/utils/data_structure/ring_buffer.h>
#include <libobject/core/utils/alloc/allocator.h>

int rbuf_set_buffer_addr(ring_buffer_t *rbuf, void *value, uint32_t len)
{
    len = !!len;//to clear compile warnning

    rbuf->buffer_addr = (char *)value;
    dbg_str(DBG_DETAIL, "rbuf_set_buffer_addr:%p", rbuf->buffer_addr);

    return 1;
}

int rbuf_get_buffer_addr(ring_buffer_t *rbuf, char *value)
{
    /*
     **value = rbuf->buffer_addr;
     */
    return 1;
}

int rbuf_set_buffer_len(ring_buffer_t *rbuf, void *value, uint32_t len)
{
    len = !!len;

    rbuf->buffer_len = *(uint32_t*)value;
    dbg_str(DBG_DETAIL, "rbuf_set_buffer_len:%d", rbuf->buffer_len);

    return 1;
}

static inline int 
rbuf_is_full(ring_buffer_t *rbuf, uint32_t len)
{
    if(((rbuf->read_index - rbuf->write_index + rbuf->buffer_len) % 
                (rbuf->buffer_len + 1))  < len)
    {
        return 1;
    } else if (rbuf->read_index == rbuf->write_index && 
            rbuf->rw_flag == RW_FLAG_WRITE)
    {
        return 1;
    }else{
        return 0;
    }
}

static inline int 
rbuf_is_null(ring_buffer_t *rbuf)
{
    if (rbuf->read_index == rbuf->write_index && rbuf->rw_flag == RW_FLAG_READ){
        return 1;
    } else {
        return 0;
    }
}

ring_buffer_t *rbuf_create()
{
    ring_buffer_t *rbuf = NULL;

    rbuf = (ring_buffer_t *)malloc(sizeof(ring_buffer_t));
    if(rbuf == NULL) return NULL;

    return rbuf;
}
int rbuf_set(ring_buffer_t *rbuf, char *attrib, void *value, uint32_t len)
{
    int ret = 0;
    if(!strcmp(attrib, "buffer_addr")){
        rbuf_set_buffer_addr(rbuf, value, len);
    }else if(!strcmp(attrib, "buffer_len")){
        rbuf_set_buffer_len(rbuf, value, len);
    }else{
        dbg_str(DBG_WARNNING, "not support now");
    }

    return ret;
}

int rbuf_init(ring_buffer_t *rbuf)
{
    int ret = 0;

    rbuf->write_index = 0;
    rbuf->read_index  = 0;
    rbuf->rw_flag = 0;

    return ret;
}

int rbuf_write(ring_buffer_t *rbuf, void *value)
{
    rbuf_msg_head_t *head = (rbuf_msg_head_t *)value;
    uint32_t len = head->len;

    dbg_str(DBG_DETAIL, "rbuf_write, write_index =%d", rbuf->write_index);

    if(rbuf_is_full(rbuf, len)) {
        dbg_str(DBG_DETAIL, "rbuf is full");
        return 0;
    } else if(rbuf->write_index + len > rbuf->buffer_len){
        int32_t first_write_len, residue;
        first_write_len = rbuf->buffer_len - rbuf->write_index;
        residue = len - first_write_len;
        memcpy(rbuf->buffer_addr + rbuf->write_index,
               (char *)value, first_write_len);
        rbuf->write_index = 0;
        memcpy(rbuf->buffer_addr + rbuf->write_index, 
               (char *)value + first_write_len, residue);
        rbuf->write_index = residue;
    } else{
        memcpy(rbuf->buffer_addr + rbuf->write_index, value, len);
        rbuf->write_index += len;
    }

    rbuf->rw_flag = RW_FLAG_WRITE;

    return len;
}

int rbuf_read(ring_buffer_t *rbuf, void *value_out)
{
    rbuf_msg_head_t *head = (rbuf_msg_head_t *)(rbuf->buffer_addr + rbuf->read_index);
    uint32_t len = head->len;

    dbg_str(DBG_DETAIL, "rbuf_write, read_index =%d", rbuf->read_index);

    if(rbuf_is_null(rbuf)){
        dbg_str(DBG_DETAIL, "rbuf is null");
        return 0;
    } else if(rbuf->read_index + len > rbuf->buffer_len){
        int32_t first_read_len, residue;
        first_read_len = rbuf->buffer_len - rbuf->read_index;
        residue = len - first_read_len;
        memcpy(value_out, rbuf->buffer_addr + rbuf->read_index, first_read_len);
        rbuf->read_index = 0;
        memcpy((char *)value_out + first_read_len,
               (char *)rbuf->buffer_addr + rbuf->read_index, residue);
        rbuf->read_index += residue;
    } else{
        memcpy(value_out, (char *)rbuf->buffer_addr + rbuf->read_index, len);
        rbuf->read_index += len;
    }
    rbuf->rw_flag = RW_FLAG_READ;

    return len;
}

int rbuf_destroy(ring_buffer_t *rbuf)
{
    int ret = 0;

    free(rbuf);

    return ret;
}

struct test_s{
    rbuf_msg_head_t head;
    int a;
    int b;
    int c;
    int d;
};

void print_test(struct test_s* t)
{
    dbg_str(DBG_DETAIL, "head.len=%d, a=%d, b=%d, c=%d, d=%d",
            t->head.len, t->a, t->b, t->c, t->d);
}

void test_datastructure_ring_buffer()
{
    ring_buffer_t *rbuf;
    char *buffer_addr;
    uint32_t len = 1024;
    allocator_t *allocator = allocator_get_default_alloc();
    char *value_out[100];
    int ret;

    struct test_s t = {
        .head.len = sizeof(struct test_s), 
        .a = 1, 
        .b = 2, 
        .c = 3, 
        .d = 4, 
    };

    buffer_addr = (char *)allocator_mem_alloc(allocator, len);

    dbg_str(DBG_DETAIL, "test_datastructure_ring_buffer");
    dbg_str(DBG_DETAIL, "buffer_addr:%p, len=%d", buffer_addr, len);
    dbg_str(DBG_DETAIL, "test len=%d", t.head.len);
    rbuf = rbuf_create();

    rbuf_set(rbuf, (char *)"buffer_addr", (void *)buffer_addr, sizeof(char *));
    rbuf_set(rbuf, (char *)"buffer_len", (void *)&len, sizeof(len));

    rbuf_init(rbuf);

    rbuf_write(rbuf, &t);
    rbuf_write(rbuf, &t);
    rbuf_write(rbuf, &t);
    rbuf_write(rbuf, &t);
    rbuf_write(rbuf, &t);

    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);
    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);
    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);
    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);
    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);
    ret = rbuf_read(rbuf, value_out);
    if(ret > 0)
        print_test((struct test_s*)value_out);

    rbuf_destroy(rbuf);

}

