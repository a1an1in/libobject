/**
 * @file timeval.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-20
 */
/* 
 *  Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/timeval/timeval.h>

int timeval_cmp(struct timeval *k1,struct timeval *k2)
{
    if (    k1->tv_sec > k2->tv_sec || 
            (k1->tv_sec == k2->tv_sec && k1->tv_usec > k2->tv_usec))
    {
        return 1;
    } else if (k1->tv_sec == k2->tv_sec && k1->tv_usec == k2->tv_usec) {
        return 0;
    } else {
        return -1;
    }
}

int timeval_add(struct timeval *k1,struct timeval *k2, struct timeval *r)
{
    (r)->tv_sec = (k1)->tv_sec + (k2)->tv_sec;      
    (r)->tv_usec = (k1)->tv_usec + (k2)->tv_usec;       
    if ((r)->tv_usec >= 1000000) {            
        (r)->tv_sec++;                
        (r)->tv_usec -= 1000000;          
    }                           

    return 0;
}

int timeval_sub(struct timeval *k1,struct timeval *k2, struct timeval *r)
{
    (r)->tv_sec = (k1)->tv_sec - (k2)->tv_sec;      
    (r)->tv_usec = (k1)->tv_usec - (k2)->tv_usec;   
    if ((r)->tv_usec < 0) {               
        (r)->tv_sec--;                
        (r)->tv_usec += 1000000;          
    }                           

    return 0;
}

int timeval_now(struct timeval *t, struct timezone *tz)
{
    gettimeofday(t, tz);
}

int timeval_clear(struct timeval *t)
{
    t->tv_sec = t->tv_usec = 0;
    return 0;
}

int timeval_print(struct timeval *t)
{
    dbg_str(DBG_DETAIL,"timeval tv_sec=%d, tv_usec=%d", t->tv_sec, t->tv_usec);

    return 0;
}

