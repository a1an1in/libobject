#ifndef __FSHELL_H__
#define __FSHELL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/Vector.h>
#include <libobject/concurrent/event/event_compat.h>

typedef struct FShell_s FShell;

struct FShell_s{
    Obj parent;

    int (*construct)(FShell *,char *);
    int (*deconstruct)(FShell *);

    /*virtual methods reimplement*/
    int (*set)(FShell *, char *attrib, void *value);
    void *(*get)(FShell *, char *attrib);
    int (*load)(FShell *, char *lib_name, int);
    int (*unload)(FShell *, char *lib_name);
    int (*get_func_addr)(FShell *, char *lib_name, char *func_name, void **);
    int (*get_func_name)(FShell *, char *lib_name, void *func_addr, char *name, unsigned int name_len);
    int (*run_func)(FShell *, String *);
    int (*open_ui)(FShell *);
    int (*is_statement)(FShell *, char *);
    int (*set_prompt)(FShell *shell, char *prompt);
    int (*init)(FShell *shell);

    Map *map;
    struct event *signal;
    int close_flag;
    char prompt[20];
};

typedef int (*fshell_func_t)(void * p1, void * p2, void * p3, void * p4, void * p5, 
                             void * p6, void * p7, void * p8, void * p9, void * p10,
                             void * p11, void * p12, void * p13, void * p14, void * p15, 
                             void * p16, void * p17, void * p18, void * p19, void * p20);

#endif
