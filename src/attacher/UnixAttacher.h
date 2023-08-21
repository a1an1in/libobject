#ifndef __UNIX_ATTACHER_H__
#define __UNIX_ATTACHER_H__

#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <libobject/core/Obj.h>
#include <libobject/attacher/dynamic_lib.h>
#include <libobject/attacher/Attacher.h>

typedef struct UnixAttacher_s UnixAttacher;

struct UnixAttacher_s {
    Attacher parent;

    int (*construct)(UnixAttacher *,char *);
    int (*deconstruct)(UnixAttacher *);

    /*virtual methods reimplement*/
    int (*set)(UnixAttacher *attacher, char *attrib, void *value);
    void *(*get)(UnixAttacher *, char *attrib);
    char *(*to_json)(UnixAttacher *); 

    int *(*attach)(UnixAttacher *, int pid);
    int *(*detach)(UnixAttacher *);
    void *(*get_function_address)(UnixAttacher *, void *local_func_address, char *module_name);
    int (*write)(UnixAttacher *attacher, void *addr, uint8_t *value, int len);
    int (*read)(UnixAttacher *attacher, void *addr, uint8_t *value, int len);
    void *(*malloc)(UnixAttacher *attacher, int size, void *value);
    int (*free)(UnixAttacher *attacher, void *addr);
    int (*set_function_pars)(UnixAttacher *attacher, struct user_regs_struct *regs, void *paramters, int num);
    long (*call_address_with_value_pars)(UnixAttacher *, void *function_adress, void *paramters, int num);
    long (*call_address)(UnixAttacher *, void *function_adress, attacher_paramater_t paramters[], int num);
    long (*call_from_lib)(UnixAttacher *, char *function_name, attacher_paramater_t paramters[], int num, char *module_name);
    int (*add_lib)(UnixAttacher *, char *name);
    int (*remove_lib)(UnixAttacher *, char *name);

    int pid;
};

#endif
