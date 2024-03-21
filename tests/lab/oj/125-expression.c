/*
 *
 * 题目描述
给你一个数学表达式，请你求出它的值。
表达式中可能含有以下运算符：”+”、”-“、”*“、”/“、”%”(取余)、”^”(指数)、”(“、”)”，其中优先级为: 括号 > “^” > “*” = “/“ = “%” > “+” = “-“。

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入文件包含一行，字符之间用空格分开，所有的数字为小于10的正整数,表达式中的字符数目不超过1000，以$为结束符号;

输出
输出一个整数，表示该表达式的值，结果小于2^31。

样例
输入样例 1 复制

( 2 + 1 ) * 3 $
输出样例 1

9

提示
本题主要考察的是栈的应用。解决方法是将中缀表达式转化成后缀表达式，进行求解。

中缀表达式转后缀表达式方法如下：
1、遇到数字时，加入后缀表达式；
2、运算符：
（1）若为 ‘(‘，入栈；
（2）若为 ‘)’，则依次把栈中的的运算符加入后缀表达式中，直到出现’(‘，从栈中删除’(‘ ；
（3）若为 除括号外的其他运算符， 当其优先级高于除’(‘以外的栈顶运算符时，直接入栈。否则从栈顶开始，依次弹出比当前处理的运算符优先级高和优先级相等的运算符，直到一个比它优先级低的或者遇到了一个左括号为止。
3、当扫描的中缀表达式结束时，栈中的的所有运算符出栈。

后缀表达式求解方法如下：
1、遇到数字时，入栈；
2、遇到运算符时，将栈顶两个元素出栈，运算完成后，再入栈；
3、当后缀表达式扫描完后，栈顶元素就是表达式的值。
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

static  struct stack_s *stack_alloc()
{
    struct stack_s *as;

    if( (as = (struct stack_s *)malloc(sizeof(struct stack_s))) == NULL ) {
        return NULL;
    }

    memset(as, 0 , sizeof(struct stack_s));

    return as;
}

static int stack_init(struct stack_s *as)
{
    int ret = 0;

    if(as->size == 0) {
        as->size = 100;
    }

    if(as->step == 0) {
        as->step = sizeof(void *);
    }

    as->top = (uint8_t *)malloc(as->size);
    if(as->top == NULL) {
        return -1;
    }
    as->cur = as->top;
    as->count = 0;

    return ret;
}

static int stack_push(struct stack_s *as, void *data)
{
    int ret = 0;

    if(as->cur - as->top + as->step < as->size) {
        memset(as->cur, 0, as->step);
        *((void **)as->cur) = data;
        as->cur += as->step;
        as->count++;
    } else {
        ret = -1;
    }

    return ret;
}

static int stack_pop(struct stack_s *as,void **out)
{
    int ret = 0;

    if(as->cur > as->top) {
        as->cur -= as->step;
        *out = *((void **)(as->cur));
        as->count--;
    } else {
        ret = -1;
    }

    return ret;
}

static int stack_peek(struct stack_s *as,void **out)
{
    int ret = 0;

    if(as->cur > as->top) {
        *out = *((void **)(as->cur - as->step));
    } else {
        *out = NULL;
        ret = -1;
    }

    return ret;
}

static int stack_destroy(struct stack_s *as)
{
    int ret = 0;

    free(as->top);
    free(as);

    return ret;
}

static int operation(int d1, int d2, char *op)
{
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

static int str_is_end(char *expression)
{
    if (expression[0] == '\0') return 1;
    else return 0;
}

static int str_is_operator(char *expression)
{
    if (expression[0] == '+' ||
        expression[0] == '-' ||
        expression[0] == '*' ||
        expression[0] == '/' ||
        expression[0] == '%' ||
        expression[0] == '^' ||
        expression[0] == '(' ||
        expression[0] == ')')
    {
        return 1;
    } else return 0;
}

static int my_atoi(char *src)
{
    int s = 0;
    int isMinus = 0;

    while (*src == ' ') {
        src++;
    }

    if(*src == '+' || *src == '-') {
        if(*src == '-') {
            isMinus = 1;
        }
        src++;
    }

    while(*src >= '0' && *src <= '9'){
        s = s * 10 + *src - '0';
        src++;
    }
    return s * (isMinus ? -1 : 1);
}

static int operator_priority(char *operator)
{
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

static int calc_infix_expression_directly(char *expression)
{
    struct stack_s *data_as, *op_as;
    char *e = expression;
    char *op;
    int *d1, *d2, d;

    data_as = stack_alloc();
    stack_init(data_as);

    op_as = stack_alloc();
    stack_init(op_as);

    while (!str_is_end(e)) {
        if (*e == ' ') {
            e++;
            continue;
        }

        if (str_is_operator(e)) {
            if (*e == ')') {
                do {
                    stack_pop(op_as, (void **)&op);
                    if (*op != '(') {
                        stack_pop(data_as, (void **)&d2);
                        stack_pop(data_as, (void **)&d1);
                        d = operation((int)d1, (int)d2, op);
                        stack_push(data_as, d);
                    }
                } while (*op != '(');
                e++;
                continue;
            } else if (*e == '(') {
                stack_push(op_as, e);
                e++;
                continue;
            } else {
                stack_peek(op_as, (void **)&op);
                if (op != NULL) {
                    while (operator_priority(op) >= operator_priority(e)) {
                        stack_pop(data_as, (void **)&d2);
                        stack_pop(data_as, (void **)&d1);
                        stack_pop(op_as, (void **)&op);
                        d = operation((int)d1, (int)d2, op);
                        stack_push(data_as, d);
                        if (op_as->count == 0) break;
                        stack_peek(op_as, (void **)&op);
                    }
                }

                stack_push(op_as, e);
                e++;
            }
        } else {
            d = my_atoi(e);
            stack_push(data_as, d);
            while(*e >= '0' && *e <= '9') {
                e++;
            }
        }
    }

    while (op_as->count != 0) {
        stack_pop(data_as, (void **)&d2);
        stack_pop(data_as, (void **)&d1);
        stack_pop(op_as, (void **)&op);
        d = operation((int)d1, (int)d2, op);
        stack_push(data_as, d);
    }

    stack_pop(data_as, (void **)&d1);
    d = (int)d1;
    stack_destroy(data_as);
    stack_destroy(op_as);

    return d;
}

static int convert_expression(char *expression)
{
    int len;

    len = strlen(expression);

    for (int i = 0; i < len; i++) {
        if (expression[i] == '$') {
            expression[i] = '\0';
            break;
        }
    }

    return 0;
}

static int test_oj_125(TEST_ENTRY *entry, void *argc, void *argv)
{
    int result;
    //char *expression = "(1 + 2) * 3 * (122 - 120)";
    //char expression[1024] = "( 2 + 1 ) * 3 $";
    //char expression[1024] = "3 - 3 * 3 % 9 $";
    //char expression[1024] = "( 2 + 3 ) * 6 ^ 2 $";
    //char expression[1024] = "( 2 + 3 ) * 6 ^ ( ( ( 2 + 1 ) - 9 ) % 3 ) - 6 / ( 9 + ( 3 ) ) $";
    char expression[1024 * 2] ="2 * 7 - 2 * 5 - 6 - 2 % 9 * 6 - 5 % 1 + 7 * 1 + 5 / 1 + 1 + 8 - 3 - 8 - 1 + 1 - 9 * 1 / 9 / 8 * 3 % 5 - 5 % 4 * 1 * 3 - 5 + 3 % 5 % 5 - 5 / 2 - 1 / 5 * 9 % 6 + 3 * 8 * 5 / 3 - 9 % 5 / 9 / 7 % 9 + 4 * 3 - 6 - 2 % 5 % 7 - 7 / 4 % 3 % 4 / 3 / 8 - 2 / 7 % 4 % 2 - 5 * 5 - 8 * 1 / 9 - 4 + 9 - 8 + 6 * 6 / 8 % 5 * 9 * 2 / 7 + 7 - 3 + 8 - 1 % 6 + 1 + 1 / 4 * 2 * 1 + 9 - 4 / 2 % 8 - 7 - 1 - 1 / 9 - 9 * 1 % 2 + 9 / 5 % 7 * 9 - 5 % 5 * 7 / 3 * 7 - 7 * 8 - 2 % 8 - 9 * 4 % 3 % 4 + 4 * 9 % 2 + 5 * 9 + 2 / 3 / 2 + 1 / 5 / 9 / 3 - 9 + 6 / 7 - 8 % 1 * 2 % 1 % 5 % 7 % 4 * 4 / 4 - 5 - 6 * 2 * 8 / 7 - 3 - 6 % 3 + 7 * 2 / 3 + 3 / 1 + 2 - 6 + 1 + 7 / 6 % 9 - 7 * 2 % 1 * 3 - 8 + 2 % 2 % 5 % 6 / 1 % 9 + 6 - 4 / 9 % 1 - 9 + 5 / 3 - 7 * 5 % 1 % 5 - 2 % 9 + 9 - 3 / 5 * 2 + 2 % 4 % 6 % 7 % 6 - 6 / 9 % 1 - 7 - 7 + 3 - 1 / 5 % 6 * 7 * 4 * 4 - 9 * 7 / 4 + 8 * 7 % 5 % 3 % 5 * 6 + 6 % 4 + 6 - 3 / 2 - 5 * 2 + 2 % 3 + 2 * 4 - 1 - 1 - 8 % 5 - 9 / 4 - 8 * 5 % 3 / 7 * 4 / 7 % 6 * 7 % 8 + 1 % 3 - 4 / 2 + 2 + 5 * 7 * 9 - 6 + 7 / 3 * 7 * 8 + 6 % 8 * 2 * 4 % 6 / 6 / 6 / 8 * 3 + 9 * 7 % 2 / 4 % 4 - 9 - 9 % 2 - 5 / 4 % 9 * 3 - 4 - 4 % 9 / 4 % 1 % 5 + 7 / 5 + 4 * 5 / 1 - 8 + 3 * 7 / 6 - 8 / 6 * 3 * 8 - 1 % 6 - 1 + 7 * 6 % 8 - 9 * 6 + 5 % 2 * 2 + 1 + 5 - 3 + 8 / 1 / 5 * 8 / 7 % 6 * 4 * 9 - 6 / 5 - 4 * 7 % 5 % 7 - 2 + 4 % 8 + 2 % 2 * 2 - 2 + 9 * 2 / 5 + 7 % 2 + 6 * 2 - 2 - 8 - 4 + 9 % 2 % 6 / 7 - 7 * 5 - 3 / 2 % 5 / 9 - 9 * 4 + 1 + 6 % 9 / 9 - 8 / 1 - 8 + 6 % 3 * 7 + 9 - 8 % 3 + 9 * 2 / 1 / 6 % 7 + 9 + 9 * 2 - 1 % 8 * 7 % 1 * 5 - 9 % 5 - 6 - 2 * 1 / 4 / 7 / 6 / 6 % 5 + 2 / 9 + 7 / 2 / 4 / 4 / 5 % 2 / 7 - 1 / 2 % 5 * 1 * 6 / 1 / 5 + 8 - 2 / 6 - 5 / 1 + 5 / 6 * 3 % 6 - 2 - 8 % 5 + 1 * 6 % 9 % 5 + 3 * 4 - 9 - 1 - 6 / 2 / 7 + 6 + 7 % 7 % 5 / 3 % 6 + 5 * 4 + 8 / 9 / 2 - 4 + 3 % 4 - 1 % 8 * 3 % 9 + 8 / 8 % 3 - 8 % 3 + 4 - 8 % 7 - 7 - 8 - 9 - 2 * 1 + 4 * 6 * 8 * 3 + 1 - 2 * 7 + 8 % 1 - 5 * 1 / 6 % 1 - 2 - 4 + 3 * 5 * 3 % 4 + 8 * 8 + 8 + 5 % 9 / 2 + 5 / 1 + 3 - 7 - 3 - 9 / 2 + 7 * 9 - 1 + 6 % 9 + 6 / 4 % 3 % 2 % 3 $";
    convert_expression(expression);

    result = calc_infix_expression_directly(expression);
    printf("%s = %d\n", expression, result);

    return 1;
}
REGISTER_TEST_CMD(test_oj_125);
