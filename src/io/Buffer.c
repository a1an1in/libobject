/**
 * @buffer Buffer.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-01-13
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/io/Buffer.h>

#define DEFAULT_BUFFER_SIZE 1024

static int __construct(Buffer *buffer,char *init_str)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    buffer->capacity = 0;

    if (buffer->capacity == 0) {
        buffer->capacity  = DEFAULT_BUFFER_SIZE;
    }

    buffer->addr = allocator_mem_alloc(allocator, buffer->capacity);
    if (buffer->addr == NULL) {
        dbg_str(IO_ERROR, "allocator_mem_alloc");
        return -1;
    }

    buffer->r_offset = 0;
    buffer->w_offset = 0;
    buffer->len = 0;

    return 0;
}

static int __deconstrcut(Buffer *buffer)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    dbg_str(IO_DETAIL,"buffer deconstruct,buffer addr:%p",buffer);
    allocator_mem_free(allocator, buffer->addr);

    return 0;
}

static int __get_len(Buffer *buffer)
{
    return buffer->w_offset;
}

static int __get_free_capacity(Buffer *buffer)
{
    return buffer->capacity - buffer->w_offset;
}

static int __set_capacity(Buffer *buffer, int capacity)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    if (capacity > buffer->capacity) {
        allocator_mem_free(allocator, buffer->addr);
        buffer->capacity = capacity;
        buffer->addr = allocator_mem_alloc(allocator, capacity);
        if (buffer->addr == NULL) {
            dbg_str(IO_ERROR, "allocator_mem_alloc");
            return -1;
        }
    }

    return 0;
}

static int __read(Buffer *buffer, void *dst, int len)
{
    int l;

    if (buffer->w_offset == buffer->r_offset) {
        l = 0;
        dbg_str(IO_WARNNING, "content is nil");
        goto end;
    } else {
        l = buffer->w_offset - buffer->r_offset;
    }

    l = l > len ? len : l;

    memcpy(dst, buffer->addr + buffer->r_offset, l);

    buffer->r_offset = buffer->r_offset + l;

end:
    return l;
}

static int __write(Buffer *buffer, void *src, int len)
{
    int l;
    allocator_t *allocator = ((Obj *)buffer)->allocator;
    int capacity;
    void *new_buf;

    l = buffer->capacity - buffer->w_offset ;

    if (l > len) {
        memcpy(buffer->addr + buffer->w_offset, src, len);
        buffer->w_offset += len;
    } else {
        capacity = buffer->capacity;
        capacity = capacity > len ? capacity : len;
        capacity *= 2;
        new_buf = allocator_mem_alloc(allocator, capacity);
        if (new_buf == NULL) {
            dbg_str(DBG_ERROR, "buffer alloc buf failed!");
            return -1;
        } 
        memset(new_buf, 0, capacity);
        buffer->capacity = capacity;
        memmove(new_buf, buffer->addr, buffer->w_offset);
        memcpy(new_buf + buffer->w_offset, src, len);
        buffer->w_offset += len;

        allocator_mem_free(allocator, buffer->addr);
        buffer->addr = new_buf;
    }

end:
    return len;
}

static int __printf(Buffer *buffer, int len, const char *fmt, ...)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;
    int capacity, l, ret;
    void *new_buf;
    va_list va;

    l = buffer->capacity - buffer->w_offset ;

    if (l < len) {
        capacity = buffer->capacity;
        capacity = capacity > len ? capacity : len;
        capacity *= 2;
        new_buf = allocator_mem_alloc(allocator, capacity);
        if (new_buf == NULL) {
            dbg_str(DBG_ERROR, "buffer alloc buf failed!");
            return -1;
        } 
        memset(new_buf, 0, capacity);
        buffer->capacity = capacity;
        memmove(new_buf, buffer->addr, buffer->w_offset);
        allocator_mem_free(allocator, buffer->addr);
        buffer->addr = new_buf;
    }

    va_start(va, fmt);
    ret = vsnprintf(buffer->addr + buffer->w_offset,
                    len, fmt, va);
    va_end(va);
    if (ret < 0 || ret >= len) {
        dbg_str(DBG_ERROR, "string is longer than the desigated len, please set a longer len!!");
        return -1;
    }

    buffer->w_offset += ret;

    return ret;
}

static char *__reset(Buffer *buffer) 
{
    buffer->r_offset = 0;
    buffer->w_offset = 0;
    buffer->len = 0;

    return 0;
}

static class_info_entry_t buffer_class_info[] = {
    Init_Obj___Entry(0 , Stream, parent),
    Init_Nfunc_Entry(1 , Buffer, construct, __construct),
    Init_Nfunc_Entry(2 , Buffer, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Buffer, set, NULL),
    Init_Vfunc_Entry(4 , Buffer, get, NULL),
    Init_Vfunc_Entry(5 , Buffer, read, __read),
    Init_Vfunc_Entry(6 , Buffer, write, __write),
    Init_Vfunc_Entry(7 , Buffer, printf, __printf),
    Init_Vfunc_Entry(8 , Buffer, reset, __reset),
    Init_Vfunc_Entry(9 , Buffer, get_len, __get_len),
    Init_Vfunc_Entry(10, Buffer, set_capacity, __set_capacity),
    Init_Vfunc_Entry(11, Buffer, get_free_capacity, __get_free_capacity),
    Init_End___Entry(12, Buffer),
};
REGISTER_CLASS("Buffer", buffer_class_info);

int test_buffer(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int len, ret;

    buffer = OBJECT_NEW(allocator, Buffer, NULL);

    buffer->set_capacity(buffer, 14);

    buffer->write(buffer, in, 5);
    buffer->write(buffer, in + 5, 8);
    buffer->read(buffer, out, 4);
    buffer->read(buffer, out + 4, 9);

    dbg_buf(IO_DETAIL, "in:", (uint8_t *)in, 13);
    dbg_buf(IO_DETAIL, "out:", (uint8_t *)out, 13);

    if (strncmp(in, out, 13) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(test_buffer);
