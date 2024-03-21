/*
 * 题目描述
在对银行账户等重要权限设置密码的时候，我们常常遇到这样的烦恼：如果为了好记用生日吧，容易被破解，不安全；如果设置不好记的密码，又担心自己也会忘记；如果写在纸上，担心纸张被别人发现或弄丢了…这个程序的任务就是把一串拼音字母转换为6位数字（密码）。我们可以使用任何好记的拼音串(比如名字，王喜明，就写：wangximing)作为输入，程序输出6位数字。变换的过程如下：

第一步. 把字符串6个一组折叠起来，比如wangximing则变为：
wangxi
ming

第二步. 把所有垂直在同一个位置的字符的ascii码值相加，得出6个数字，如上面的例子，则得出：
228 202 220 206 120 105

第三步. 再把每个数字“缩位”处理：就是把每个位的数字相加，得出的数字如果不是一位数字，就再缩位，直到变成一位数字为止。例如: 228 => 2+2+8=12 => 1+2=3

上面的数字缩位后变为：344836, 这就是程序最终的输出结果！

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入的第一行是一个整数n（n<100），表示下边有多少输入行，接下来是n行长度不超过100的字符串S，就是等待变换的字符串。

输出
输出n行变换后的6位密码。

样例
输入样例 1 复制

5
zhangfeng
wangximing
jiujingfazi
woaibeijingtiananmen
haohaoxuexi
输出样例 1

772243
344836
297332
716652
875843

提示
 * */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/mockery/mockery.h>

unsigned int compress_integer(unsigned int num)
{
    int remainder = 0, quotient;

    while (num > 0) {
        quotient = num / 10;
        remainder += num % 10;
        num = quotient;
    }

    if (remainder > 10) {
        return compress_integer(remainder);
    } else
        return remainder;
}

int password_generator(char *plain_text, char *pwd)
{
    int tmp[6] = {0};

    for (int i = 0; plain_text[i] != '\0'; i++) {
        tmp[i % 6] += plain_text[i];
    }

    for (int i = 0; i < 6; i++) {
        tmp[i] = compress_integer(tmp[i]);
        sprintf(pwd + i, "%d", tmp[i]);
    }
    pwd[6] = '\0';

    return 0;
}

static int test_oj_80(TEST_ENTRY *entry, void *argc, void *argv) {
    char *s1  = "zhangfeng";
    char *s2 = "772243";
    char pwd[7];

    password_generator(s1, pwd);
    printf("%s's password is %s, expect:%s\n", s1, pwd, s2);

    return 1;
}
REGISTER_TEST_CMD(test_oj_80);
