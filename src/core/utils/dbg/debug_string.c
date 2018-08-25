/*
 * =====================================================================================
 *
 *       Filename:  debug_string.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/25/2015 11:37:41 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug_string.h>

uint32_t debug_string_buf_to_str(uint8_t *buf_addr,
                                 size_t buf_len,
                                 char *str,
                                 size_t str_len)
{
    size_t i;
    size_t offset=0;

    for(i = 0; i < buf_len; i++){
        if(str_len - offset <= 4){
            break;
        }
        offset += snprintf(str+offset, str_len-offset, "%x-", buf_addr[i]);
    }
    return offset;
}

int debug_string_itoa(int val, char* buf,int radix)
{
    char* p;
    unsigned int a; //every digit
    int len;
    char* b; //start of the digit char
    char temp;
    unsigned int u;

    p = buf;
    if (val < 0) {
        *p++ = '-';
        val = 0 - val;
    }
    u = (unsigned int)val;
    b = p;
    do {
        a     = u % radix;
        u    /= radix;
        *p++  = a + '0';
    } while (u > 0);

    len = (int)(p - buf);
    *p-- = 0;

    do {
        temp = *p;
        *p   = *b;
        *b   = temp;
        --p;
        ++b;
    } while (b < p);

    return len;
}

