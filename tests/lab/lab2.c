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
#include <signal.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>

// int compute_slash_count(char *path)
// {
//     int i, len = strlen(path), cnt = 0;

//     for (i = 0; i < len; i++) {
//         if (path[i] == '/') {
//             cnt++;
//         }
//     }

//     return cnt;
// }

int get_lookup_keys(char *str, char *res_name, int *lynx_index, int *fxb_index)
{
    int i, len, cnt = 0, ret;

    if (str == NULL || res_name == NULL ||
        lynx_index == NULL || fxb_index == NULL) {
        return -1;
    }

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] == '/') {
            cnt++;
        }
    }
    if (cnt == 0) { return 0; }

    ret = sscanf(str,"%[a-z_0-9]/lynx_%d/fxb_%d", res_name, lynx_index, fxb_index);
    if (ret == 0) {
        return -1;
    }

    return 1;
}

int test_str_split2(TEST_ENTRY *entry)
{
    int ret = 0;
    char str[140] = "rx_iq_11/lynx_1/fxb_12";
    char name[128];
    int i, lynx_index = 0, fxb_index = 0;

    get_lookup_keys(str, name, &lynx_index, &fxb_index);
    printf("name:%s, ly:%d, fxb:%d\n", name, lynx_index, fxb_index);

    return ret;
}
REGISTER_TEST_CMD(test_str_split2);