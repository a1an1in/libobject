#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct file_s File;

struct file_s{
	Obj obj;

	int (*construct)(File *,char *init_str);
	int (*deconstruct)(File *);
	int (*set)(File *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
