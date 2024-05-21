#include <stdio.h>
#include <string.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/dbg/debug.h>

long long ex(long long a, long long b)
{
    uint64_t d = 1;
    int i;

    for (i = 0; i < b ; i++) {
        d = d * a;
    }

    return d;
}

long long str_hex_to_integer(char *str)
{
    long long len, i, d, t = 0, count = 0;
    char c;

    len = strlen(str);

    for (i = len - 1; i >= 0; i--) {
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
    }
    // printf("str_hex_to_integer, str:%s, value:%llx\n", str, t);

    return t;
}

char *str_trim(char *str)
{
    char *p = str;
    char *p1;
    if(p)
    {
        p1 = p + strlen(str) - 1;
        while(*p && isspace(*p))
            p++;
        while(p1 > p && isspace(*p1))
            *p1--=0;
    }
    return p;
}

/* 找第二个字符 */
char* strchr_n(char *s, char c, int n)
{
    int count = 0;

    while (*s != '\0') {
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


