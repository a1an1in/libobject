/*
 *
 * 328. 字符串处理1
出题人： OJ

标签：

级别： 简单
总分数： 100

 13

 8

 收藏
 分享到 eSpace

通过次数

1739

提交次数

3208

题目详情 提交记录 讨论 代码检视 官方解答
题目描述
给你两个字符串 t 和 p ，要求从 t 中找到一个和 p 相同的连续子串，并输出该字串第一个字符的下标。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入文件包括两行，分别表示字符串 t 和 p ，保证 t 的长度不小于 p ，且 t 的长度不超过1000000，p 的长度不超过10000。

输出
如果能从 t 中找到一个和 p 相等的连续子串，则输出该子串第一个字符在t中的下标（下标从左到右依次为1,2,3,…）；如果不能则输出”No”；如果含有多个这样的子串，则输出第一个字符下标最小的。

样例
输入样例 1 复制

AVERDXIVYERDIAN
RDXI
输出样例 1

4

提示
简单 Kmp
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/registry/registry.h>

void get_next(char *p, int *next) {
    int p_len = strlen(p);
    next[0] = -1;
    int k = -1;
    int j = 0;
    while (j < p_len - 1) {
        //p[k]表示前缀，p[j]表示后缀
        if (k == -1 || p[j] == p[k]) {
            ++k;
            ++j;
            next[j] = k;
        } else {
            k = next[k];
        }
    }
}

int kmp_search(char *s, char *p) {
    int i = 0, j = 0;
    int s_len = strlen(s);
    int p_len = strlen(p);
    int *next;

    next = malloc(strlen(p) * sizeof(int));
    get_next(p, next);

    while (i < s_len && j < p_len) {
        if (j == -1 || s[i] == p[j]) {
            i++;
            j++;
        } else {
            j = next[j];
        }
    }
    if (j == p_len)
        return i - j;
    else
        return -1;
}

static int test_oj_238(TEST_ENTRY *entry, void *argc, void *argv) {
    char *str = "AVERDXIVYERDIAN";
    char *pattern = "RDXI";
    int ret, expect = 4;

    ret = kmp_search(str, pattern);
    if (ret == expect) {
        printf("kmp_search test ok\n");
    } else {
        printf("kmp_search test failed, expect:%d ret=%d\n", expect,ret);
    }
    return 1;
}

REGISTER_TEST_FUNC(test_oj_238);
