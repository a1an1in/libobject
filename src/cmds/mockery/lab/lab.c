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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>


int check_endian( )
{
    union w {  
        int a;
        char b;
    } c;
    c.a = 1;
    return(c.b ==1);
}

static int test_check_endian(TEST_ENTRY *entry, void *argc, void *argv)
{
    if(check_endian()) {
        dbg_str(DBG_DETAIL,"little endian");
    } else {
        dbg_str(DBG_DETAIL,"big endian");
    }

    return 1;
}
REGISTER_TEST_CMD(test_check_endian);

static int test_rand(TEST_ENTRY *entry)
{
    int ret;

    ret = rand();
    printf("rand:%d\n", ret);

    return 1;
}
REGISTER_TEST_CMD(test_rand);

//#define cgs_lambda( return_type, function_body) \
//    ({return_type cgs_lambda_func function_body cgs_lambda_func;})
//
//static int test_lambda(TEST_ENTRY *entry)
//{
//    printf( "Sum = %d\n", cgs_lambda(int, (int x, int y){return x + y;})(3, 4));
//
//    return 0;
//}
//REGISTER_TEST_CMD(test_lambda);
