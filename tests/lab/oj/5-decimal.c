/*
 * 题目描述
编写程序实现将任意10进制正小数m转换成n进制的正小数，小数点后保留10位小数。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入包含两个数m,n，用空格隔开。输入包含多组测试，当m,n都为0时输入结束。

Limits:

0.0000009<m<1
1<n<10
输出
输出10进制正小数m的n进制小数。结果保留10位小数。

样例
输入样例 1 复制

0.795 3
0 0
输出样例 1

0.2101101122

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

int fun(double data, int base, char *ret)
{
    int tmp[10];
    int i;

    printf("%lf %d\n", data, base);

    for (i = 0; i < 10; i++) {
        data *= base;
        tmp[i] = (int)data;
        if (data >= 1.0) data -= tmp[i];
    }

    snprintf(ret, 12, "0.");
    for (i = 0; i < 10; i++) snprintf(ret + strlen(ret), 12, "%d", tmp[i]);
    snprintf(ret + strlen(ret), 12, "\n");

    printf("ret:%s\n", ret);
}

static int test_oj_5(TEST_ENTRY *entry, void *argc, void *argv)
{
    double data = 0.795;
    char *expect = "0.2101101122", ret[20] = {0};
    int base = 3;

    fun(data, base, ret);

    if (strcmp(ret, expect) == 0) return 1;
    else return 0;

}
REGISTER_TEST_FUNC(test_oj_5)
