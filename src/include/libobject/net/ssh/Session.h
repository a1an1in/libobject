#ifndef __SSH_SESSION_H__
#define __SSH_SESSION_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct SSH_Session_s SSH_Session;

struct SSH_Session_s{
    Obj parent;

    int (*construct)(SSH_Session *,char *);
    int (*deconstruct)(SSH_Session *);

    /*virtual methods reimplement*/
    int (*set)(SSH_Session *module, char *attrib, void *value);
    void *(*get)(SSH_Session *, char *attrib);
    char *(*to_json)(SSH_Session *); 
};

#endif
