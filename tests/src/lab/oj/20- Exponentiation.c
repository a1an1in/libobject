/*
 * 题目描述
Problems involving the computation of exact values of very large magnitude and precision are common. For example, the computation of the national debt is a taxing experience for many computer systems.
This problem requires that you write a program to compute the exact value of Rn where R is a real number ( 0.0 < R < 99.999 ) and n is an integer such that 0 < n ≤ 25.

解答要求
时间限制：4999ms, 内存限制：64MB
输入
The input will consist of a set of pairs of values for R and n. The R value will occupy columns 1 through 6, and the n value will be in columns 8 and 9.

输出
The output will consist of one line for each line of input giving the exact value of Rn. Leading zeros should be suppressed in the output. Insignificant trailing zeros must not be printed. Don't print the decimal point if the result is an integer.

样例
输入样例 1 复制

95.123 12
0.4321 20
5.1234 15
6.7592  9
98.999 10
1.0100 12
输出样例 1

548815620517731830194541.899025343415715973535967221869852721
.00000005148554641076956121994511276767154838481760200726351203835429763013462401
43992025569.928573701266488041146654993318703707511666295476720493953024
29448126.764121021618164430206909037173276672
90429072743629540498.107596019456651774561044010001
1.126825030131969720661201

提示
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/registry/registry.h>

static int del_decimal_point(char *a)
{
    int len, count = 0;

    len = strlen(a);

    while (--len >= 0) {
        if (a[len] == '.') {
            break;
        }
        count++;
    }

    len = strlen(a);
    if (count > 0) {
        int i = 0;
        while (i < count) {
            a[len - count - 1 + i] = a[len - count + i];
            i++;
        }
        a[len - 1] = '\0';
    }

    return count;
}
static int add_decimal_point(char *a, int count)
{
    int len, i = 0;

    len = strlen(a);

    if (count <= 0) return 0;

    a[len + 1] = '\0';

    len = strlen(a);

    while (i < count) {
        a[len - i] = a[len - i - 1];
        i++;
    }
    a[len - i] = '.';

    return count;
}

static int str_multply_with_dot(char *a_in, char *b_in, char *out)
{
    int i, j, a_len, b_len, o_len;
    char c;
    int decimal_count = 0;
    char *a, *b;

    a = malloc(strlen(a_in) + 1);
    b = malloc(strlen(b_in) + 1);

    strcpy(a, a_in);
    strcpy(b, b_in);
    decimal_count = del_decimal_point(a);
    decimal_count += del_decimal_point(b);

    a_len = strlen(a);
    b_len = strlen(b);

    for (i = 0; i < a_len; i++) {
        for (j = 0; j < b_len; j++) {
            out[i + j] += (a[a_len - i - 1] - '0') * (b[b_len - j - 1] - '0');
            out[i + j + 1] += out[i + j] / 10;
            out[i + j] = out[i + j] % 10;
        }
    }

    o_len = strlen(out);
    if (out[a_len + b_len - 1] == 0) {
        o_len = a_len + b_len - 1;
    } else {
        o_len = a_len + b_len;
    }
    for (i = 0; i < o_len; i++) {
        out[i] += '0';
    }

    for (i = 0; i < o_len / 2; i++) {
        c = out[i];
        out[i] = out[o_len - i - 1];
        out[o_len - i - 1] = c;
    }

    add_decimal_point(out, decimal_count);

    if (decimal_count > 0) {
        o_len = strlen(out);
        for (i = 0; i < o_len; i++) {
            if (out[o_len - i - 1] == '0') {
                out[o_len - i - 1] = '\0';
            } else {
                break;
            }
        }
    }

    //printf("%s\n", out);
    free(a);
    free(b);

    return  0;
}

static int test_str_multply_with_dot(TEST_ENTRY *entry, void *argc, void *argv)
{
    char out[1024] = {0};
    char *str1 = "123.456789";
    char *str2 = "987.654321";
    char *expect = "121932.631112635269";

    str_multply_with_dot(str1, str2, out);
    if (strcmp(out, expect) == 0) {
        //return 1;
    } else {
        return 0;
    }

    char *data = "95.123";
    char tmp[1024] = {0};
    expect = "548815620517731830194541.899025343415715973535967221869852721";
    int num = 11;

    strcpy(out, data);
    while (num > 0) {
        strcpy(tmp, out);
        memset(out, 0, 1024);
        str_multply_with_dot(data, tmp, out);
        num--;
    }

    printf("%s\n", out);
    if (strcmp(out, expect) == 0) {
        return 1;
    } else {
        return 0;
    }

}
REGISTER_TEST_FUNC(test_str_multply_with_dot);
