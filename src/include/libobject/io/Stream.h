#ifndef __STREAM_H__
#define __STREAM_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct stream_s Stream;

struct stream_s{
	Obj obj;

	int (*construct)(Stream *,char *init_str);
	int (*deconstruct)(Stream *);
	int (*set)(Stream *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*buffer_read)(Stream *, void *dst, int len);
    int (*buffer_write)(Stream *, void *src, int len);
};

#endif
