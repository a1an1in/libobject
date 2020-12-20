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

int ex(int a, int b)
{
    int d = 1;
    int i;

    for (i = 0; i < b ; i++) {
        d = d * a;
    }

    return d;
}
int hexstr_to_int(char *str)
{
    int len, i, d, t = 0, count = 0;
    char c;

    len = strlen(str);

    for (i = len - 1; i >= 0; i--) {
        printf("%d\n", i);
        c = str[i];

        if (c >= 'a' && c <= 'f') {
            d = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            d = c - 'A' + 10;
        } else {
            d = c - '0';
        }

        if (c == 'x' || c == 'X') break;

        t += ex(16, count++) * d;

        printf("len = %d count=%d e=%d d=%d\n",len,  count, ex(16, count), d);
    }

    return t;
}
int test_hex_to_int()
{
    char buf[1024];
    int d;

    while(gets(buf)) {
        d = hexstr_to_int(buf);
        printf("%d\n", d);
    }
}
REGISTER_TEST_CMD(test_hex_to_int);

char* strchr_n(char *s, char c, int n)
{
    int count = 0;

    while(*s != '\0') {
        if (*s == c) {
            count++;
        }

        if (count == n) break;

        ++s;
    }

    if (count == n) {
        return s;
    } else return NULL;
}

int test_strchr(TEST_ENTRY *entry)
{
    char *str = "lbrnsepcfjzcpfgzqdiujo";
    char *p;
    int len, i;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        p = strchr_n(str, str[i], 2);
        if (p == NULL) {
            printf("found %c", str[i]);
            break;
        }
    }

}
REGISTER_TEST_CMD(test_strchr);
