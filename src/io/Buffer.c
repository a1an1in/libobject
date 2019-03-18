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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/io/buffer.h>
#include <libobject/core/utils/registry/registry.h>

#define DEFAULT_BUFFER_SIZE 1024

static int __construct(Buffer *buffer,char *init_str)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    if (buffer->size == 0) {
        buffer->size  = DEFAULT_BUFFER_SIZE;
    }

    buffer->addr = allocator_mem_alloc(allocator, buffer->size);
    if (buffer->addr == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        return -1;
    }

    buffer->r_offset = 0;
    buffer->w_offset = 0;
    buffer->last_operation_flag = 0;

    return 0;
}

static int __deconstrcut(Buffer *buffer)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    dbg_str(EV_DETAIL,"buffer deconstruct,buffer addr:%p",buffer);
    allocator_mem_free(allocator, buffer->addr);

    return 0;
}

static int __set(Buffer *buffer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        buffer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        buffer->get = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        buffer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        buffer->deconstruct = value;
    } else if (strcmp(attrib, "read") == 0) {
        buffer->read = value;
    } else if (strcmp(attrib, "write") == 0) {
        buffer->write = value;
    } else if (strcmp(attrib, "printf") == 0) {
        buffer->printf = value;
    } else if (strcmp(attrib, "memcopy") == 0) {
        buffer->memcopy = value;
    } else if (strcmp(attrib, "get_len") == 0) {
        buffer->get_len = value;
    } else if (strcmp(attrib, "set_size") == 0) {
        buffer->set_size = value;
    } else {
        dbg_str(EV_DETAIL,"buffer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Buffer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"buffer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __get_len(Buffer *buffer)
{
}

static int __set_size(Buffer *buffer, int size)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    if (size != buffer->size) {
        allocator_mem_free(allocator, buffer->addr);
        buffer->size =  size;
        buffer->addr = allocator_mem_alloc(allocator, size);
        if (buffer->addr == NULL) {
            dbg_str(DBG_ERROR, "allocator_mem_alloc");
            return -1;
        }
    }

    return 0;
}

static int __read(Buffer *buffer, void *dst, int len)
{
    int l;

    if (buffer->last_operation_flag != BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        l = 0;
        dbg_str(DBG_WARNNING, "buffer is nil");
        goto end;
    } else if (buffer->last_operation_flag == BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        l = buffer->size;
    } else {
        l = (buffer->w_offset - buffer->r_offset + buffer->size) % buffer->size;
    }

    l = l > len ? len : l;

    dbg_str(DBG_DETAIL, "read len=%d, w=%d, r=%d",
            l, buffer->w_offset, buffer->r_offset);
    if (buffer->w_offset > buffer->r_offset) {
        memcpy(dst, buffer->addr + buffer->r_offset, l);
    } else {
        memcpy(dst, buffer->addr + buffer->r_offset, 
                buffer->size - buffer->r_offset);
        /*
         *dbg_buf(DBG_DETAIL, "read char:", buffer->addr + buffer->r_offset,
         *        buffer->size - buffer->r_offset );
         */
        memcpy(dst + buffer->size - buffer->r_offset,
                buffer->addr, 
                l - buffer->size + buffer->r_offset);
        /*
         *dbg_buf(DBG_DETAIL, "read char:", buffer->addr,
         *        l - buffer->size + buffer->r_offset);
         */
    }

    buffer->r_offset = (buffer->r_offset + l) % buffer->size;
    buffer->last_operation_flag = BUFFER_READ_OPERATION;

end:
    return l;
}

static int __write(Buffer *buffer, void *src, int len)
{
    int l;

    if (buffer->last_operation_flag == BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        l = 0;
        dbg_str(DBG_WARNNING, "buffer is full");
        goto end;
    } else if (buffer->w_offset == buffer->r_offset) {
        l =  buffer->size;
    } else {
        l = (buffer->r_offset - buffer->w_offset + buffer->size) % buffer->size;
    }

    l = l > len ? len : l;

    if (buffer->w_offset + l <= buffer->size) {
        memcpy(buffer->addr + buffer->w_offset, src, l);
    } else {
        memcpy(buffer->addr + buffer->w_offset,
               src, buffer->size - buffer->w_offset);
        memcpy(buffer->addr,  src + buffer->size - buffer->w_offset, 
                l -  buffer->size + buffer->w_offset);
    }

    buffer->w_offset = (buffer->w_offset + l) % buffer->size;
    buffer->last_operation_flag = BUFFER_WRITE_OPERATION;

end:
    return l;
}

static int __printf(Buffer *buffer, const char *fmt, ...)
{
    int ret;
    va_list va;
    int l;

    if (buffer->last_operation_flag == BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        ret = l = 0;
        dbg_str(DBG_WARNNING, "buffer is full");
        goto end;
    } else if (buffer->w_offset == buffer->r_offset) {
        l =  buffer->size;
    } else {
        l = (buffer->r_offset - buffer->w_offset + buffer->size) % buffer->size;
    }

    va_start(va, fmt);
    ret = vsnprintf(buffer->addr + buffer->w_offset,
                    l, fmt, va);
    buffer->w_offset = (buffer->w_offset + ret) % buffer->size;
    va_end(va);

end:
    return ret;
}

static int __memcpy(Buffer *buffer, void *addr, int len)
{
    int ret;
    va_list va;
    int l;

    if (buffer->last_operation_flag == BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        ret = l = 0;
        dbg_str(DBG_WARNNING, "buffer is full");
        goto end;
    } else if (buffer->w_offset == buffer->r_offset) {
        l =  buffer->size;
    } else {
        l = (buffer->r_offset - buffer->w_offset + buffer->size) % buffer->size;
    }

    l = l > len ? len : l;

    ret = memcpy(buffer->addr + buffer->w_offset, addr, l);
    buffer->w_offset = (buffer->w_offset + ret) % buffer->size;

end:
    return ret;
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
    Init_Vfunc_Entry(8 , Buffer, memcopy, __memcpy),
    Init_Vfunc_Entry(9 , Buffer, get_len, __get_len),
    Init_Vfunc_Entry(10, Buffer, set_size, __set_size),
    Init_End___Entry(11),
};
REGISTER_CLASS("Buffer",buffer_class_info);

int Test_buffer(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[9] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    char out[9];
    int len;

    buffer = OBJECT_NEW(allocator, Buffer, NULL);

    buffer->set_size(buffer, 9);
    buffer->write(buffer, in, 5);
    dbg_buf(DBG_DETAIL, "write 5 char:", in, 5);
    buffer->read(buffer, out, 4);
    dbg_buf(DBG_DETAIL, "read 4 char:", out, 4);
    buffer->write(buffer, in, 9);
    dbg_buf(DBG_DETAIL, "write 9 char:", in, 9);
    buffer->read(buffer, out, 9);
    dbg_buf(DBG_DETAIL, "read 9 char:", out, 9);


    object_destroy(buffer);

    return 1;
}
REGISTER_TEST_FUNC(Test_buffer);
