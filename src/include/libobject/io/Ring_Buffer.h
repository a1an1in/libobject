#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/io/Stream.h>
#include <libobject/core/String.h>
#include <libobject/io/Buffer.h>

typedef struct ring_buffer_s Ring_Buffer;

enum ring_buffer_operation_flag_s{
    BUFFER_WRITE_OPERATION = 1,
    BUFFER_READ_OPERATION,
};

struct ring_buffer_s{
    Stream parent;

    int (*construct)(Ring_Buffer *,char *init_str);
    int (*deconstruct)(Ring_Buffer *);
    int (*set)(Ring_Buffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*read)(Ring_Buffer *, void *dst, int len);
    int (*read_to_string)(Ring_Buffer *, String *str, int len);
    int (*read_to_buffer)(Ring_Buffer *rb, Buffer *buffer, int len);
    int (*write)(Ring_Buffer *, void *src, int len);
    void *(*find)(Ring_Buffer *buffer, void* needle, int needle_len, int len);
    int (*get_needle_offset)(Ring_Buffer *buffer, void *needle, int needle_len, int len);
    int (*get_len)(Ring_Buffer *);
    int (*set_size)(Ring_Buffer *, int size);
    int (*printf)(Ring_Buffer *buffer, const char *fmt, ...);
    int (*memcopy)(Ring_Buffer *buffer, void *addr, int len);

    /*attribs*/
    void *addr;
    int r_offset, w_offset;
    int size;
    uint8_t last_operation_flag;
};

#endif
