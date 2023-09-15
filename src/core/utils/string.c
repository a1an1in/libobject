#include <stdio.h>
#include <string.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>


long ex(long a, long b)
{
    long d = 1;
    int i;

    for (i = 0; i < b ; i++) {
        d = d * a;
    }

    return d;
}

long str_hex_to_int(char *str)
{
    long len, i, d, t = 0, count = 0;
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
    // printf("str_hex_to_int, str:%s, value:%lx\n", str, t);

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

static int test_hex_to_int()
{
    char buf[1024] = "0x123";
    int d;
    int ret, expect = 0x123;

    TRY {
        d = str_hex_to_int(buf);
        THROW_IF(d != expect, -1);
    } CATCH (ret) {
    }

    return ret;
}
REGISTER_TEST_FUNC(test_hex_to_int);

