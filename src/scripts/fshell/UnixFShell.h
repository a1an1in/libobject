#ifndef __FSHELLUNIX_H__
#define __FSHELLUNIX_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/scripts/FShell.h>

typedef struct UnixFShell_s UnixFShell;

struct UnixFShell_s{
    FShell parent;

    int (*construct)(UnixFShell *,char *);
    int (*deconstruct)(UnixFShell *);

    /*virtual methods reimplement*/
    int (*set)(UnixFShell *, char *attrib, void *value);
    void *(*get)(UnixFShell *, char *attrib);
    int (*load)(UnixFShell *, char *lib_name, int);
    int (*unload)(UnixFShell *, char *lib_name);
    int (*get_func_addr)(UnixFShell *, char *lib_name, char *func_name, void **);
    int (*get_func_name)(UnixFShell *, char *lib_name, void *func_addr, char *name, unsigned int name_len);
    int (*run_func)(UnixFShell *, char *func_name, int argc, char *argv[]);
    int (*open)(UnixFShell *);
    int (*close)(UnixFShell *);

};

#endif
