/*
 *
 * 题目描述
给定一个正整数，我们可以定义出下面的公式:
N=a[1]+a[2]+a[3]+…+a[m];
a[i]>0,1<=m<=N;
对于一个正整数，求解满足上面公式的所有算式组合，如，对于整数 4 :

4 = 4;
4 = 3 + 1;
4 = 2 + 2;
4 = 2 + 1 + 1;
4 = 1 + 1 + 1 + 1;
所以上面的结果是 5 。
注意：对于 “4 = 3 + 1” 和 “4 = 1 + 3” ，这两处算式实际上是同一个组合!

解答要求
时间限制：1000ms, 内存限制：64MB
输入
每个用例中，会有多行输入，每行输入一个正整数，表示要求解的正整数N(1 ≤ N ≤ 120) 。

输出
对输入中的每个整数求解答案，并输出一行(回车换行);

样例
输入样例 1 复制

4
10
20
输出样例 1

5
42
627

提示
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int st[100][100];

int dp(int n, int k) {
    //先解决第一行和第一列的边界情况
    for (int j = 1; j <= k; j++) {
        st[1][j] = 1;
    }
    for (int i = 1; i <= n; i++) {
        st[i][1] = 1;
    }
    //再解决常规情况
    for (int i = 2; i <= n; i++) {
        for (int j = 2; j <= k; j++) {
            if (j > i)
                st[i][j] = st[i][i];
            else if (j == i)
                st[i][j] = st[i][j - 1] + 1;
            else if (j < i)
                st[i][j] = st[i - j][j] + st[i][j - 1];
        }
    }
    return st[n][k];
}

#include <libobject/mockery/mockery.h>

static int test_oj_31_1(TEST_ENTRY *entry, void *argc, void *argv) {
    memset(st, 0, sizeof(0));
    int num = 5;
    printf("partition num: %d\n", dp(num, num));
    return 1;
}
REGISTER_TEST_CMD(test_oj_31_1);


static void recursion(int num, int max, int *count)
{
    if (num <= 0) {
        (*count)++;
        return;
    }

    if (num < max) {
        max = num;
    }

    for (int i = 0; i < max; i++) {
        recursion(num - max + i,  max - i, count);
    }
}

static int test_oj_31_2(TEST_ENTRY *entry, void *argc, void *argv) {
    memset(st, 0, sizeof(0));
    int num = 5;
    int count = 0;

    recursion(num, num, &count);
    printf("%d\n", count);

    return 1;
}
REGISTER_TEST_CMD(test_oj_31_2);
