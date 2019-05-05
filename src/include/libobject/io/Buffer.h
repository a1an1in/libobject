#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Stream.h>
#include <libobject/core/String.h>

typedef struct buffer_s Buffer;

struct buffer_s{
	Stream parent;

	int (*construct)(Buffer *,char *init_str);
	int (*deconstruct)(Buffer *);
	int (*set)(Buffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*read)(Stream *, void *dst, int len);
    int (*write)(Stream *, void *src, int len);
    int (*get_len)(Stream *);
    int (*set_capacity)(Stream *, int size);
    int (*get_free_capacity)(Buffer *buffer);

    /*attribs*/
    void *addr;
    int r_offset, w_offset;
    int len;
    int capacity;
    int free_capacity;
};

#endif
