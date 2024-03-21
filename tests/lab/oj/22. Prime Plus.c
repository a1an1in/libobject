/*
 * 题目描述
Write a program which reads an integer n and prints the number of prime numbers which are less than or equal to n. A prime number is a natural number which has exactly two distinct natural number divisors: 1 and itself. For example, the first four prime numbers are: 2, 3, 5, 7.

解答要求
时间限制：5000ms, 内存限制：64MB
输入
Input only has an integer n (0<n<100000001) in a line

输出
Prints the number of prime numbers

样例
输入样例 1 复制

6
输出样例 1

3

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
#include <libobject/core/utils/dbg/debug.h>

int count_primes(int n) {
    if (n < 3) {
        return 0;
    }

    int count = n / 2;// 筛掉一半偶数
    int *not_prime = malloc(n *sizeof (int));
    memset(not_prime, 0, n * sizeof(int));
    //int not_prime[10240] = {0};
    for (int i = 3; i * i < n; i += 2) {// 只筛3≤i<√n奇数
        if (!not_prime[i]) {
            for (int j = i * i; j < n; j += 2 * i) {// 从i*i开始筛
                if (!not_prime[j]) {
                    not_prime[j] = 1;
                    count--;
                }
            }
        }
    }
    return count;
}

static int test_oj_22(TEST_ENTRY *entry, void *argc, void *argv) {
    int num = 6;
    int expect = 3, ret;

    dbg_str(DBG_DETAIL, "test_oj_22");
    //please define the C input here. For example: int n; scanf("%d",&n);
    ret = count_primes(num);

    if (ret == expect) {
        return 1;
    } else {
        return 0;
    }
}
REGISTER_TEST_CMD(test_oj_22);
