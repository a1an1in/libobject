#ifndef __TURN_H__
#define __TURN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include "Request.h"
#include "Response.h"

typedef struct Turn_s Turn_Client;

struct Turn_s{
    Obj parent;

    int (*construct)(Turn_Client *,char *);
    int (*deconstruct)(Turn_Client *);

    /*virtual methods reimplement*/
    int (*set)(Turn_Client *module, char *attrib, void *value);
    void *(*get)(Turn_Client *, char *attrib);
    char *(*to_json)(Turn_Client *); 
    int (*connect)(Turn_Client *turn, char *host, char *service);
    int (*send)(Turn_Client *turn);
    int (*allocate_address)(Turn_Client *turn);
    int (*set_read_post_callback)(Turn_Client *turn, int (*func)(Response *, void *arg));

    Request *req;
    Response *response;
};

#endif
