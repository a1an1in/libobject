/*
 *
 * 题目描述
I have a very simple problem for you. Given two integers A and B, your job is to multiply the product is of A × B.

解答要求
时间限制：1000ms, 内存限制：64MB
输入
Each line will contain two integers A and B. Process to end of file.Notice that the integers are very large,that means you should not process them by using 32-bit integer.
You may assume the length of each integer will not exceed 1000.

输出
For each case, output the product is of A × B in one line.

样例
输入样例 1 复制

1 2
3 11
14512451451245124512 15125125124512451245
输出样例 1

2
33
219502644063494817653152060344354417440

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

static int str_multply(char *a, char *b, char *out)
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

    o_len = a_len + b_len;
    while (o_len > 0 && out[o_len - 1] == '\0') {
        o_len--;
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

static int test_str_multply(TEST_ENTRY *entry, void *argc, void *argv)
{

    char out[1024] = {0};
    char *str1 = "123456789";
    char *str2 = "987654321";
    char *expect = "121932631112635269";

    str_multply(str1, str2, out);
    if (strcmp(out, expect) == 0) {
        return 1;
    } else {
        return 0;
    }

    str1 = "5";
    str2 = "2";
    expect = "120";
    memset(out, 0 , strlen(out));
    str_multply(str1, str2, out);
    if (strcmp(out, expect) == 0) {
        return 1;
    } else {
        return 0;
    }

}
REGISTER_TEST_FUNC(test_str_multply);
