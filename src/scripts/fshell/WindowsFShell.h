#ifndef __FSHELLUNIX_H__
#define __FSHELLUNIX_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/scripts/fshell/FShell.h>

typedef struct WindowsFShell_s WindowsFShell;

struct WindowsFShell_s {
    FShell parent;

    int (*construct)(WindowsFShell *,char *);
    int (*deconstruct)(WindowsFShell *);

    /*virtual methods reimplement*/
    int (*set)(WindowsFShell *, char *attrib, void *value);
    void *(*get)(WindowsFShell *, char *attrib);
    int (*load)(WindowsFShell *, char *lib_name, int);
    int (*unload)(WindowsFShell *, char *lib_name);
    int (*get_func_addr)(WindowsFShell *, char *lib_name, char *func_name, void **);
    int (*get_func_name)(WindowsFShell *, char *lib_name, void *func_addr, char *name, unsigned int name_len);
    int (*run_func)(WindowsFShell *, String *s);
    int (*open_ui)(WindowsFShell *);
    int (*is_statement)(WindowsFShell *, char *);
};

#endif
