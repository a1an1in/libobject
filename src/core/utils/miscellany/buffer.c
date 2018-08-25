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
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/utils/dbg/debug.h>

void addr_to_buffer(void *addr, uint8_t *buffer)
{
    unsigned long data = (unsigned long)addr;
    int i;

    for(i = 0; i < (int)sizeof(int *); i++){
        buffer[i] = data >> 8 * (sizeof(int *) - 1 - i);
    }
}

void *buffer_to_addr(uint8_t *buffer)
{
    void *ret = NULL;
    unsigned long d = 0, t = 0;
    int i;

    for ( i = 0; i < (int)sizeof(int *); i++){
        t = buffer[i];
        d |= t << 8 * (sizeof(int *) - 1 - i); 
    }

    ret = (void *)d;

    return ret;
}

void *buffer_to_addr_wb(uint8_t *buffer, void **ret)
{
    unsigned long d = 0, t = 0;
    int i;

    for ( i = 0; i < (int)sizeof(int *); i++){
        t = buffer[i];
        d |= t << 8 * (sizeof(int *) - 1 - i); 
    }

    *ret = (void *)d;

    return (void *)d;
}

void printf_buffer(unsigned char *buf, int len)
{
#define MAX_PRINT_BUFFER_LEN 1024 *40                   
    char buffer[MAX_PRINT_BUFFER_LEN];                      
    int i;                           
    int buffer_len;

    if(len * 2 > MAX_PRINT_BUFFER_LEN){                      
        dbg_str(DBG_ERROR, "buffer too long, please check, len=%d", len);               
        return ;                      
    }                            

    memset(buffer, 0, MAX_PRINT_BUFFER_LEN);                      
    buffer_len = strlen(buffer);
    
    for(i = 0; i < len; i++) {                   
        snprintf(buffer + buffer_len , MAX_PRINT_BUFFER_LEN, "%x ", buf[i]);               
        buffer_len = strlen(buffer);
    }                           

    snprintf(buffer + buffer_len, MAX_PRINT_BUFFER_LEN, "\n");                      

    printf("%s\n", buffer);
#undef MAX_PRINT_BUFFER_LEN                    
}
