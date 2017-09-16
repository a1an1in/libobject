/**
 * @file lab.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-10-11
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>     /* basic system data types */
#include <sys/socket.h>    /* basic socket definitions */
#include <netinet/in.h>    /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>     /* inet(3) functions */
#include <sys/epoll.h>     /* epoll function */
#include <fcntl.h>         /* nonblocking */
#include <sys/resource.h>  /*setrlimit */
#include <signal.h>
#include <sys/un.h>
#include <libobject/utils/dbg/debug.h>

typedef struct __uint8_s{
    unsigned char __pad1;
}__attribute__ ((packed)) proto_uint8_t;
typedef struct __uint16_s{
    unsigned char __pad1;
    unsigned char __pad2;
}__attribute__ ((packed)) proto_uint16_t;
typedef struct __uint32_s{
    unsigned char __pad1;
    unsigned char __pad2;
    unsigned char __pad3;
    unsigned char __pad4;
}__attribute__ ((packed)) proto_uint32_t;

/*test end: failed, c language cant convert diffent type to anthor,except pointer*/
int lab3()
{
    proto_uint8_t a8;
    proto_uint16_t a16;
    proto_uint32_t a32;

    uint8_t b8;
    uint16_t b16;
    uint32_t b32;

    uint8_t *c8;
    uint16_t *c16;
    uint32_t *c32;

    b8 = 0x12;
    b16 = 0x1234;
    b32 = 0x12345678;

    c8 = (uint8_t *)&a8;
    c16 = (uint16_t *)&a16;
    c32 = (uint32_t *)&a32;

    *c8 = b8;
    *c16 = b16;
    *c32 = b32;

    a8 = *((proto_uint8_t *)&b8);
    a16 = *((proto_uint16_t *)&b16);
    /*
     *a32 = (proto_uint32_t)b32;
     */

    /*
     *b8 = b32;
     *b32 = b8;
     */

    printf("addr:a8 =%p,a16=%p,a32=%p\n",&a8,&a16,&a32);
    printf("addr:c8 =%p,c16=%p,c32=%p\n",c8,c16,c32);
    printf("content:a8 =%x,a16=%x,a32=%x\n",*c8,*c16,*c32);

    return 0;
}
