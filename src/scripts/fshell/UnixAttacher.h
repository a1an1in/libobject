#ifndef __UNIX_ATTACHER_H__
#define __UNIX_ATTACHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/scripts/fshell/Attacher.h>

typedef struct UnixAttacher_s UnixAttacher;

struct UnixAttacher_s{
    Attacher parent;

    int (*construct)(UnixAttacher *,char *);
    int (*deconstruct)(UnixAttacher *);

    /*virtual methods reimplement*/
    int (*set)(UnixAttacher *attacher, char *attrib, void *value);
    void *(*get)(UnixAttacher *, char *attrib);
    char *(*to_json)(UnixAttacher *); 

    int *(*attach)(UnixAttacher *, int pid);
    int *(*detach)(UnixAttacher *);
    int (*call)(UnixAttacher *, char *function_name, void *paramters, int num);
    int (*add_lib)(UnixAttacher *, char *name);
    int (*remove_lib)(UnixAttacher *, char *name);

    int pid;
};

#endif
