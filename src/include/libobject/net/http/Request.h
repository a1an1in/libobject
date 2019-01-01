#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct request_s Request;

struct request_s{
	Obj obj;

	int (*construct)(Request *,char *init_str);
	int (*deconstruct)(Request *);
	int (*set)(Request *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
