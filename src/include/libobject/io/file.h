#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/String.h>
#include <libobject/io/Stream.h>

typedef struct file_s File;

struct file_s{
	Stream parent;

	int (*construct)(File *,char *init_str);
	int (*deconstruct)(File *);
	int (*set)(File *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*open)(File *file, char *path, char *mode);
    int (*rename)(File *file, char *path);
    int (*read)(Stream *, void *dst, int len);
    int (*write)(Stream *, void *src, int len);
    int (*close)(File *file);

    /*attribs*/
    String *name;
    FILE *f;
};

#endif
