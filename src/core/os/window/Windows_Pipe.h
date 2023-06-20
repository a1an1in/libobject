#ifndef __WINDOWS_PIPE_H__
#define __WINDOWS_PIPE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/Pipe.h>
#include <windows.h>
#include <libobject/core/String.h>

typedef struct Windows_Pipe_s Windows_Pipe;

struct Windows_Pipe_s{
    Pipe parent;

    int (*construct)(Windows_Pipe *,char *init_str);
    int (*deconstruct)(Windows_Pipe *);
    int (*set)(Windows_Pipe *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*init)(Windows_Pipe *pipe);
    int (*read)(Windows_Pipe *, void *dst, int len);
    int (*write)(Windows_Pipe *, void *src, int len);

    HANDLE read_handle;
    HANDLE write_handle;
};

#endif
