
#ifndef __TURN_HEADER_H__
#define __TURN_HEADER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>

#define TURN_METHOD_BINDREQ                             0x0001
#define TURN_METHOD_SECREQ                              0x0002
#define TURN_METHOD_ALLOCATE                            0x0003
#define TURN_METHOD_REFRESH                             0x0004
#define TURN_METHOD_SEND                                0x0006   
#define TURN_METHOD_DATA                                0x0007   
#define TURN_METHOD_CREATEPERMISSION                    0x0008   
#define TURN_METHOD_CHANNELBIND                         0x0009    
#define TURN_METHOD_BINDRESP                            0x0101
#define TURN_METHOD_SECRESP                             0x0102
#define TURN_METHOD_BINDERROR                           0x0111
#define TURN_METHOD_SECERROR                            0x0112

#define TURN_ATTR_TYPE_MAPPED_ADDR                      0x0001
#define TURN_ATTR_TYPE_RESPONSE_ADDRESS	                0x0002
#define TURN_ATTR_TYPE_CHANGE_REQUEST	                0x0003
#define TURN_ATTR_TYPE_SOURCE_ADDRESS	                0x0004
#define TURN_ATTR_TYPE_CHANGED_ADDRESS	                0x0005
#define TURN_ATTR_TYPE_USERNAME			                0x0006
#define TURN_ATTR_TYPE_PASSWORD			                0x0007
#define TURN_ATTR_TYPE_INTEGRITY		                0x0008
#define TURN_ATTR_TYPE_ERROR_CODE		               	0x0009
#define TURN_ATTR_TYPE_UNKNOWN_ATTRIBUTES            	0x000a
#define TURN_ATTR_TYPE_REFLECTED_FROM	               	0x000b
#define TURN_ATTR_TYPE_CHANNEL_NUMBER                   0x000C 
#define TURN_ATTR_TYPE_LIFETIME                         0x000D 
#define TURN_ATTR_TYPE_XOR_PEER_ADDRESS                 0x0012 
#define TURN_ATTR_TYPE_DATA                             0x0013 
#define TURN_ATTR_TYPE_REALM                            0x0014 
#define TURN_ATTR_TYPE_NONCE                            0x0015 
#define TURN_ATTR_TYPE_XOR_RELAYED_ADDRESS              0x0016 
#define TURN_ATTR_TYPE_REQUESTED_FAMILY                 0x0017 
#define TURN_ATTR_TYPE_EVEN_PORT                        0x0018 
#define TURN_ATTR_TYPE_REQUESTED_TRANSPORT              0x0019 
#define TURN_ATTR_TYPE_DONT_FRAGMENT                    0x001A 
#define TURN_ATTR_TYPE_XOR_MAPPED_ADDRESS               0x0020
#define TURN_ATTR_TYPE_Reserved                         0x0021 
#define TURN_ATTR_TYPE_RESERVATION_TOKEN                0x0022 
#define TURN_ATTR_TYPE_SOFTWARE                         0x8022 
#define TURN_ATTR_TYPE_FINGERPRINT                      0x8028 

typedef struct Request_s Request;

/* TURN Message Structure
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0|     TURN Message Type     |         Message Length        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Magic Cookie                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                     Transaction ID (96 bits)                  |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct turn_header_s {
    unsigned short msgtype;
    /*
     *The message length MUST contain the size, in bytes, of the message
     *not including the 20-byte TURN header.
     */
    unsigned short msglen; 
    unsigned int magic_cookie;
    unsigned int transaction_id[3];
    unsigned char attr[0];
} turn_header_t;

/*
 * Message Attributes
 *
 * After the header are 0 or more attributes.  Each attribute is TLV
 * encoded, with a 16 bit type, 16 bit length, and variable value:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         Type                  |            Length             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             Value                             ....
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct turn_attrib_header_s {
    unsigned short type;
    unsigned short len;
} turn_attrib_header_t;

typedef struct mapped_address_s {
    unsigned short type;
    unsigned short len;
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    union {
        uint8_t ipv4[4];
        uint8_t ipv6[16];
    } u;
} turn_attrib_mapped_address_t;

typedef struct changed_address_s {
    unsigned short type;
    unsigned short len;
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    union {
        uint8_t ipv4[4];
        uint8_t ipv6[16];
    } u;
} turn_attrib_changed_address_t;

typedef struct xor_mapped_address_s {
    unsigned short type;
    unsigned short len;
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    union {
        uint8_t ipv4[4];
        uint8_t ipv6[16];
    } u;
} turn_attrib_xor_mapped_address_t;

typedef struct nonce_s {
    unsigned short type;
    unsigned short len;
    uint8_t value[0];
} turn_attrib_nonce_t;

typedef struct lifetime_s {
    unsigned short type;
    unsigned short len;
    uint32_t value;
} turn_attrib_lifetime_t;

typedef struct change_request_s {
    unsigned short type;
    unsigned short len;
    uint32_t value;
} turn_attrib_change_request_t;

typedef struct requested_transport_s {
    unsigned short type;
    unsigned short len;
    uint8_t protocol;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
} turn_attrib_requested_transport_t;

typedef struct error_code_s {
    unsigned short type;
    unsigned short len;
    uint16_t reserved;
    uint8_t class_bits:3;
    uint8_t number_bits;
    uint8_t reason[0];
} turn_attrib_error_code_t;

typedef struct realm_s {
    unsigned short type;
    unsigned short len;
    uint8_t value[0];
} turn_attrib_realm_t;

typedef struct software_s {
    unsigned short type;
    unsigned short len;
    uint8_t value[0];
} turn_attrib_software_t;

typedef struct turn_attrib_fingerprint_s {
    unsigned short type;
    unsigned short len;
    int crc32;
} turn_attrib_fingerprint_t;

typedef struct turn_attrib_requested_family_s {
    unsigned short type;
    unsigned short len;
    uint32_t family:8;
    uint32_t reserved:24;
} turn_attrib_requested_family_t;

typedef struct turn_attrib_username_s {
    unsigned short type;
    unsigned short len;
    uint8_t value[0];
} turn_attrib_username_t;

typedef struct turn_attribs_s {
    turn_attrib_mapped_address_t *mapped_address;
    turn_attrib_changed_address_t *changed_address;
    turn_attrib_xor_mapped_address_t *xor_mapped_address;
    turn_attrib_change_request_t *change_request;
    turn_attrib_nonce_t *nonce;
    turn_attrib_lifetime_t *lifetime;
    turn_attrib_requested_transport_t *requested_transport;
    turn_attrib_error_code_t *error_code;
    turn_attrib_realm_t *realm;
    turn_attrib_software_t *software;
    turn_attrib_fingerprint_t *fingerprint;
} turn_attribs_t;

typedef struct attrib_parse_policy_s {
    int (*policy)(turn_attribs_t *attribs, uint8_t *addr);
} attrib_parse_policy_t;


typedef struct attrib_type_map_s {
    int index;
    int type;
} attrib_type_map_t;

attrib_parse_policy_t *turn_get_parser_policies();

int turn_get_parser_policy_index(int type);
int turn_set_attrib_requested_transport(turn_attrib_requested_transport_t *attrib, uint8_t protocol);
int turn_set_attrib_nonce(turn_attrib_nonce_t *attrib, uint8_t attr_len, uint8_t *nonce, uint8_t nonce_len);
int turn_set_attrib_realm(turn_attrib_realm_t *attrib, uint8_t attr_len, uint8_t *realm, uint8_t realm_len);
int turn_set_attrib_username(turn_attrib_username_t *attrib, uint8_t attr_len, uint8_t *username, uint8_t username_len);
int turn_set_attrib_lifetime(turn_attrib_lifetime_t *attrib, uint32_t lifetime);
int turn_set_attrib_requested_family(turn_attrib_requested_family_t *attrib, uint8_t family);

#endif
