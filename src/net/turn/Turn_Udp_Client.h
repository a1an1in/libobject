
#ifndef __TURN_UDP_H__
#define __TURN_UDP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/net/worker/Client.h>
#include "Turn_Client.h"

typedef struct Turn_Udp_s Turn_Udp_Client;

struct Turn_Udp_s{
    Turn_Client parent;

    int (*construct)(Turn_Udp_Client *,char *);
    int (*deconstruct)(Turn_Udp_Client *);

    /*virtual methods reimplement*/
    int (*set)(Turn_Udp_Client *, char *attrib, void *value);
    void *(*get)(Turn_Udp_Client *, char *attrib);
    char *(*to_json)(Turn_Udp_Client *); 
    int (*connect)(Turn_Udp_Client *turn, char *host, char *service);
    int (*send)(Turn_Udp_Client *turn);
    int (*allocate_address)(Turn_Udp_Client *turn);
    int (*set_read_post_callback)(Turn_Udp_Client *turn, int (*func)(Response *, void *arg));

    Client *c;
};

#endif