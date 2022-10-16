#ifndef __SSH_CLIENT_H__
#define __SSH_CLIENT_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct SSH_Client_s SSH_Client;

struct SSH_Client_s{
    Obj parent;

    int (*construct)(SSH_Client *,char *);
    int (*deconstruct)(SSH_Client *);

    /*virtual methods reimplement*/
    int (*set)(SSH_Client *module, char *attrib, void *value);
    void *(*get)(SSH_Client *, char *attrib);
    char *(*to_json)(SSH_Client *); 
};

#endif
