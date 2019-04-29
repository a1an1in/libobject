/**
 * @buffer RingBuffer.c
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
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/io/RingBuffer.h>
#include <libobject/core/utils/registry/registry.h>

#define DEFAULT_BUFFER_SIZE 1024

static int __construct(RingBuffer *buffer,char *init_str)
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

static int __deconstrcut(RingBuffer *buffer)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    dbg_str(EV_DETAIL,"buffer deconstruct,buffer addr:%p",buffer);
    allocator_mem_free(allocator, buffer->addr);

    return 0;
}

static int __get_len(RingBuffer *buffer)
{
    if (buffer->w_offset > buffer->r_offset) {
        return buffer->w_offset - buffer->r_offset;
    } else {
        return buffer->w_offset - buffer->r_offset + buffer->size;
    }
}

static int __set_size(RingBuffer *buffer, int size)
{
    allocator_t *allocator = ((Obj *)buffer)->allocator;

    if (size != buffer->size) {
        allocator_mem_free(allocator, buffer->addr);
        buffer->size = size;
        buffer->addr = allocator_mem_alloc(allocator, size);
        if (buffer->addr == NULL) {
            dbg_str(DBG_ERROR, "allocator_mem_alloc");
            return -1;
        }
    }

    return 0;
}

static int __read(RingBuffer *buffer, void *dst, int len)
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
    } else if (l < buffer->size - buffer->r_offset) {
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

static int __read_to_string(RingBuffer *buffer, String *str, int len)
{
    int ret = 0;

    if (len < str->value_max_len) {
        str->assign_fixed_len(str, buffer->addr + buffer->r_offset, len);
        buffer->r_offset = (buffer->r_offset + len) % buffer->size;
        ret = 1;
    } else {
        dbg_str(DBG_ERROR, "string buffer is overflow, please reset");
        ret = -1;
    }

    return ret;
}

static void * __find(RingBuffer *buffer, void *needle)
{
    void *ret = NULL;

    if (buffer->last_operation_flag != BUFFER_WRITE_OPERATION &&
        buffer->w_offset == buffer->r_offset) {
        dbg_str(DBG_WARNNING, "buffer is nil");
        goto end;
    } 

    if (buffer->w_offset > buffer->r_offset) {
        ret = strnstr(buffer->addr + buffer->r_offset,
                      needle, buffer->w_offset - buffer->r_offset);
    } else {
        ret = strnstr(buffer->addr + buffer->r_offset,
                      needle, buffer->size - buffer->r_offset);
        if (ret != NULL) {
            goto end;
        }

        ret = strnstr(buffer->addr,
                      needle, buffer->w_offset);
    }

end:
    return ret;
}


static int __get_len_to_needle(RingBuffer *buffer, void *needle)
{
    void *addr;
    int len, needle_len, needle_offset;

    needle_len = strlen(needle);

    addr = buffer->find(buffer, needle);
    if (addr != NULL) {
        needle_offset = addr - buffer->addr;
        len = needle_offset - buffer->r_offset;
        if (len > 0) {
            return len + needle_len;
        } else {
            return (len + buffer->size + needle_len) % buffer->size;
        }
    }

    return -1;
}

static int __write(RingBuffer *buffer, void *src, int len)
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

static int __printf(RingBuffer *buffer, const char *fmt, ...)
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

static int __memcpy(RingBuffer *buffer, void *addr, int len)
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

static class_info_entry_t ring_buffer_class_info[] = {
    Init_Obj___Entry(0 , Stream, parent),
    Init_Nfunc_Entry(1 , RingBuffer, construct, __construct),
    Init_Nfunc_Entry(2 , RingBuffer, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , RingBuffer, set, NULL),
    Init_Vfunc_Entry(4 , RingBuffer, get, NULL),
    Init_Vfunc_Entry(5 , RingBuffer, read, __read),
    Init_Vfunc_Entry(6 , RingBuffer, read_to_string, __read_to_string),
    Init_Vfunc_Entry(7 , RingBuffer, write, __write),
    Init_Vfunc_Entry(8 , RingBuffer, find, __find),
    Init_Vfunc_Entry(9 , RingBuffer, get_len_to_needle, __get_len_to_needle),
    Init_Vfunc_Entry(10, RingBuffer, printf, __printf),
    Init_Vfunc_Entry(11, RingBuffer, memcopy, __memcpy),
    Init_Vfunc_Entry(12, RingBuffer, get_len, __get_len),
    Init_Vfunc_Entry(13, RingBuffer, set_size, __set_size),
    Init_End___Entry(14, RingBuffer),
};
REGISTER_CLASS("RingBuffer",ring_buffer_class_info);

int Test_ring_buffer(TEST_ENTRY *entry)
{
    RingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int len, ret;

    buffer = OBJECT_NEW(allocator, RingBuffer, NULL);

    buffer->set_size(buffer, 9);

    buffer->write(buffer, in, 5);
    buffer->read(buffer, out, 4);
    buffer->write(buffer, in + 5, 8);
    buffer->read(buffer, out + 4, 9);

    dbg_buf(DBG_DETAIL, "in:", in, 13);
    dbg_buf(DBG_DETAIL, "out:", out, 13);

    if (strncmp(in, out, 13) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(Test_ring_buffer);

int Test_ring_buffer_find(TEST_ENTRY *entry)
{
    RingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello world\r\n you gota work hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    buffer = OBJECT_NEW(allocator, RingBuffer, NULL);

    buffer->set_size(buffer, len);
    buffer->write(buffer, test, len);
    addr = buffer->find(buffer, "\r\n");

    dbg_str(DBG_DETAIL, "test addr:%p, find addr:%p", (buffer->addr + 15), addr);
    if (addr == (buffer->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(Test_ring_buffer_find);

int Test_ring_buffer_find2(TEST_ENTRY *entry)
{
    RingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello "\
                 "world\r\n "\
                 "you gotta "\
                 "wo rk hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    buffer = OBJECT_NEW(allocator, RingBuffer, NULL);

    buffer->set_size(buffer, len);
    buffer->write(buffer, test, len);

    buffer->w_offset = buffer->r_offset = 22;
    buffer->last_operation_flag = BUFFER_WRITE_OPERATION;

    addr = buffer->find(buffer, "\r\n");

    dbg_str(DBG_DETAIL, "test addr:%p, find addr:%p", (buffer->addr + 15), addr);
    if (addr == (buffer->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(Test_ring_buffer_find2);

int Test_ring_buffer_get_len_to_needle(TEST_ENTRY *entry)
{
    RingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello "\
                 "world\r\n yo"\
                 "u gotta wo"\
                 "rk hardddd";
    int len, ret;
    void *addr;

    len = strlen(test);

    buffer = OBJECT_NEW(allocator, RingBuffer, NULL);

    dbg_str(DBG_DETAIL, "buffer size=%d", len);
    buffer->set_size(buffer, len);
    buffer->write(buffer, test, len);

    buffer->w_offset = buffer->r_offset = 20;
    buffer->last_operation_flag = BUFFER_WRITE_OPERATION;

    len = buffer->get_len_to_needle(buffer, "\r\n");

    dbg_str(DBG_DETAIL, "len=%d", len);
    if (len == 37) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(Test_ring_buffer_get_len_to_needle);
