
#if !defined(__ASYN_RINGBUFFER_H______)
#define __ASYN_RINGBUFFER_H______

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/mutex_lock.h>
#include <libobject/io/RingBuffer.h>


typedef struct async_buffer_s AsynRingBuffer;

struct async_buffer_s{
	Stream parent;

	int (*construct)(AsynRingBuffer *,char *init_str);
	int (*deconstruct)(AsynRingBuffer *);
	int (*set)(AsynRingBuffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*async_buffer_read)(Stream *, void *dst, int len);
    int (*async_buffer_write)(Stream *, void *src, int len);
    int (*size)(AsynRingBuffer *);
    int (*is_empty)(AsynRingBuffer*);
    /*int (*printf)(AsynRingBuffer *buffer, const char *fmt, ...);
     *int (*memcopy)(AsynRingBuffer *buffer, void *addr, int len);
     */
    void (*clear)(AsynRingBuffer*);
    int (*async_buffer_move_unref)(AsynRingBuffer *,AsynRingBuffer*,size_t len);
    int (*async_buffer_copy_ref)(AsynRingBuffer*,AsynRingBuffer *);
    int (*async_buffer_copy)(AsynRingBuffer*,AsynRingBuffer *,size_t len);
    int (*async_buffer_find)(AsynRingBuffer*,u_char c);
    void (*async_buffer_destroy)(AsynRingBuffer*);
    int (*async_buffer_used_size)(AsynRingBuffer*);
    int (*async_buffer_free_size)(AsynRingBuffer*);
    
    void (*async_buffer_adapter_internal)(AsynRingBuffer*);
    int  (*async_buffer_expand_container)(AsynRingBuffer*);
    /*attribs*/
    RingBuffer * ringbuffer;
    Mutex_Lock * lock;
};


#endif // __ASYN_RINGBUFFER_H______

