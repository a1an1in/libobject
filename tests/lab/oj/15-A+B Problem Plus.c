/*
 * 题目描述
I have a very simple problem for you. Given two integers A and B, your job is to calculate the Sum of A + B.

解答要求
时间限制：1000ms, 内存限制：64MB
输入
The first line of the input contains an integer T(1≤T≤20) which means the number of test cases.
Then T lines follow, each line consists of two positive integers, A and B. Notice that the integers are very large,that means you should not process them by using 32-bit integer.You may assume the length of each integer will not exceed 1000.

输出
For each test case, you should output two lines. The first line is "Case #:", # means the number of the test case. The second line is the an equation "A + B = Sum", Sum means the result of A + B.Note there are some spaces int the equation. Output a blank line between two test cases.

样例
输入样例 1 复制

2
1 2
112233445566778899 998877665544332211
输出样例 1

Case 1:
1 + 2 = 3

Case 2:
112233445566778899 + 998877665544332211 = 1111111111111111110

提示
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/mockery/mockery.h>

int str_plus(char *a, char *b, char *out)
{
    int a_len, b_len, max_len, o_len;
    char c, c1, c2;

    a_len = strlen(a);
    b_len = strlen(b);

    max_len = a_len > b_len ? a_len : b_len;

    for (int i = 0; i < max_len; i++) {
        if (a_len >= i + 1) c1 =  a[a_len - i - 1] - '0';
        else c1 = 0;

        if (b_len >= i + 1) c2 = b[b_len - i  - 1] - '0';
        else c2 = 0;

        out[i] += (c1 + c2);
        out[i + 1] += out[i] / 10;
        out[i] = out[i] % 10;
        //out[i] += '0';
    }

    o_len = a_len + b_len;

    while (o_len > 0 && out[o_len - 1] == 0) {
        o_len--;
    }

    for (int i = 0; i < o_len; i++) {
        out[i] += '0';
    }

    for (int i = 0; i < o_len / 2; i++) {
        c = out[i];
        out[i] = out[o_len - i - 1];
        out[o_len - i - 1] = c;
    }

}

static int test_str_plus(TEST_ENTRY *entry, void *argc, void *argv)
{
    int num = 10;
    char out[1024] = {0};
    char *a = "1234";
    char *b = "4321";

    char *a1 = "112233445566778899";
    char *b1 = "998877665544332211";
    //please finish the function body here.
    str_plus(a, b, out);
    //please define the C output here. For example: printf("%d\n",a);
    printf("%s\n",  out);

    memset(out, 0, 1024);
    str_plus(a1, b1, out);
    //please define the C output here. For example: printf("%d\n",a);
    printf("%s\n",  out);

    return 1;
}
REGISTER_TEST_CMD(test_str_plus)
