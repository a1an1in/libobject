
#ifndef __STUN_HEADER_H__
#define __STUN_HEADER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>

#define STUN_BINDREQ    0x0001
#define STUN_BINDRESP   0x0101
#define STUN_BINDERROR  0x0111
#define STUN_SECREQ     0x0002
#define STUN_SECRESP    0x0102
#define STUN_SECERROR   0x0112

/* STUN magic cookie (RFC 5389) */
#define STUN_MAGIC_COOKIE 0x2112A442

/* 属性头长度: 2(type) + 2(len) */
#define STUN_ATTR_HEADER_LEN 4
#define STUN_ATTR_ALIGN      4

/* RFC 3489 经典属性 */
#define STUN_ATR_TYPE_MAPPED_ADDR           0x0001
#define STUN_ATR_TYPE_RESPONSE_ADDRESS      0x0002
#define STUN_ATR_TYPE_CHANGE_REQUEST        0x0003
#define STUN_ATR_TYPE_SOURCE_ADDRESS        0x0004
#define STUN_ATR_TYPE_CHANGED_ADDRESS       0x0005
#define STUN_ATR_TYPE_USERNAME              0x0006
#define STUN_ATR_TYPE_PASSWORD              0x0007
#define STUN_ATR_TYPE_INTEGRITY             0x0008
#define STUN_ATR_TYPE_ERROR_CODE            0x0009
#define STUN_ATR_TYPE_UNKNOWN_ATTRIBUTES    0x000a
#define STUN_ATR_TYPE_REFLECTED_FROM        0x000b
/* RFC 5389 属性 */
#define STUN_ATR_TYPE_XOR_MAPPED_ADDR       0x0020
#define STUN_ATR_TYPE_PRIORITY              0x0024
#define STUN_ATR_TYPE_USE_CANDIDATE         0x0025
#define STUN_ATR_TYPE_SOFTWARE              0x8022
#define STUN_ATR_TYPE_ALTERNATE_SERVER      0x8023
#define STUN_ATR_TYPE_FINGERPRINT           0x8028
/* 兼容旧宏名（带连字符，仅历史遗留） */
#define STUN_ATR_TYPE_XOR_MAPPED_ADDRESS    STUN_ATR_TYPE_XOR_MAPPED_ADDR
#define STUN_ATR_TYPE_MAX                   0x8029

typedef struct Request_s Request;

/* STUN Message Structure
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0|     STUN Message Type     |         Message Length        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Magic Cookie                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                     Transaction ID (96 bits)                  |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct stun_header_s {
    unsigned short msgtype;
    /*
     *The message length MUST contain the size, in bytes, of the message
     *not including the 20-byte STUN header.
     */
    unsigned short msglen; 
    unsigned int magic_cookie;
    unsigned int transaction_id[3];
    unsigned char attr[0];
} stun_header_t;

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

/*
 * MAPPED-ADDRESS / XOR-MAPPED-ADDRESS / CHANGED-ADDRESS 的 value 布局：
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0 0 0 0 0 0 0|    Family     |           Port                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * family: 0x01 = IPv4, 0x02 = IPv6
 */

typedef struct mapped_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[16];
    char host[64];
    char service[16];
} mapped_address_t;

typedef struct changed_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[16];
    char host[64];
    char service[16];
} changed_address_t;

typedef struct xor_mapped_address_s {
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint8_t ip[16];
    char host[64];
    char service[16];
} xor_mapped_address_t;

typedef struct change_request_s {
    uint32_t value;
} change_request_t;

typedef struct stun_attrib_s {
    unsigned short type;
    unsigned short len;
    union {
        mapped_address_t mapped_address;
        changed_address_t changed_address;
        xor_mapped_address_t xor_mapped_address;
        change_request_t change_request;
    }u;
    unsigned char value[0];
} stun_attrib_t;



#endif
