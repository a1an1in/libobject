/*
 * 题目描述
Given an integer N(0 ≤ N ≤ 10000), your task is to calculate N!.

解答要求
时间限制：5000ms, 内存限制：64MB
输入
One N in one line.

输出
For each N, output N! in one line.

样例
输入样例 1 复制

3
输出样例 1

6

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

static  int str_multply(char *a, char *b, char *out)
{
    int i, j, a_len, b_len, o_len;
    char c;

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

    //printf("%s\n", out);

    return  0;
}

static int factorial(int n, char *out)
{
    char a[1024*100];
    char b[1024*100] = "1";

    for (int i = 1; i <= n; i++) {
        //itoa(i, a, 10);
        sprintf(a, "%d", i);
        strcpy(b, out);
        memset(out, 0, strlen(out));
        str_multply(a, b, out);
    }

    return 0;
}

static int test_factorial(TEST_ENTRY *entry, void *argc, void *argv)
{
    int num = 10;
    char out[1024] = "1";

    //please finish the function body here.
    factorial(num, out);
    //please define the C output here. For example: printf("%d\n",a);
    printf("%s\n",  out);

    return 1;
}
REGISTER_TEST_FUNC(test_factorial);
