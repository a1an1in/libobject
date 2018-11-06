#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct msg_publisher_s Publisher;

struct msg_publisher_s{
	Obj obj;

	int (*construct)(Publisher *,char *init_str);
	int (*deconstruct)(Publisher *);
	int (*set)(Publisher *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
