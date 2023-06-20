#ifndef __PIPE_H__
#define __PIPE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/Stream.h>
#include <libobject/core/String.h>

typedef struct Pipe_s Pipe;

struct Pipe_s{
    Obj parent;

    int (*construct)(Pipe *,char *init_str);
    int (*deconstruct)(Pipe *);
    int (*set)(Pipe *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*init)(Pipe *pipe);
    int (*read)(Pipe *, void *dst, int len);
    int (*write)(Pipe *, void *src, int len);

    /*attribs*/
};

#endif
