/*
 * 题目描述
Word Maze 是一个网络小游戏，你需要找到以字母标注的食物，但要求以给定单词字母的顺序吃掉。假设给定单词if，你必须先吃掉i然后才能吃掉f。
但现在你的任务可没有这么简单，你现在处于一个迷宫Maze（n×m的矩阵）当中，里面到处都是以字母标注的食物，但你只能吃掉能连成给定单词W的食物。

注意区分英文字母大小写,并且你只能上下左右行走。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入第一行包含两个整数n、m(0<n,m<21)分别表示n行m列的矩阵，第二行是长度不超过100的单词W，从第3行到底n+3行是只包含大小写英文字母的长度为m的字符串。

输出
如果能在地图中连成给定的单词，则输出“YES”，否则输出“NO”。注意：每个字母只能用一次。

样例
输入样例 1 复制

5 5
SOLO
CPUCY
EKLQH
CRSOL
EKLQO
PGRBC
输出样例 1

YES

提示
 *
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

#if 0
static char test[22][22] = {{'C', 'P', 'U', 'C', 'Y'},
                            {'E', 'K', 'L', 'Q', 'H'},
                            {'C', 'R', 'S', 'O', 'L'},
                            {'E', 'K', 'L', 'Q', 'O'},
                            {'P', 'G', 'R', 'B', 'C'}};
static char w[101] = "SOLO";
static int m = 5, n = 5;
#else
static char test[22][22] = {{'a', 'o', 'l', 'f'},
                            {'e', 's', 'i', 'l'}};
static char w[101] = "solo";
static int m = 2, n = 4;
#endif
static int move[4][2] = {{-1, 0},
                         {0,  -1},
                         {0,  1},
                         {1,  0}};


static int visited[22][22] = {0};

static int dfs(int x, int y, int t) {
    int i;
    int xx, yy;

    if (w[t] == '\0')
        return 1;

    if (test[y][x] == w[t]) {
        visited[y][x] = 1;             //0表示没有访问
        for (i = 0; i < 4; i++) {
            xx = x + move[i][0];
            yy = y + move[i][1];
            if (xx >= 0 && yy >= 0 && xx < n && yy < m && (!visited[yy][xx])) {
                if (dfs(xx, yy, t + 1) == 1)
                    return 1;
            }
        }
    }

    visited[y][x] = 0;

    return 0;
}

static int test_oj_12(TEST_ENTRY *entry, void *argc, void *argv) {
    int i, j;
    int flag = 0;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            if (dfs(i, j, 0) == 1)
                flag = 1;
        }
    }

    if (1 == flag)
        printf("YES\n");
    else
        printf("NO\n");

    return 1;
}

REGISTER_TEST_CMD(test_oj_12);
