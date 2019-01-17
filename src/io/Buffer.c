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

#define DEFAULT_BUFFER_SIZE 4096

static int __construct(Buffer *self,char *init_str)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    if (self->size == 0) {
        self->size  = DEFAULT_BUFFER_SIZE + 1 ;
    } else {
        self->size = (self->size + 1 + sizeof(int) - 1 )&(~(sizeof(int) -1 )) ;
    }
    
    self->buffer = allocator_mem_alloc(allocator, self->size);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        return -1;
    }

    self->r_offset       = 0;
    self->w_offset       = 0;
    self->available_size = self->size - 1;
    self->used_size      = 0;
    self->ref_count      = 1;
    return 0;
}

static int __deconstrcut(Buffer *self)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    dbg_str(EV_DETAIL,"buffer deconstruct,buffer addr:%p",self);
    allocator_mem_free(allocator, self->buffer);
    self->ref_count = 0;
    return 0;
}

static int __set(Buffer *buffer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        buffer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        buffer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        buffer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        buffer->deconstruct = value;
    } else if (strcmp(attrib, "buffer_read") == 0) {
        buffer->buffer_read = value;
    } else if (strcmp(attrib, "buffer_write") == 0) {
        buffer->buffer_write = value;
    } else if (strcmp(attrib, "size") == 0) {
        buffer->size = value;
    } else if (strcmp(attrib, "is_empty") == 0) {
        buffer->is_empty = value;
    } else if (strcmp(attrib, "clear") == 0) {
        buffer->clear = value;
    } else if (strcmp(attrib, "buffer_move") == 0) {
        buffer->buffer_move = value;
    } else if (strcmp(attrib, "buffer_find") == 0) {
        buffer->buffer_find = value;
    } else if (strcmp(attrib, "buffer_used_size") == 0) {
        buffer->buffer_used_size = value;
    } else if (strcmp(attrib, "buffer_free_size") == 0) {
        buffer->buffer_free_size = value;
    } else if (strcmp(attrib, "buffer_copy") == 0) {
        buffer->buffer_copy = value;
    } else if (strcmp(attrib, "buffer_remove") == 0) {
        buffer->buffer_remove = value;
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

static int __size (Buffer *self)
{
    return self->size -1;
}

static int __buffer_free_size(Buffer *self)
{
    return self->available_size;
} 

static int __buffer_used_size(Buffer *self)
{
    return self->used_size;
}

static int __is_empty(Buffer *self)
{
    return self->used_size ? 0:1;
}

static int __clear(Buffer *self)
{
    self->r_offset       = 0;
    self->w_offset       = 0;
    self->used_size      = 0;
    self->available_size = self->size -1 ;
    return 1;
}

//读指针 追 写指针

static  int __buffer_adapter_internal(Buffer *self)
{
    if (self->r_offset < self->w_offset) {
        self->used_size = self->w_offset - self->r_offset;
    } else if (self->r_offset > self->w_offset ) {
        self->used_size = self->size - self->r_offset + self->w_offset;
    } else {
        self->used_size = 0;
    }

    self->available_size = self->size -1 - self->used_size;
    return 1;
}

static int __buffer_expand_container(Buffer *self,int len)
{
    int available_write_size = self->available_size;
    int capacity = self->size ;
    void * tmp_buffer = NULL;
    if (len <= available_write_size) {
        return 0;
    }

    len = (2*capacity + len + sizeof(int) - 1 )&(~(sizeof(int) -1 ));
    tmp_buffer = self->buffer;
    //allocator_mem_free(allocator,self->buffer);
    self->buffer = allocator_mem_alloc(allocator, len);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        self->buffer = tmp_buffer;
        return -1;
    }
    
    if (self->r_offset < self->w_offset){
        memcpy(self->buffer+r_offset,tmp_buffer+r_offset,self->used_size);
    } else if (self->w_offset < self->r_offset) {
        memcpy(self->buffer,tmp_buffer,self->w_offset);
        memcpy(self->buffer+r_offset,tmp_buffer+r_offset,self->size - self->r_offset);
    } else {

    }
    allocator_mem_free(allocator,tmp_buffer);

    self->size = len -1;
    self->available_size = self->size - self->used_size - 1;

    return 0;
}

static int __buffer_read(Buffer *buffer, void *dst, int len)
{
   int  available_read_size = self->used_size;
   int  internal_end_size   = self->size - self->r_offset;

   if ( available_read_size == 0 || dst == NULL || len <= 0 ) {
       len = 0;
       return len;
   }
   //保证不会多读
   if ( len > available_read_size ) {
       len = available_read_size;
   }
   //两种情况
   //r < w 
   if (self->r_offset < self->w_offset) {
       memcpy(dst,self->buffer+r_offset,len);
       self->r_offset =+ len;
   // r > w
   } else if (self->r_offset > self->w_offset) {
       if (internal_end_size >= len ) {
           memcpy(dst,self->buffer+self->r_offset,len);
           self->r_offset += len;
       } else {
           memcpy(dst,self->buffer+self->r_offset,internal_end_size);
           self->r_offset += internal_end_size;
           memcpy(dst+internal_end_size,self->buffer,len - internal_end_size);
           self->r_offset = len - internal_end_size;
       }
   }

   self->buffer_adapter_internal(self);
   return len;
}

static int __buffer_write(Buffer *self, void *src, int len)
{ 
    int available_write_size = self->available_size;
    int internal_end_size    = self->size - self->w_offset;

    if (available_write_size == 0 || src == NULL || len <= 0 ) {
        len = 0;
        return len;
    }
    //保证不会多写
   if ( len > available_write_size ) {
       len = available_write_size;
   }
   //两种情况
   //r < w 
   if (self->r_offset < self->w_offset ) {
       if (len <= internal_end_size) {
           memcpy(self->buffer+self->w_offset,src,len);
           self->w_offset += len;
       } else {
           memcpy(self->buffer+self->w_offset,src,internal_end_size);
           self->w_offset += internal_end_size;
           memcpy(self->buffer,src+internal_end_size,len - internal_end_size);
           self->w_offset = len - internal_end_size;
       }
   }
   self->buffer_adapter_internal(self);
   return len;
}

static  int __buffer_find(Buffer *self,u_char c)
{
    int i;
    int internal_end_size;
    if (self->is_empty(self)) {
        return -1;
    }

    if (self->r_offset < self->w_offset) {
        for (i = self->r_offset ;i < self->w_offset;i++) {
             if (*(self->buffer+i) == c) {
                 return i;
             }
        }
    } else if (self->r_offset > self->w_offset) {
        internal_end_size = self->size - self->r_offset;
        for (i = self->r_offset;i < self->size;i++) {
            if (*(self->buffer+i) == c) {
                 return i;
            }
        }

        internal_end_size = self->used_size - internal_end_size;
        for (i = 0 ;i < internal_end_size;i++){
            if (*(self->buffer+i) == c) {
                 return i;
             }
        }
    }
    return -1;
}

static void __buffer_destroy(Buffer *self)
{
    if (--self->ref_count == 0){
        object_destroy(self);
    }
}

static int __buffer_copy_ref(Buffer *self,Buffer *dst)
{   
    if (dst == NULL||self == NULL ) {
        return -1;
    }
    dst = self;
    self->ref_count += 1;
    return 0;
}

static int __buffer_copy(Buffer *self,Buffer *dst,size_t len ) 
{   

}

static class_info_entry_t buffer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Stream","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","read", __read,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","write", __write,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","printf", __printf,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","memcopy", __memcpy,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_len", __get_len,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","set_size", __set_size,sizeof(void *)},
    [11] = {ENTRY_TYPE_END},
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
