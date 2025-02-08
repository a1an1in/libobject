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
        while(p < p1 && isspace(*p))
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

int strrcnt(char *s, char *end, char c)
{
    int i = 0, count = 0;

    while ((s - i) != end) {
        if (*(s - i) == c) {
            count++;
        }
        i++;
    }

    return count;
}

int str_split(char *str, char *delim, char **out, int *cnt) 
{
    int index = 0;
    char *p, *ptr = NULL;

    while((p = strtok_r(str, delim, &ptr)) != NULL) {  
        *(out + index++) = p;
        str = NULL;  
        if (ptr == NULL) break;
    }  

    return *cnt = index;
}

int str_remove_spaces_around_comma(char *str) 
{
    char *pos = str;

    while (*pos) {
        if (*pos == ',') {
            // 移动逗号前的空格
            char *start = pos - 1;
            while (start >= str && isspace((unsigned char)*start)) start--;
            // 移动逗号后的空格
            char *end = pos + 1;
            while (*end && isspace((unsigned char)*end)) end++;
            // 移动逗号后的内容到逗号位置之后
            memmove(pos + 1, end, strlen(end) + 1);
            // 如果逗号前有非空格字符，则将逗号前移
            if (start + 1 < pos) {
                memmove(start + 1, pos, strlen(pos) + 1);
                pos = start + 1; // 更新pos位置
            } else {
                pos++; // 没有非空格字符，直接移动到下一个字符
            }
        } else {
            pos++;
        }
    }

    return 1;
}