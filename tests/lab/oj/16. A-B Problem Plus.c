/*
 * 题目描述
I have a very simple problem for you again. Given two integers A and B, your job is to calculate the result of A - B.

解答要求
时间限制：1000ms, 内存限制：64MB
输入
The first line of the input contains an integer T(1<=T<=20) which means the number of test cases.
Then T lines follow, each line consists of two positive integers, A and B. Notice that the integers are very large,that means you should not process them by using 32-bit integer.
You may assume the length of each integer will not exceed 1000.

输出
For each test case, you should output two lines. The first line is "Case #:", # means the number of the test case. The second line is the an equation "A - B = ?", ? means the result of A - B.Note there are some spaces int the equation. Output a blank line between two test cases.

样例
输入样例 1 复制

3
9 8
12 8
123456789 987654321
输出样例 1

Case 1:
9 - 8 = 1

Case 2:
12 - 8 = 4

Case 3:
123456789 - 987654321 = -864197532

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

int str_minus(char *a, char *b, char *out)
{
    int a_len, b_len, max_len, o_len;
    char c, c1, c2;
    int i;
    int minus_flag = 0;

    a_len = strlen(a);
    b_len = strlen(b);

    if (a_len < b_len || (a_len == b_len && strcmp(a, b) < 0)) {
        minus_flag = 1;
        char *t;
        t = a;
        a = b;
        b = t;

        a_len = strlen(a);
        b_len = strlen(b);
    }

    max_len = a_len > b_len ? a_len : b_len;

    for (i = 0; i < max_len; i++) {
        if (a_len >= i + 1) c1 =  a[a_len - i - 1] - '0';
        else c1 = 0;

        if (b_len >= i + 1) c2 = b[b_len - i  - 1] - '0';
        else c2 = 0;

        if (out[i] == 0) {
            if (c1 >= c2) {
                out[i] = c1 - c2;
            } else {
                out[i] = c1 - c2 + 10;
                out[i + 1] = 1;
            }
        } else if (out[i] == 1) {
            if (c1 >= c2 + out[i]) {
                out[i] = c1 - c2 - out[i];
            } else {
                out[i] = c1 - c2 + 10 - out[i];
                out[i + 1] = 1;
            }
        }
    }

    o_len = a_len + b_len;
    while (o_len > 0 && out[o_len - 1] == 0) {
        o_len--;
    }

    for (int i = 0; i < o_len; i++) {
        out[i] += '0';
    }
    if (minus_flag == 1) {
        out[o_len] = '-';
        out[o_len + 1] = '\0';
    }

    o_len = strlen(out);

    for (int i = 0; i < o_len / 2; i++) {
        c = out[i];
        out[i] = out[o_len - i - 1];
        out[o_len - i - 1] = c;
    }

}

static int test_str_minus(TEST_ENTRY *entry, void *argc, void *argv)
{
    int num = 10;
    char out[1024] = {0};
    char *a = "4321";
    char *b = "1234";
    char *expect = "3087";

    //please finish the function body here.
    str_minus(a, b, out);
    //please define the C output here. For example: printf("%d\n",a);
    printf("%s\n",  out);
    if (strcmp(expect, out) != 0) {
        return 0;
    }

    a = "1234";
    b = "4321";
    expect = "-3087";
    memset(out, 0, 1024);
    //please finish the function body here.
    str_minus(a, b, out);
    //please define the C output here. For example: printf("%d\n",a);
    printf("%s\n",  out);
    if (strcmp(expect, out) != 0) {
        return 0;
    }


    return 1;
}
REGISTER_TEST_CMD(test_str_minus)
