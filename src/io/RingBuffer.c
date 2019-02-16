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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/io/RingBuffer.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/thread.h>
#define DEFAULT_BUFFER_SIZE 1024

static int __construct(RingBuffer *self,char *init_str)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    if (self->capacity == 0) {
        self->capacity  = DEFAULT_BUFFER_SIZE + 1 ;
    } else {
        self->capacity = (self->capacity + 1 + sizeof(int) - 1 )&(~(sizeof(int) - 1 )) ;
    }
    
    self->buffer = allocator_mem_alloc(allocator, self->capacity);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        return -1;
    }

    self->r_offset       = 0;
    self->w_offset       = 0;
    self->available_size = self->capacity - 1;
    self->used_size      = 0;
    self->ref_count      = 1;
    return 0;
}

static int __deconstrcut(RingBuffer *self)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    dbg_str(EV_DETAIL,"buffer deconstruct,buffer addr:%p",self);
    allocator_mem_free(allocator, self->buffer);
    self->ref_count = 0;
    return 0;
}

static int __set(RingBuffer *buffer, char *attrib, void *value)
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
    } else if (strcmp(attrib, "buffer_move_unref") == 0) {
        buffer->buffer_move_unref = value;
    } else if (strcmp(attrib, "buffer_find") == 0) {
        buffer->buffer_find = value;
    } else if (strcmp(attrib, "buffer_used_size") == 0) {
        buffer->buffer_used_size = value;
    } else if (strcmp(attrib, "buffer_free_size") == 0) {
        buffer->buffer_free_size = value;
    } else if (strcmp(attrib, "buffer_copy") == 0) {
        buffer->buffer_copy = value;
    } else if (strcmp(attrib, "buffer_destroy") == 0) {
        buffer->buffer_destroy = value;
    } else if (strcmp(attrib, "buffer_adapter_internal") == 0) {
        buffer->buffer_adapter_internal = value;
    } else if (strcmp(attrib, "buffer_expand_container") == 0) {
        buffer->buffer_expand_container = value;
    } else {
        dbg_str(EV_DETAIL,"buffer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(RingBuffer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"buffer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __size (RingBuffer *self)
{
    return self->capacity -1;
}

static int __buffer_free_size(RingBuffer *self)
{
    return self->available_size;
} 

static int __buffer_used_size(RingBuffer *self)
{
    return self->used_size;
}

static int __is_empty(RingBuffer *self)
{
    return self->used_size ? 0:1;
}

static int __clear(RingBuffer *self)
{
    self->r_offset       = 0;
    self->w_offset       = 0;
    self->used_size      = 0;
    self->available_size = self->capacity -1 ;
    return 1;
}

//读指针 追 写指针

static  int __buffer_adapter_internal(RingBuffer *self)
{
    if (self->r_offset < self->w_offset) {
        self->used_size = self->w_offset - self->r_offset;
    } else if (self->r_offset > self->w_offset ) {
        self->used_size = self->capacity - self->r_offset + self->w_offset;
    } else {
        self->used_size = 0;
    }

    self->available_size = self->capacity -1 - self->used_size;
    return 1;
}

static int __buffer_expand_container(RingBuffer *self,int len)
{
    int available_write_size = self->available_size;
    int capacity = self->capacity ;
    void * tmp_buffer = NULL;
    if (len <= available_write_size) {
        return 0;
    }

    len = (2*capacity + len + sizeof(int) - 1 )&(~(sizeof(int) -1 ));
    tmp_buffer = self->buffer;
    //allocator_mem_free(allocator,self->buffer);
    self->buffer = allocator_mem_alloc((self->parent.obj).allocator, len);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        self->buffer = tmp_buffer;
        return -1;
    }
    
    if (self->r_offset < self->w_offset){
        memcpy(self->buffer+self->r_offset,tmp_buffer+self->r_offset,self->used_size);
    } else if (self->w_offset < self->r_offset) {
        memcpy(self->buffer,tmp_buffer,self->w_offset);
        memcpy(self->buffer+self->r_offset,tmp_buffer+self->r_offset,self->capacity - self->r_offset);
    } else {
        
    }
    allocator_mem_free((self->parent.obj).allocator,tmp_buffer);

    self->capacity = len -1;
    self->available_size = self->capacity - self->used_size - 1;

    return 0;
}

static int __buffer_read(RingBuffer *self, void *dst, int len)
{
   int  available_read_size = self->used_size;
   int  internal_end_size   = self->capacity - self->r_offset;

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
       memcpy(dst,self->buffer+self->r_offset,len);
       self->r_offset += len;
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

static int __buffer_write(RingBuffer *self, void *src, int len)
{ 
    int available_write_size = self->available_size;
    int internal_end_size    = self->capacity - self->w_offset;

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
   if (self->r_offset <= self->w_offset ) {
       if (len <= internal_end_size) {
           memcpy(self->buffer+self->w_offset,src,len);
           self->w_offset += len;
       } else {
           memcpy(self->buffer+self->w_offset,src,internal_end_size);
           self->w_offset += internal_end_size;
           memcpy(self->buffer,src+internal_end_size,len - internal_end_size);
           self->w_offset = len - internal_end_size;
       }
   } else if (self->r_offset > self->w_offset) {
       memcpy(self->buffer+self->w_offset,src,len);
       self->w_offset += len;
   }

   self->buffer_adapter_internal(self);
   return len;
}

static  int __buffer_find(RingBuffer *self,u_char c)
{
    int i;
    int internal_end_size;
    if (self->is_empty(self)) {
        return -1;
    }

    if (self->r_offset < self->w_offset) {
        for (i = self->r_offset ;i < self->w_offset;i++) {
             if (*((char*)self->buffer+i) == c) {
                 return i;
             }
        }
    } else if (self->r_offset > self->w_offset) {
        internal_end_size = self->capacity - self->r_offset;
        for (i = self->r_offset;i < self->capacity;i++) {
            if (*((char*)self->buffer+i) == c) {
                 return i;
            }
        }

        internal_end_size = self->used_size - internal_end_size;
        for (i = 0 ;i < internal_end_size;i++){
            if (*((char*)self->buffer+i) == c) {
                 return i;
             }
        }
    }
    return -1;
}

static void __buffer_destroy(RingBuffer *self)
{
    if (--self->ref_count == 0){
        object_destroy(self);
    }
}

static int __buffer_copy_ref(RingBuffer *self,RingBuffer *dst)
{   
    if (dst == NULL||self == NULL ) {
        return -1;
    }
    dst = self;
    self->ref_count += 1;
    return 0;
}

static int __buffer_copy(RingBuffer *self,RingBuffer *dst,size_t len ) 
{   
    return -1;
}

static int __buffer_move_unref(RingBuffer *self,RingBuffer*dst,size_t len)
{
    return -1;
}

static class_info_entry_t buffer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Stream","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_read", __buffer_read,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_write", __buffer_write,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","size", __size,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","clear", __clear,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","is_empty", __is_empty,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_copy_ref", __buffer_copy_ref,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_destroy", __buffer_destroy,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_move_unref", __buffer_move_unref,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_copy", __buffer_copy,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_expand_container", __buffer_expand_container,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_adapter_internal", __buffer_adapter_internal,sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_used_size", __buffer_used_size,sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_free_size", __buffer_free_size,sizeof(void *)},
    [18] = {ENTRY_TYPE_VFUNC_POINTER,"","buffer_find", __buffer_find,sizeof(void *)},
    [19] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("RingBuffer",buffer_class_info);

int Test_buffer(TEST_ENTRY *entry)
{
    RingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[9] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    char out[9];
    int len;
    char *p = "fdasfasd";
    char dst[100]={0};

    buffer = OBJECT_NEW(allocator, RingBuffer, NULL);
    
    len=buffer->buffer_write(buffer,p,strlen(p));
    
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

   len=buffer->buffer_write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    len=buffer->buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);
  
    
    buffer->buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);
    
    buffer->buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    memset(dst,0,100);
    buffer->buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    
    len=buffer->buffer_write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    len=buffer->buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    
    len=buffer->buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->buffer_used_size(buffer),
                buffer->buffer_free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    object_destroy(buffer);

    return 1;
}
REGISTER_TEST_FUNC(Test_buffer);



static void* ringbuffer_producer(void * arg)
{
    RingBuffer * buffer = (RingBuffer *)arg;
    char * p ="pruducer ";
    int size ;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->buffer_write(buffer,p,strlen(p));
        
        if (size){
            dbg_str(DBG_SUC,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->buffer_used_size(buffer),
                    buffer->buffer_free_size(buffer));
            usleep(200000);
        }
        else {
            dbg_str(DBG_ERROR,"producer buffer is full");
            dbg_str(DBG_ERROR,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->buffer_used_size(buffer),
                    buffer->buffer_free_size(buffer));
            usleep(1000000);
        }
    }
    return NULL;
}

static void * ringbuffer_consumer(void *arg)
{
    RingBuffer * buffer = (RingBuffer *)arg;
    char tmp[1024] = {0};
    char * p ="pruducer ";
    int size;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->buffer_read(buffer,tmp,sizeof(tmp)/sizeof(char));
        //usleep(10000);
        if (size){
            dbg_str(DBG_SUC,"consume buffer:%s",tmp);
            dbg_str(DBG_SUC,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->buffer_used_size(buffer),
                    buffer->buffer_free_size(buffer));

            memset(tmp,0,100);
        }  else {
            dbg_str(DBG_ERROR,"consume ringbuffer is nil ");
            dbg_str(DBG_ERROR,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->buffer_used_size(buffer),
                    buffer->buffer_free_size(buffer));

            usleep(1000000);
        }
    }
    return NULL;
}


static int test_ringbuffer_producer_consumer()
{
    allocator_t * allocator = allocator_get_default_alloc();
    RingBuffer * buffer =   OBJECT_NEW(allocator,RingBuffer ,NULL);
    
    Thread * producer = OBJECT_NEW(allocator,Thread ,NULL);
    producer->set_start_arg(producer,buffer);
    producer->set_start_routine(producer,ringbuffer_producer);
    producer->start(producer);
    producer->detach(producer);
    
    Thread * consumer = OBJECT_NEW(allocator,Thread ,NULL);
    consumer->set_start_arg(consumer,buffer);
    consumer->set_start_routine(consumer,ringbuffer_consumer);
    consumer->start(consumer);
    consumer->detach(consumer);
    pause();
    object_destroy(buffer);
    object_destroy(producer);
    object_destroy(consumer);

    return 1;
}

REGISTER_STANDALONE_TEST_FUNC(test_ringbuffer_producer_consumer);