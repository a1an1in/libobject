/*
 *
 * 题目详情 提交记录 讨论 代码检视 官方解答
题目描述
给你一串未加密的字符串s，我们需要对字符串进行加密，我们通过对字符串的每一个字母进行改变来实现加密，改变方式是对字母进行一定量的偏移，这里我们偏移的意思是把这个字母变成排在自己后面的那个字母，例如a变成b，b变成c，c变成d，y变成z，特别地，z变成a。加密方式是在每一个字母s[i]偏移斐波拉契数列数a[i]的量，斐波拉契数列是a[1]=1,a[2]=1,a[i]=a[i-1]+a[i-2]，例如：原文 uvwxyz 加密后 vwyadg，其中偏移量分别是1,1,2,3,5,8。

解答要求
时间限制：2000ms, 内存限制：64MB
输入
第一行为一个整数T（1<=T<=1000），表示有T组测试数据。每组数据包含一行，有原文s（只含有小写字母，长度小于50）。

输出
每组测试数据输出一行，表示字符串的密文

样例
输入样例 1 复制

2
uvwxyz
abcde
输出样例 1

vwyadh
bcegj

提示
计算斐波那契的时候要使用long long, 在处理字符串的时候，记得先减去’a’，然后就是一个26以内的数字，就好处理了，处理完后，在加上’a’，就得到了答案了。
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int fibonacci(int index)
{
    if (index == 0) {
        return 1;
    } else if (index == 1) {
        return 1;
    } else {
        return fibonacci(index - 1) + fibonacci(index - 2);
    }
}
int encrypt_str(char *str)
{
    int len, offset;

    len = strlen(str);

    for (int i = 0; i < len; i++) {
        offset = fibonacci(i);
        offset %= ('z' - 'a' + 1);
        if (str[i] + offset >= 'a' && str[i] + offset <= 'z') {
            str[i] += offset;
        } else if (str[i] + offset > 'z') {
            str[i] = 'a' + str[i] + offset - 'z' - 1;
        }
    }
}
static int test_oj_294(TEST_ENTRY *entry, void *argc, void *argv) {
    int i, j;
    int flag = 0;
    char test[1024] = "uvwxyz";
    char *expect = "vwyadh";

    encrypt_str(test);

    if (strcmp(test, expect) == 0) {
        return 1;
    } else {
        dbg_str(DBG_ERROR, "encrypt str: test:%s expect:%s", test, expect);
        return 0;
    }
    return 1;
}

REGISTER_TEST_CMD(test_oj_294);
