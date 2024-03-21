/*
 *
 * 题目描述
Solo小学二年级了，可是问题又来了，他经常把算术表达式中的括号搞混乱，让老师很是头大，于是老师决定再次雇用你编写一个程序来检验Solo的答案的括号是否完全匹配。
注意：(1+2)(23)是括号完全匹配的，((1+2)(23)和((1+2)23则没有完全匹配。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入只有一行，即一个长度不超过100的字符串S，表示Solo的算术表达式，（你只需考虑相互之间的括号是否完全匹配，不需考虑表达式的其他合法问题）。
注意：S中不一定包含括号。

输出
若表达式的括号完全匹配了则输出“YES”，否则输出“NO”。

样例
输入样例 1 复制

5.6*(-2*(1+(-3)))
输出样例 1

YES

输入样例 2 复制

-2
输出样例 2

YES

输入样例 3 复制

1+2)
输出样例 3

NO

输入样例 4 复制

(1+2))(
输出样例 4

NO

提示
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>

struct stack_s {
    uint8_t step;
    uint8_t *cur;
    int size;
    uint8_t *top;
    int count;
};

static struct stack_s *stack_alloc() {
    struct stack_s *as;

    if ((as = (struct stack_s *) malloc(sizeof(struct stack_s))) == NULL) {
        return NULL;
    }

    memset(as, 0, sizeof(struct stack_s));

    return as;
}

static int stack_init(struct stack_s *as) {
    int ret = 0;

    if (as->size == 0) {
        as->size = 100;
    }

    if (as->step == 0) {
        as->step = sizeof(void *);
    }

    as->top = (uint8_t *) malloc(as->size);
    if (as->top == NULL) {
        return -1;
    }
    as->cur = as->top;
    as->count = 0;

    return ret;
}

static int stack_push(struct stack_s *as, void *data) {
    int ret = 0;

    if (as->cur - as->top + as->step < as->size) {
        memset(as->cur, 0, as->step);
        *((void **) as->cur) = data;
        as->cur += as->step;
        as->count++;
    } else {
        ret = -1;
    }

    return ret;
}

static int stack_pop(struct stack_s *as, void **out) {
    int ret = 0;

    if (as->cur > as->top) {
        as->cur -= as->step;
        *out = *((void **) (as->cur));
        as->count--;
    } else {
        *out = NULL;
        ret = -1;
    }

    return ret;
}

static int stack_peek(struct stack_s *as, void **out) {
    int ret = 0;

    if (as->cur > as->top) {
        *out = *((void **) (as->cur - as->step));
    } else {
        *out = NULL;
        ret = -1;
    }

    return ret;
}

static int stack_destroy(struct stack_s *as) {
    int ret = 0;

    free(as->top);
    free(as);

    return ret;
}

static int operation(int d1, int d2, char *op) {
    int d;

    switch (*op) {
        case '+':
            d = d1 + d2;
            break;
        case '-':
            d = d1 - d2;
            break;
        case '*':
            d = d1 * d2;
            break;
        case '/':
            d = d1 / d2;
            break;
        case '%':
            d = d1 % d2;
            break;
        case '^': {
            if (d2 > 1) {
                while (d2-- > 1) {
                    d1 = d1 * d1;
                }
                d = d1;
            } else if (d2 == 0) {
                d = 1;
            } else if (d2 == 1) {
                d = d1;
            }

            break;
        }
    }

    return d;
}

static int str_is_end(char *expression) {
    if (expression[0] == '\0') return 1;
    else return 0;
}

static int str_is_operator(char *expression) {
    if (expression[0] == '+' ||
        expression[0] == '-' ||
        expression[0] == '*' ||
        expression[0] == '/' ||
        expression[0] == '%' ||
        expression[0] == '^' ||
        expression[0] == '(' ||
        expression[0] == ')') {
        return 1;
    } else return 0;
}

static int my_atoi(char *src) {
    int s = 0;
    int isMinus = 0;

    while (*src == ' ') {
        src++;
    }

    if (*src == '+' || *src == '-') {
        if (*src == '-') {
            isMinus = 1;
        }
        src++;
    }

    while (*src >= '0' && *src <= '9') {
        s = s * 10 + *src - '0';
        src++;
    }
    return s * (isMinus ? -1 : 1);
}

static int operator_priority(char *operator) {
    switch (*operator) {
        case '(':
            return 1;
        case '+':
        case '-':
            return 2;
        case '*':
        case '/':
        case '%':
            return 3;
        case '^':
            return 4;
        case ')':
            return 6;
    }
}

static int is_expression_right(char *expression) {
    struct stack_s *data_as, *op_as;
    char *e = expression, pre_char = '+';
    char *op;
    int *d1, *d2, d;
    int ret = 0;

    data_as = stack_alloc();
    stack_init(data_as);

    op_as = stack_alloc();
    stack_init(op_as);

    while (!str_is_end(e)) {
        if (*e == ' ') {
            e++;
            continue;
        }

        if ((str_is_operator(e) && *e != '-') ||
            (!str_is_operator(&pre_char) && *e == '-') ||
            ((str_is_operator(&pre_char) && *e == '-') && pre_char == ')')) {
            if (*e == ')') {
                do {
                    stack_pop(op_as, (void **)&op);
                    if (op == NULL) {
                        ret = 0;
                        goto end;
                    }
                } while (*op != '(');
                e++;
                continue;
            } else if (*e == '(') {
                stack_push(op_as, e);
                pre_char = *e;
                e++;
                continue;
            } else {
                stack_peek(op_as, (void **)&op);
                if (op != NULL) {
                    while (operator_priority(op) >= operator_priority(e)) {
                        stack_pop(op_as, (void **)&op);
                        if (*op == '(') {
                            ret = 0;
                            goto end;
                        }
                        if (op_as->count == 0) break;
                        stack_peek(op_as, (void **)&op);
                    }
                }

                stack_push(op_as, e);
                pre_char = *e;
                e++;
            }
        } else {
            if (*e == '-') e++;
            while (*e >= '0' && *e <= '9') {
                e++;
                if (*e == '.') e++;
            }
            pre_char = *(e - 1);
        }
    }

    while (op_as->count != 0) {
        stack_pop(op_as, (void **)&op);
        if (*op == '(') {
            ret = 0;
            goto end;
        }
    }

    ret = 1;

    end:
    stack_destroy(data_as);
    stack_destroy(op_as);

    return ret;
}


static int test_oj_36(TEST_ENTRY *entry, void *argc, void *argv) {
    int result;
    //char *expression = "(1 + 2) * 3 * (122 - 120)";
    char *expression = "5.6*(-2*(1+(-3)))";

    result = is_expression_right(expression);
    printf("is expression :%s is %s\n", expression, result == 1 ? "YES" : "NO");

    return 1;
}

REGISTER_TEST_CMD(test_oj_36);
