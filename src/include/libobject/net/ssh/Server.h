#ifndef __SSH_SERVER_H__
#define __SSH_SERVER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct SSH_Server_s SSH_Server;

struct SSH_Server_s{
    Obj parent;

    int (*construct)(SSH_Server *,char *);
    int (*deconstruct)(SSH_Server *);

    /*virtual methods reimplement*/
    int (*set)(SSH_Server *module, char *attrib, void *value);
    void *(*get)(SSH_Server *, char *attrib);
    char *(*to_json)(SSH_Server *); 
};

#endif
