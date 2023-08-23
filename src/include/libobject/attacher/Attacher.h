#ifndef __ATTACHER_H__
#define __ATTACHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>

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
    void *(*get_function_address)(Attacher *, void *local_func_address, char *module_name);
    int (*write)(Attacher *attacher, void *addr, uint8_t *value, int len);
    int (*read)(Attacher *attacher, void *addr, uint8_t *value, int len);
    void *(*malloc)(Attacher *attacher, int size, void *value);
    int (*free)(Attacher *attacher, void *addr);
    long (*call_address)(Attacher *, void *function_address, void *paramters, int num);
    long (*call_address_with_value_pars)(Attacher *, void *function_adress, attacher_paramater_t paramters[], int num);
    long (*call_from_lib)(Attacher *, char *function_name, attacher_paramater_t paramters[], int num, char *module_name);
    int (*add_lib)(Attacher *, char *name);
    int (*remove_lib)(Attacher *, char *name);

    int pid;
    Map *map;
};

#endif
