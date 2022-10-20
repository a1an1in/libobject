#ifndef __FSHELL_H__
#define __FSHELL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
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
    int (*run_func)(FShell *, char *func_name, int argc, char *argv[]);
    int (*open)(FShell *);
    int (*close)(FShell *);
    int (*set_prompt)(FShell *shell, char *prompt);

    Map *map;
    struct event *signal;
    int close_flag;
    char prompt[20];
};

#endif
