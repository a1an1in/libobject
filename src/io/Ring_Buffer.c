/**
 * @rb Ring_Buffer.c
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
#include <libobject/io/Ring_Buffer.h>

#define DEFAULT_BUFFER_SIZE 1024


static int __construct(Ring_Buffer *rb,char *init_str)
{
    allocator_t *allocator = ((Obj *)rb)->allocator;

    rb->size = 0;

    if (rb->size == 0) {
        rb->size  = DEFAULT_BUFFER_SIZE;
    }

    rb->addr = allocator_mem_alloc(allocator, rb->size);
    if (rb->addr == NULL) {
        dbg_str(IO_ERROR, "allocator_mem_alloc");
        return -1;
    }

    rb->r_offset = 0;
    rb->w_offset = 0;
    rb->last_operation_flag = 0;

    return 0;
}

static int __deconstrcut(Ring_Buffer *rb)
{
    allocator_t *allocator = ((Obj *)rb)->allocator;

    dbg_str(IO_DETAIL,"rb deconstruct,rb addr:%p",rb);
    allocator_mem_free(allocator, rb->addr);

    return 0;
}

static int __get_len(Ring_Buffer *rb)
{
    if (rb->w_offset > rb->r_offset) {
        return rb->w_offset - rb->r_offset;

    } else if (rb->last_operation_flag == BUFFER_READ_OPERATION &&
               rb->w_offset > rb->r_offset){
        return 0;
    } else {
        return (rb->w_offset - rb->r_offset + rb->size);
    }
}

static int __set_size(Ring_Buffer *rb, int size)
{
    allocator_t *allocator = ((Obj *)rb)->allocator;

    if (size != rb->size) {
        allocator_mem_free(allocator, rb->addr);
        rb->size = size;
        rb->addr = allocator_mem_alloc(allocator, size);
        if (rb->addr == NULL) {
            dbg_str(IO_ERROR, "allocator_mem_alloc");
            return -1;
        }
    }

    return 0;
}

static int __read(Ring_Buffer *rb, void *dst, int len)
{
    int l;

    if (rb->last_operation_flag != BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = 0;
        dbg_str(IO_WARNNING, "rb is nil");
        goto end;
    } else if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = rb->size;
    } else {
        l = (rb->w_offset - rb->r_offset + rb->size) % rb->size;
    }

    l = l > len ? len : l;

    dbg_str(IO_DETAIL, "read len=%d, w=%d, r=%d",
            l, rb->w_offset, rb->r_offset);

    if (rb->w_offset > rb->r_offset) {
        memcpy(dst, rb->addr + rb->r_offset, l);
    } else if (l < rb->size - rb->r_offset) {
        memcpy(dst, rb->addr + rb->r_offset, l);
    } else {
        memcpy(dst, rb->addr + rb->r_offset, 
                rb->size - rb->r_offset);
        /*
         *dbg_buf(IO_DETAIL, "read char:", rb->addr + rb->r_offset,
         *        rb->size - rb->r_offset );
         */
        memcpy(dst + rb->size - rb->r_offset,
               rb->addr, 
               l - rb->size + rb->r_offset);
        /*
         *dbg_buf(IO_DETAIL, "read char:", rb->addr,
         *        l - rb->size + rb->r_offset);
         */
    }

    rb->r_offset = (rb->r_offset + l) % rb->size;
    rb->last_operation_flag = BUFFER_READ_OPERATION;

end:
    return l;
}

static int __read_to_string(Ring_Buffer *rb, String *str, int len)
{
    int l;

    if (rb->last_operation_flag != BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = 0;
        dbg_str(IO_WARNNING, "rb is nil");
        goto end;
    } else if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = rb->size;
    } else {
        l = (rb->w_offset - rb->r_offset + rb->size) % rb->size;
    }

    l = l > len ? len : l;

    dbg_str(IO_DETAIL, "read len=%d, w=%d, r=%d",
            l, rb->w_offset, rb->r_offset);

    if (rb->w_offset > rb->r_offset) {
        str->append(str, rb->addr + rb->r_offset, l);
    } else if (l < rb->size - rb->r_offset) {
        str->append(str, rb->addr + rb->r_offset, l);
    } else {
        str->append(str, rb->addr + rb->r_offset,
                      rb->size - rb->r_offset);
        str->append(str, rb->addr,
                      l - rb->size + rb->r_offset);
    }

    rb->r_offset = (rb->r_offset + l) % rb->size;
    rb->last_operation_flag = BUFFER_READ_OPERATION;

end:
    return l;
}

static int __read_to_buffer(Ring_Buffer *rb, Buffer *buffer, int len)
{
    int l = 0;

    if (rb->last_operation_flag != BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = 0;
        dbg_str(IO_WARNNING, "ring buffer read to buffer, rb is nil");
        goto end;
    } else if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = rb->size;
    } else {
        l = (rb->w_offset - rb->r_offset + rb->size) % rb->size;
    }

    l = l > len ? len : l;

    /*
     *dbg_str(IO_DETAIL, "read len=%d, w=%d, r=%d",
     *        l, rb->w_offset, rb->r_offset);
     */

    if (rb->w_offset > rb->r_offset) {
        l = buffer->write(buffer, rb->addr + rb->r_offset, l);
    } else if (l < rb->size - rb->r_offset) {
        l = buffer->write(buffer, rb->addr + rb->r_offset, l);
    } else {
        buffer->write(buffer, rb->addr + rb->r_offset,
                      rb->size - rb->r_offset);
        buffer->write(buffer, rb->addr,
                      l - rb->size + rb->r_offset);
        /*
         *dbg_buf(IO_DETAIL, "read char:", rb->addr,
         *        l - rb->size + rb->r_offset);
         */
    }

    rb->r_offset = (rb->r_offset + l) % rb->size;
    rb->last_operation_flag = BUFFER_READ_OPERATION;

end:
    return l;

    return l;
}

static void* __find(Ring_Buffer *rb, void* needle, int needle_len, int len)
{
    void *haystack;
    int haystack_len, cnt = 0;
    void *tmp;


    haystack_len = rb->get_len(rb);

    if (haystack_len < len        || 
        haystack_len < needle_len ||
        needle_len > len) {
        return NULL;
    }

    while (cnt <= len - needle_len) {
        if (rb->r_offset + cnt + needle_len <= rb->size) {
            haystack = rb->addr + rb->r_offset + cnt;
            if (!memcmp(haystack, needle, needle_len)) {
                return haystack;
            }
        } else if (rb->r_offset + cnt >= rb->size) {
            haystack = rb->addr + ((cnt + rb->r_offset) % rb->size);
            if (!memcmp(haystack, needle, needle_len)) {
                return haystack;
            }
        } else {
            haystack = rb->addr + rb->r_offset + cnt;
            if (memcmp(haystack, needle,
                rb->size - (rb->r_offset + cnt)) != 0) {
                cnt++;
                continue;
            }

            haystack = rb->addr;
            if (!memcmp(haystack, needle + rb->size - rb->r_offset - cnt,
                        needle_len - (rb->size - rb->r_offset- cnt))) {
                return rb->addr + rb->r_offset + cnt;
            }
        }

        cnt++;
    }

    return NULL;
}

/*find needle in the haystack*/
static int __get_needle_offset(Ring_Buffer *rb, void *needle, int needle_len, int len)
{
    void *addr;
    int needle_offset;

    addr = rb->find(rb, needle, needle_len, len);
    if (addr != NULL) {
        needle_offset = addr - rb->addr;
        len = needle_offset - rb->r_offset;
        if (len > 0) {
            return len + needle_len;
        } else {
            return (len + rb->size + needle_len) % rb->size;
        }
    }

    return -1;
}

static int __write(Ring_Buffer *rb, void *src, int len)
{
    int l;

    if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        l = 0;
        dbg_str(IO_WARNNING, "rb is full");
        goto end;
    } else if (rb->w_offset == rb->r_offset) {
        l =  rb->size;
    } else {
        l = (rb->r_offset - rb->w_offset + rb->size) % rb->size;
    }

    l = l > len ? len : l;

    if (rb->w_offset + l <= rb->size) {
        memcpy(rb->addr + rb->w_offset, src, l);
    } else {
        memcpy(rb->addr + rb->w_offset,
               src, rb->size - rb->w_offset);
        memcpy(rb->addr,  src + rb->size - rb->w_offset, 
                l -  rb->size + rb->w_offset);
    }

    rb->w_offset = (rb->w_offset + l) % rb->size;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

end:
    return l;
}

static int __printf(Ring_Buffer *rb, const char *fmt, ...)
{
    int ret;
    va_list va;
    int l;

    if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        ret = l = 0;
        dbg_str(IO_WARNNING, "rb is full");
        goto end;
    } else if (rb->w_offset == rb->r_offset) {
        l =  rb->size;
    } else {
        l = (rb->r_offset - rb->w_offset + rb->size) % rb->size;
    }

    va_start(va, fmt);
    ret = vsnprintf(rb->addr + rb->w_offset,
                    l, fmt, va);
    rb->w_offset = (rb->w_offset + ret) % rb->size;
    va_end(va);

end:
    return ret;
}

static int __memcpy(Ring_Buffer *rb, void *addr, int len)
{
    int ret;
    va_list va;
    int l;

    if (rb->last_operation_flag == BUFFER_WRITE_OPERATION &&
        rb->w_offset == rb->r_offset) {
        ret = l = 0;
        dbg_str(IO_WARNNING, "rb is full");
        goto end;
    } else if (rb->w_offset == rb->r_offset) {
        l =  rb->size;
    } else {
        l = (rb->r_offset - rb->w_offset + rb->size) % rb->size;
    }

    l = l > len ? len : l;

    memcpy(rb->addr + rb->w_offset, addr, l);
    dbg_str(IO_DETAIL, "memcpy, ret=%d, l=%d", ret, l);
    rb->w_offset = (rb->w_offset + l) % rb->size;

end:
    return ret;
}

static class_info_entry_t ring_rb_class_info[] = {
    Init_Obj___Entry(0 , Stream, parent),
    Init_Nfunc_Entry(1 , Ring_Buffer, construct, __construct),
    Init_Nfunc_Entry(2 , Ring_Buffer, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Ring_Buffer, set, NULL),
    Init_Vfunc_Entry(4 , Ring_Buffer, get, NULL),
    Init_Vfunc_Entry(5 , Ring_Buffer, read, __read),
    Init_Vfunc_Entry(6 , Ring_Buffer, read_to_string, __read_to_string),
    Init_Vfunc_Entry(7 , Ring_Buffer, read_to_buffer, __read_to_buffer),
    Init_Vfunc_Entry(8 , Ring_Buffer, write, __write),
    Init_Vfunc_Entry(9 , Ring_Buffer, find, __find),
    Init_Vfunc_Entry(10, Ring_Buffer, get_needle_offset, __get_needle_offset),
    Init_Vfunc_Entry(11, Ring_Buffer, printf, __printf),
    Init_Vfunc_Entry(12, Ring_Buffer, memcopy, __memcpy),
    Init_Vfunc_Entry(13, Ring_Buffer, get_len, __get_len),
    Init_Vfunc_Entry(14, Ring_Buffer, set_size, __set_size),
    Init_End___Entry(15, Ring_Buffer),
};
REGISTER_CLASS("Ring_Buffer",ring_rb_class_info);

int test_ring_rb(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int len, ret;

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, 9);

    rb->write(rb, in, 5);
    rb->read(rb, out, 4);
    rb->write(rb, in + 5, 8);
    rb->read(rb, out + 4, 9);

    dbg_buf(IO_DETAIL, "in:", (uint8_t *)in, 13);
    dbg_buf(IO_DETAIL, "out:", (uint8_t *)out, 13);

    if (strncmp(in, out, 13) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb);

int test_ring_rb_find(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello world\r\n you gota work hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);
    addr = rb->find(rb, "\r\n", 2, len);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 15), addr);
    if (addr == (rb->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find);

int test_ring_rb_find2(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello "\
                 "world\r\n "\
                 "you gotta "\
                 "wo rk hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 22;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    addr = rb->find(rb, "\r\n", 2, len);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 15), addr);
    if (addr == (rb->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find2);

int test_ring_rb_find3(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello "\
                 "world\r\n   "\
                 "you gotta "\
                 "wo rk hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 30;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    addr = rb->find(rb, "dabc", 4, len);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 39), addr);
    if (addr == (rb->addr + 39)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find3);

int test_ring_rb_get_needle_offset(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *test = "abc hello "\
                 "world\r\n yo"\
                 "u gotta wo"\
                 "rk hardddd";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    dbg_str(IO_DETAIL, "rb size=%d", len);
    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 20;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    len = rb->get_needle_offset(rb, "\r\n", 2, len);

    dbg_str(IO_DETAIL, "len=%d", len);
    if (len == 37) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_get_needle_offset);
