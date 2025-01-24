#ifndef __ATTACHER_H__
#define __ATTACHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/Interval_Tree.h>
#include <libobject/stub/stub.h>
#include <libobject/attacher/dynamic_lib.h>

#define ATTACHER_PARAMATER_ARRAY_MAX_SIZE 10

typedef struct Attacher_s Attacher;

typedef struct attacher_paramater_s {
    void *value;
    int size;
} attacher_paramater_t;

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
    void *(*get_remote_builtin_function_address)(Attacher *, char *func_name, char *module_name);
    void *(*get_remote_function_address)(Attacher *, char *lib_name, char *func_name);
    int (*write)(Attacher *attacher, void *addr, uint8_t *value, int len);
    int (*read)(Attacher *attacher, void *addr, uint8_t *value, int len);
    void *(*malloc)(Attacher *attacher, int size, void *value);
    int (*free)(Attacher *attacher, void *addr);
    long (*call_address_with_value_pars)(Attacher *, void *function_adress, void *paramters, int num);
    long (*call_address)(Attacher *attacher, void *function_address, attacher_paramater_t pars[], int num);
    long (*call)(Attacher *attacher, char *lib_name, char *addr, attacher_paramater_t pars[], int num);
    int (*add_lib)(Attacher *, char *name);
    int (*remove_lib)(Attacher *, char *name);
    int (*init)(Attacher *);
    stub_t *(*alloc_stub)(Attacher *);
    int (*add_stub_hooks)(Attacher *attacher, stub_t *stub, char *func, char *pre, char *new_fn, char *post, int para_count);
    int (*remove_stub_hooks)(Attacher *attacher, stub_t *stub);
    int (*free_stub)(Attacher *, stub_t *stub);
    int (*run)(Attacher *);
    int (*stop)(Attacher *);
    
    int pid;
    Map *map;
    Interval_Tree *tree;
};

#endif
