#ifndef __TRANACEIVER_H__
#define __TRANACEIVER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Transceiver_s Transceiver;

struct Transceiver_s {
    Obj parent;

    int (*construct)(Transceiver *,char *);
    int (*deconstruct)(Transceiver *);

    /*virtual methods reimplement*/
    int (*set)(Transceiver *module, char *attrib, void *value);
    void *(*get)(Transceiver *, char *attrib);
    char *(*to_json)(Transceiver *); 
};

#endif
