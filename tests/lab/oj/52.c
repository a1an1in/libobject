/*
 *
 * 题目描述
给定两个由大小写字母和空格组成的字符串s1和 s2，它们的长度都不超过 100 个字符。判断压缩掉空格、并忽略大小写后，这两个字符串在是否相等。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入两个字符串（分两行输入），只包含字母和空格。输入有多组测试，且到文件末尾结束。

输出
如果两个字符串相等则输出"Yes",否则输出"No"。

样例
输入样例 1 复制

asdf
aSDf
asdf aaa
aSdf    aaa
输出样例 1

Yes
Yes

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

int str_is_same(char *s1, char *s2)
{
    while (1) {
        while (*s1 == ' ') {
            s1++;
        }
        while (*s2 == ' ') {
            s2++;
        }

        if (*s1 == *s2 && *s1 == '\0') {
            s1++;
            s2++;
            return 1;
        } else if ((*s1 > *s2 && *s1 - *s2 ==  'a' - 'A') ||
                   (*s2 > *s1 && *s2 - *s1 ==  'a' - 'A') ||
                   (*s1 == *s2 && *s1 != '\0')) {
            s1++;
            s2++;
            continue;
        } else
            return 0;
    }
}

static int test_oj_52(TEST_ENTRY *entry, void *argc, void *argv) {
    char *s1  = "hello world";
    char *s2 = "Hello  W0rld";

    printf("is %s and %s is same:%d\n", s1, s2, str_is_same(s1, s2));
    return 1;
}
REGISTER_TEST_CMD(test_oj_52);
