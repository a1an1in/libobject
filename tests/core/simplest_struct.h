#ifndef __SIMPLEST_STR_H__
#define __SIMPLEST_STR_H__

#include <stdio.h>
#include <libobject/core/String.h>

typedef struct simplest_struct_s simplest_struct;

struct simplest_struct_s {
    int type;
    int len;
    char *name;
};

#endif