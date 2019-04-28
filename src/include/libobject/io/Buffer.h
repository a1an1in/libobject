#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Stream.h>

typedef struct buffer_s Buffer;

enum buffer_operation_flag_s{
    BUFFER_WRITE_OPERATION = 1,
    BUFFER_READ_OPERATION,
};

struct buffer_s{
	Stream parent;

	int (*construct)(Buffer *,char *init_str);
	int (*deconstruct)(Buffer *);
	int (*set)(Buffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*read)(Stream *, void *dst, int len);
    int (*write)(Stream *, void *src, int len);
    void * (*find)(Buffer *buffer, void *needle, int len);
    int (*get_len)(Stream *);
    int (*set_size)(Stream *, int size);
    int (*printf)(Buffer *buffer, const char *fmt, ...);
    int (*memcopy)(Buffer *buffer, void *addr, int len);

    /*attribs*/
    void *addr;
    int r_offset, w_offset;
    int size;
    uint8_t last_operation_flag;
};

#endif
