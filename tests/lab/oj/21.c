/*
 * 题目描述
为什么1小时有60分钟，而不是100分钟呢？这是历史上的习惯导致。但也并非纯粹的偶然：60是个优秀的数字，它的因子比较多。事实上，它是1至6的每个数字的倍数。即1,2,3,4,5,6都是可以除尽60。
我们希望寻找到能除尽1至n的的每个数字的最小整数。

解答要求
时间限制：6000ms, 内存限制：64MB
输入
输入一个整数n,(1<n<=100），测试包含组样例，读到文件末尾结束。

输出
输出求1至n的最小公倍数。

样例
输入样例 1 复制

6
10
100
输出样例 1

60
2520
69720375229712477164533808935312303556800

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

    return  0;
}

static int str_common_multiple_get_factor_raw(int *array, int *index, int from, int to)
{
    int i, count, d, t;

    if (from > to) {
        return 1;
    }

    t = from;
    count = *index;
    for (i = 0; i < count; i++) {
        d = array[i];
        if (t % d == 0) {
            t = t / d;
        }
    }
    if (t != 1) {
        array[count] = t;
        (*index) += 1;
    }

    str_common_multiple_get_factor_raw(array, index, from + 1, to);
}

static int str_common_multiple_1_to_n_raw(int n, char *out)
{
    int ret;
    int array[1024], index = 0;
    char str1[1024], str2[1024];

    str_common_multiple_get_factor_raw(array, &index, 1, n);
    sprintf(out, "%d", array[0]);

    for (int i = 1; i < index; i++) {
        memset(str1, 0, 1024);
        sprintf(str1, "%d", array[i]);
        strcpy(str2, out);
        memset(out, 0, strlen(out));
        str_multply(str1, str2, out);
    }

    printf("%s\n", out);

}

static int test_str_common_multiple_1_to_n_raw_method(TEST_ENTRY *entry, void *argc, void *argv)
{
    int num = 6;
    char out[1024] = {0};

    str_common_multiple_1_to_n_raw(num, out);

    return 1;
}
REGISTER_TEST_CMD(test_str_common_multiple_1_to_n_raw_method)
