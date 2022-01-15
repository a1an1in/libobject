
#ifndef __TURN_HEADER_H__
#define __TURN_HEADER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>

#define TURN_METHOD_BINDREQ    0x0001
#define TURN_METHOD_BINDRESP   0x0101
#define TURN_METHOD_BINDERROR  0x0111
#define TURN_METHOD_SECREQ     0x0002
#define TURN_METHOD_SECRESP    0x0102
#define TURN_METHOD_SECERROR   0x0112
#define TURN_METHOD_ALLOCATE   0x0003

#define TURN_ATR_TYPE_MAPPED_ADDR   	               	0x0001
#define TURN_ATR_TYPE_RESPONSE_ADDRESS	                0x0002
#define TURN_ATR_TYPE_CHANGE_REQUEST	                0x0003
#define TURN_ATR_TYPE_SOURCE_ADDRESS	                0x0004
#define TURN_ATR_TYPE_CHANGED_ADDRESS	                0x0005
#define TURN_ATR_TYPE_USERNAME			                0x0006
#define TURN_ATR_TYPE_PASSWORD			                0x0007
#define TURN_ATR_TYPE_INTEGRITY		                    0x0008
#define TURN_ATR_TYPE_ERROR_CODE		               	0x0009
#define TURN_ATR_TYPE_UNKNOWN_ATTRIBUTES               	0x000a
#define TURN_ATR_TYPE_REFLECTED_FROM	               	0x000b
#define TURN_ATR_TYPE_XOR_MAPPED_ADDRESS               	0x0020
#define TURN_ATR_TYPE_CHANNEL_NUMBER                    0x000C 
#define TURN_ATR_TYPE_LIFETIME                          0x000D 
#define TURN_ATR_TYPE_Reserved (was BANDWIDTH)          0x0010 
#define TURN_ATR_TYPE_XOR_PEER_ADDRESS                  0x0012 
#define TURN_ATR_TYPE_DATA                              0x0013 
#define TURN_ATR_TYPE_XOR_RELAYED_ADDRESS               0x0016 
#define TURN_ATR_TYPE_EVEN_PORT                         0x0018 
#define TURN_ATR_TYPE_REQUESTED_TRANSPORT               0x0019 
#define TURN_ATR_TYPE_DONT_FRAGMENT                     0x001A 
#define TURN_ATR_TYPE_Reserved (was TIMER_VAL)          0x0021 
#define TURN_ATR_TYPE_RESERVATION_TOKEN                 0x0022 
#define TURN_ATR_TYPE_MAX	                            0x0023


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

typedef struct mapped_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[8];
    char host[32];
    char service[8];
} mapped_address_t;

typedef struct changed_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[8];
    char host[32];
    char service[8];
} changed_address_t;

typedef struct xor_mapped_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[8];
    char host[32];
    char service[8];
} xor_mapped_address_t;

typedef struct nonce_s {
    uint8_t value[0];
} nonce_t;

typedef struct lifetime_s {
    uint32_t value;
} lifetime_t;

typedef struct change_request_s {
    uint32_t value;
} change_request_t;

typedef struct requested_transport_s {
    uint8_t protocol;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
} requested_transport_t;

typedef struct turn_attrib_s {
    unsigned short type;
    unsigned short len;
    union {
        mapped_address_t mapped_address;
        changed_address_t changed_address;
        xor_mapped_address_t xor_mapped_address;
        change_request_t change_request;
        nonce_t nonce;
        lifetime_t lifetime;
        requested_transport_t requested_transport;
    }u;
    unsigned char value[0];
} turn_attrib_t;


typedef struct attrib_parse_policy_s {
    int (*policy)(turn_attrib_t *, turn_attrib_t *);
} attrib_parse_policy_t;


attrib_parse_policy_t *protocol_get_parse_policies();
#endif
