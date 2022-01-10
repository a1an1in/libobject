
#ifndef __STUN_HEADER_H__
#define __STUN_HEADER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>

#define STUN_BINDREQ    0x0001
#define STUN_BINDRESP   0x0101
#define STUN_BINDERROR  0x0111
#define STUN_SECREQ     0x0002
#define STUN_SECRESP    0x0102
#define STUN_SECERROR   0x0112

#define STUN_ATR_TYPE_MAPPED_ADDR   		0x0001
#define STUN_ATR_TYPE_RESPONSE_ADDRESS	    0x0002
#define STUN_ATR_TYPE_CHANGE_REQUEST	    0x0003
#define STUN_ATR_TYPE_SOURCE_ADDRESS	    0x0004
#define STUN_ATR_TYPE_CHANGED_ADDRESS	    0x0005
#define STUN_ATR_TYPE_USERNAME			    0x0006
#define STUN_ATR_TYPE_PASSWORD			    0x0007
#define STUN_ATR_TYPE_INTEGRITY		        0x0008
#define STUN_ATR_TYPE_ERROR_CODE			0x0009
#define STUN_ATR_TYPE_UNKNOWN_ATTRIBUTES	0x000a
#define STUN_ATR_TYPE_REFLECTED_FROM		0x000b
#define STUN_ATR_TYPE_XOR-MAPPED-ADDRESS	0x0020

typedef struct Request_s Request;

/* STUN Message Structure
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0 0|     STUN Message Type     |         Message Length        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Magic Cookie                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                     Transaction ID (96 bits)                  |
 * |                                                               |
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

typedef struct stun_attrib_s {
    unsigned short type;
    unsigned short len;
    unsigned char value[0];
} stun_attrib_t;


#endif
