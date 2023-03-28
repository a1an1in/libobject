
/*
 * 题目描述
苗苗今天刚刚学会使用小括号，不过他分不清小中大括号和尖括号，不知道怎么使用其他括号，他认为（>以及{]是正确的（其实是错误的），你能帮助他判断括号是否使用正确（匹配）吗？

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入文件包含六组测试数据，每组测试数据是一行只包含’(‘,’)’,’{‘,’}’,’[‘,’]’,’<’,’>’的字符串（长度不超过10000）。

输出
对于每组测试数据，如果使用正确输出yes，否则输出no。每个占一行。

样例
输入样例 1 复制

[>
([]{<>})
({[<>]})
()
()
()
输出样例 1

no
yes
yes
yes
yes
yes

提示
栈的应用：
依次读入字符，如果是左括号，将它放进栈，如果是右括号，而且栈顶元素是相对应的左括号，就把栈顶元素弹出，最后如果栈空就跳出循环，结果为no，因为这样说明栈中没有左括号；字符全都读入，如果栈空的话，就是yes，否则就是no。
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

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
        expression[0] == '{' ||
        expression[0] == '}' ||
        expression[0] == '<' ||
        expression[0] == '>' ||
        expression[0] == '[' ||
        expression[0] == ']' ||
        expression[0] == '(' ||
        expression[0] == ')'
            ) {
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
    struct stack_s *op_as;
    char *e = expression;
    char *op;
    int ret = 0;

    op_as = stack_alloc();
    stack_init(op_as);

    while (!str_is_end(e)) {
        if (*e == ' ') {
            e++;
            continue;
        }

        if (str_is_operator(e)) {
            if (*e == ')') {
                stack_pop(op_as, (void **)&op);
                if (op == NULL) {
                    ret = 0;
                    goto end;
                }
                if (*op != '(') {
                    ret = 0;
                    goto end;
                }
                e++;
                continue;
            } else if (*e == '(') {
                stack_push(op_as, e);
                e++;
                continue;
            } else if (*e == ']') {
                stack_pop(op_as, (void **)&op);
                if (op == NULL) {
                    ret = 0;
                    goto end;
                }
                if (*op != '[') {
                    ret = 0;
                    goto end;
                }
                e++;
                continue;
            } else if (*e == '[') {
                stack_push(op_as, e);
                e++;
                continue;
            } else if (*e == '}') {
                stack_pop(op_as, (void **)&op);
                if (op == NULL) {
                    ret = 0;
                    goto end;
                }
                if (*op != '{') {
                    ret = 0;
                    goto end;
                }
                e++;
                continue;
            } else if (*e == '{') {
                stack_push(op_as, e);
                e++;
                continue;
            } else if (*e == '>') {
                stack_pop(op_as, (void **)&op);
                if (op == NULL) {
                    ret = 0;
                    goto end;
                }
                if (*op != '<') {
                    ret = 0;
                    goto end;
                }
                e++;
                continue;
            } else if (*e == '<') {
                stack_push(op_as, e);
                e++;
                continue;
            }
        }
    }

    while (op_as->count != 0) {
        ret = 0;
        goto end;
    }

    ret = 1;
    end:
    stack_destroy(op_as);
    return ret;
}

#if 1

static int test_oj_124(TEST_ENTRY *entry, void *argc, void *argv) {
    int result;
    //char *expression = "(1 + 2) * 3 * (122 - 120)";
    //char *expression = "([{<>})";
    //char *expression = "([]{<>})";
    char *expression = "((((((((([[[[[{{{{{}}}}}]]]]])))))))))";

    result = is_expression_right(expression);
    printf("is expression :%s is %s\n", expression, result == 1 ? "YES" : "NO");

    return 1;
}

REGISTER_TEST_CMD(test_oj_124);

#endif
