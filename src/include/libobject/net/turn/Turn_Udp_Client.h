
#ifndef __TURN_UDP_H__
#define __TURN_UDP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/net/api.h>
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
    int (*allocate_address)(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg);
    int (*create_permission)(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg);
    int (*set_read_post_callback)(Turn_Udp_Client *turn, int (*func)(Response *, void *arg));
    int (*generate_auth_code)(Turn_Udp_Client *turn, char *username, char *realm, char *password, uint8_t *out, uint32_t len);
    int (*compute_integrity)(Turn_Udp_Client *turn, uint8_t *key, uint8_t key_len, uint8_t *out, uint8_t out_len);
    int (*send_indication)(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg, uint8_t *value, int len);
    int (*wait)(Turn_Udp_Client *turn, enum turn_status_e status, int time);

    Client *c;
};

#endif
