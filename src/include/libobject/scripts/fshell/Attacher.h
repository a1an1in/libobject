#ifndef __ATTACHER_H__
#define __ATTACHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Attacher_s Attacher;

struct Attacher_s {
    Obj parent;

    int (*construct)(Attacher *,char *);
    int (*deconstruct)(Attacher *);

    /*virtual methods reimplement*/
    int (*set)(Attacher *attacher, char *attrib, void *value);
    void *(*get)(Attacher *, char *attrib);
    char *(*to_json)(Attacher *); 

    int *(*attach)(Attacher *, int pid);
    int *(*detach)(Attacher *);
    void *(*get_function_address)(Attacher *, void *local_func_address, char *module_name);
    int (*write)(Attacher *attacher, void *addr, uint8_t *value, int len);
    int (*read)(Attacher *attacher, void *addr, uint8_t *value, int len);
    int (*call)(Attacher *, void *function_address, void *paramters, int num);
    int (*add_lib)(Attacher *, char *name);
    int (*remove_lib)(Attacher *, char *name);

    int pid;
};

#endif
