#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Stream.h>

typedef struct buffer_s RingBuffer;

enum buffer_operation_flag_s{
    BUFFER_WRITE_OPERATION = 1,
    BUFFER_READ_OPERATION,
};

struct buffer_s{
	Stream parent;

	int (*construct)(RingBuffer *,char *init_str);
	int (*deconstruct)(RingBuffer *);
	int (*set)(RingBuffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*buffer_read)(Stream *, void *dst, int len);
    int (*buffer_write)(Stream *, void *src, int len);
    int (*size)(RingBuffer *);
    int (*is_empty)(RingBuffer*);
    /*int (*printf)(RingBuffer *buffer, const char *fmt, ...);
     *int (*memcopy)(RingBuffer *buffer, void *addr, int len);
     */
    void (*clear)(RingBuffer*);
    int (*buffer_move_unref)(RingBuffer *,RingBuffer*,size_t len);
    int (*buffer_copy_ref)(RingBuffer*,RingBuffer *);
    int (*buffer_copy)(RingBuffer*,RingBuffer *,size_t len);
    int (*buffer_find)(RingBuffer*,u_char c);
    void (*buffer_destroy)(RingBuffer*);
    int (*buffer_used_size)(RingBuffer*);
    int (*buffer_free_size)(RingBuffer*);
    
    void (*buffer_adapter_internal)(RingBuffer*);
    int  (*buffer_expand_container)(RingBuffer*);
    /*attribs*/
    void   *buffer;
    int    r_offset;
    int    w_offset; 
    int    used_size;      //已经使用的空间
    int    available_size; //未使用的空间
    int    capacity;           //总大小
    size_t ref_count;
    uint8_t last_operation_flag;
};

#endif
