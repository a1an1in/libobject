/**
 * @buffer AsynRingBuffer.c
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
#include <libobject/io/Async_RingBuffer.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/thread.h>



static int __construct(AsynRingBuffer *self,char *init_str)
{
    allocator_t *allocator = ((Obj *)self)->allocator;
    self->ringbuffer = OBJECT_NEW(allocator,RingBuffer,NULL);
    if (self->ringbuffer == NULL) {
        dbg_str(DBG_ERROR,"construct Async_RingBuffer failed!");
        return -1;
    }
    self->lock =  OBJECT_NEW(allocator,Mutex_Lock,NULL);
    if (self->lock == NULL) {
        dbg_str(DBG_ERROR,"construct Mutex_Lock failed!");
        return -1;
    }
    return 0;
}

static int __deconstrcut(AsynRingBuffer *self)
{
    allocator_t *allocator = ((Obj *)self)->allocator;
    if (self->lock) {
        object_destroy(self->lock);
        self->lock = NULL;
    }
    if (self->ringbuffer) {
        object_destroy(self->ringbuffer);
        self->ringbuffer = NULL;
    }
    return 0;
}

static int __set(AsynRingBuffer *buffer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        buffer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        buffer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        buffer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        buffer->deconstruct = value;
    } else if (strcmp(attrib, "async_buffer_read") == 0) {
        buffer->async_buffer_read = value;
    } else if (strcmp(attrib, "async_buffer_write") == 0) {
        buffer->async_buffer_write = value;
    } else if (strcmp(attrib, "size") == 0) {
        buffer->size = value;
    } else if (strcmp(attrib, "is_empty") == 0) {
        buffer->is_empty = value;
    } else if (strcmp(attrib, "clear") == 0) {
        buffer->clear = value;
    } else if (strcmp(attrib, "async_buffer_move_unref") == 0) {
        buffer->async_buffer_move_unref = value;
    } else if (strcmp(attrib, "async_buffer_find") == 0) {
        buffer->async_buffer_find = value;
    } else if (strcmp(attrib, "async_buffer_used_size") == 0) {
        buffer->async_buffer_used_size = value;
    } else if (strcmp(attrib, "async_buffer_free_size") == 0) {
        buffer->async_buffer_free_size = value;
    } else if (strcmp(attrib, "async_buffer_copy") == 0) {
        buffer->async_buffer_copy = value;
    } else if (strcmp(attrib, "async_buffer_destroy") == 0) {
        buffer->async_buffer_destroy = value;
    } else if (strcmp(attrib, "async_buffer_adapter_internal") == 0) {
        buffer->async_buffer_adapter_internal = value;
    } else if (strcmp(attrib, "async_buffer_expand_container") == 0) {
        buffer->async_buffer_expand_container = value;
    } else {
        dbg_str(EV_DETAIL,"buffer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(AsynRingBuffer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"buffer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __size (AsynRingBuffer *self)
{
    return self->ringbuffer->size(self->ringbuffer);
}

static int __async_buffer_free_size(AsynRingBuffer *self)
{
    RingBuffer * ringbuffer = self->ringbuffer;
    int available_size = 0 ;
    self->lock->trylock(self->lock);
    available_size=ringbuffer->buffer_free_size(ringbuffer);
    self->lock->unlock(self->lock);
    return available_size;
} 

static int __async_buffer_used_size(AsynRingBuffer *self)
{
    RingBuffer * ringbuffer = (RingBuffer*)self->ringbuffer;
    int used_size = 0 ;
    self->lock->trylock(self->lock);
    used_size=ringbuffer->buffer_used_size(ringbuffer);
    self->lock->unlock(self->lock);
    return used_size;
}

static int __is_empty(AsynRingBuffer *self)
{   
    return self->async_buffer_used_size(self) ? 0:1;
}

static int __clear(AsynRingBuffer *self)
{
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ringbuffer->clear(ringbuffer);
    self->lock->unlock(self->lock);
    return 1;
}

//读指针 追 写指针

static  int __async_buffer_adapter_internal(AsynRingBuffer *self)
{
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ringbuffer->buffer_adapter_internal(ringbuffer);
    self->lock->unlock(self->lock);
    return 1;
}

static int __async_buffer_expand_container(AsynRingBuffer *self,int len)
{
    int ret = 0;
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ret = ringbuffer->buffer_expand_container(ringbuffer);
    self->lock->unlock(self->lock);
    return ret;
}

static int __async_buffer_read(AsynRingBuffer *self, void *dst, int len)
{
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    len = ringbuffer->buffer_read(ringbuffer,dst,len);
    self->lock->unlock(self->lock);
    return len;
}

static int __async_buffer_write(AsynRingBuffer *self, void *src, int len)
{ 
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    len = ringbuffer->buffer_write(ringbuffer,src,len);
    self->lock->unlock(self->lock);
    return len;
}

static  int __async_buffer_find(AsynRingBuffer *self,u_char c)
{
    int ret = 0;
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ret = ringbuffer->buffer_find(ringbuffer,c);
    self->lock->unlock(self->lock);
    return ret;
}

static void __async_buffer_destroy(AsynRingBuffer *self)
{
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ringbuffer->buffer_destroy(ringbuffer);
    self->lock->unlock(self->lock);  
}

static int __async_buffer_copy_ref(AsynRingBuffer *self,AsynRingBuffer *dst)
{   
    int ret ;
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ret = ringbuffer->buffer_copy_ref(ringbuffer,(RingBuffer *)dst);
    self->lock->unlock(self->lock);  
    return ret;
}

static int __async_buffer_copy(AsynRingBuffer *self,AsynRingBuffer *dst,size_t len ) 
{   
    int ret ;
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ret = ringbuffer->buffer_copy(ringbuffer,(RingBuffer *)dst,len);
    self->lock->unlock(self->lock);  
    return ret;
}

static int __async_buffer_move_unref(AsynRingBuffer *self,AsynRingBuffer*dst,size_t len)
{
    int ret ;
    RingBuffer * ringbuffer = self->ringbuffer;
    self->lock->trylock(self->lock);
    ret = ringbuffer->buffer_move_unref(ringbuffer,(RingBuffer *)dst,len);
    self->lock->unlock(self->lock);  
    return ret;
}

static class_info_entry_t buffer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Stream","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_read", __async_buffer_read,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_write", __async_buffer_write,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","size", __size,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","clear", __clear,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","is_empty", __is_empty,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_copy_ref", __async_buffer_copy_ref,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_destroy", __async_buffer_destroy,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_move_unref", __async_buffer_move_unref,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_copy", __async_buffer_copy,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_expand_container", __async_buffer_expand_container,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_adapter_internal", __async_buffer_adapter_internal,sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_used_size", __async_buffer_used_size,sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_free_size", __async_buffer_free_size,sizeof(void *)},
    [18] = {ENTRY_TYPE_VFUNC_POINTER,"","async_buffer_find", __async_buffer_find,sizeof(void *)},
    [19] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("AsynRingBuffer",buffer_class_info);

int Test_async_Ringbuffer(TEST_ENTRY *entry)
{
    AsynRingBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[9] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    char out[9];
    int len;
    char *p = "fdasfasd";
    char dst[100]={0};

    buffer = OBJECT_NEW(allocator, AsynRingBuffer, NULL);
    RingBuffer *ringbuffer = buffer->ringbuffer;

    len=buffer->async_buffer_write(buffer,p,strlen(p));
    
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

   len=buffer->async_buffer_write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    len=buffer->async_buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);
  
    
    buffer->async_buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);
    
    buffer->async_buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    memset(dst,0,100);
    buffer->async_buffer_read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    
    len=buffer->async_buffer_write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    len=buffer->async_buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    
    len=buffer->async_buffer_write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->async_buffer_used_size(buffer),
                buffer->async_buffer_free_size(buffer),
                ringbuffer->r_offset,
                ringbuffer->w_offset);

    object_destroy(buffer);

    return 1;
}
REGISTER_TEST_FUNC(Test_async_Ringbuffer);

static void* ringbuffer_producer(void * arg)
{
    AsynRingBuffer * buffer = (AsynRingBuffer *)arg;
    char * p ="pruducer ";
    int size ;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->async_buffer_write(buffer,p,strlen(p));
        
        if (size){
            dbg_str(DBG_SUC,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->async_buffer_used_size(buffer),
                    buffer->async_buffer_free_size(buffer));
            usleep(200000);
        }
        else {
            dbg_str(DBG_ERROR,"producer buffer is full");
            dbg_str(DBG_ERROR,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->async_buffer_used_size(buffer),
                    buffer->async_buffer_free_size(buffer));
            usleep(1000000);
        }
    }
    return NULL;
}

static void * ringbuffer_consumer(void *arg)
{
    AsynRingBuffer * buffer = (AsynRingBuffer *)arg;
    char tmp[1024] = {0};
    char * p ="pruducer ";
    int size;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->async_buffer_read(buffer,tmp,100);
        //usleep(10000);
        if (size){
            dbg_str(DBG_SUC,"consume buffer:%s",tmp);
            dbg_str(DBG_SUC,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->async_buffer_used_size(buffer),
                    buffer->async_buffer_free_size(buffer));

            memset(tmp,0,100);
        }  else {
            dbg_str(DBG_ERROR,"consume ringbuffer is nil ");
            dbg_str(DBG_ERROR,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->async_buffer_used_size(buffer),
                    buffer->async_buffer_free_size(buffer));

            usleep(1000000);
        }
    }
    return NULL;
}


static int test_ringbuffer_asysn_producer_consumer()
{
    allocator_t * allocator = allocator_get_default_alloc();
    AsynRingBuffer * buffer =   OBJECT_NEW(allocator,AsynRingBuffer ,NULL);
    
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

    Thread * producer1 = OBJECT_NEW(allocator,Thread ,NULL);
    producer1->set_start_arg(producer1,buffer);
    producer1->set_start_routine(producer1,ringbuffer_producer);
    producer1->start(producer1);
    producer1->detach(producer1);
    
    Thread * producer2 = OBJECT_NEW(allocator,Thread ,NULL);
    producer2->set_start_arg(producer2,buffer);
    producer2->set_start_routine(producer2,ringbuffer_producer);
    producer2->start(producer2);
    producer2->detach(producer2);


    pause();
    object_destroy(buffer);
    object_destroy(producer);
    object_destroy(producer1);
    object_destroy(producer2);
    object_destroy(consumer);
    return 1;
}

REGISTER_STANDALONE_TEST_FUNC(test_ringbuffer_asysn_producer_consumer);