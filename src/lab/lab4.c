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

static inline uint32_t 
pa_pow(uint32_t x,uint32_t y)
{

    uint32_t pow_value = 1;
    uint32_t i;
    
    for(i = 0;i < y; i++)
        pow_value *= x;

    return pow_value;
}

int get_index(int size, int min_data_size, int max_index)
{
    int i;
    int remainder, interger;
    int ret = -1;

    for (i = 0; i < max_index; i++) {
        interger  = size / (pa_pow(2, i) * min_data_size);
        remainder = size % (pa_pow(2, i) * min_data_size);
        if (interger == 0 || (interger == 1 && remainder == 0)) {
            return i;
        }
    }

    return ret;
}

int lab4(int argc,char **argv)
{
    int i;
    int index;

    for (i = 0; i < argc; i++) {
        dbg_str(DBG_DETAIL,"arg %d, argv:%s", i, argv[i]);
    }

    index = get_index(atoi(argv[0]), 8, 24);
    dbg_str(DBG_DETAIL,"size =%d, index=%d", atoi(argv[0]), index);

    return 0;
}
