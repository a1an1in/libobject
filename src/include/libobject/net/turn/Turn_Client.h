#ifndef __TURN_H__
#define __TURN_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/crypto/Digest.h>
#include "Request.h"
#include "Response.h"

typedef struct Turn_Client_s Turn_Client;

enum turn_status_e {
    TURN_STATUS_UNKNOWN,
    TURN_STATUS_ALLOC_ERROR,
    TURN_STATUS_PERMISION_ERROR,
    TURN_STATUS_INIT_ERROR,
    TURN_STATUS_ALLOC_SUC,
    TURN_STATUS_PERMISION_SUC,
    TURN_STATUS_INIT_SUC,
};

typedef struct turn_method_policy_s {
    int (*policy)(Response *, void *opaque);
} turn_method_policy_t;

typedef struct turn_method_map_s {
    int index;
    int type;
} turn_method_map_t;

typedef struct allocate_address_reqest_arg_s {
    uint8_t *nonce;
    int nonce_len;
    char *realm;
    char *user;
    uint32_t lifetime;
    uint8_t family;
    char *password;
} allocate_address_reqest_arg_t;


struct Turn_Client_s{
    Obj parent;

    int (*construct)(Turn_Client *,char *);
    int (*deconstruct)(Turn_Client *);

    /*virtual methods reimplement*/
    int (*set)(Turn_Client *module, char *attrib, void *value);
    void *(*get)(Turn_Client *, char *attrib);
    char *(*to_json)(Turn_Client *); 
    int (*connect)(Turn_Client *turn, char *host, char *service);
    int (*send)(Turn_Client *turn);
    int (*allocate_address)(Turn_Client *turn, allocate_address_reqest_arg_t *arg);
    int (*create_permission)(Turn_Client *turn, allocate_address_reqest_arg_t *arg);
    int (*set_read_post_callback)(Turn_Client *turn, int (*func)(Response *, void *arg));
    int (*generate_auth_code)(Turn_Client *turn, char *username, char *realm, char *password, uint8_t *out, uint32_t len);
    int (*compute_integrity)(Turn_Client *turn, uint8_t *key, uint8_t key_len, uint8_t *out, uint8_t out_len);
    int (*send_indication)(Turn_Client *turn, uint8_t *value, int len);

    Request *req;
    Response *response;
    Digest *digest;
    Digest *hmac_sha1_digest;

    char *user;
    char *realm;
    char *password;
    turn_attrib_nonce_t *nonce;
    struct addrinfo  *addr;
    uint8_t auth_code[16];
    int status;
};

int write_attrib_to_send_buffer_for_each(int index, void *element, void *arg);
int count_attrib_len_for_each(int index, void *element, void *arg);

#endif
