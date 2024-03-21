/*
 *
 * 题目描述
Jungle不喜欢计算，所以他希望拥有一个万能的计算器，能够计算表达式的值，你来帮他写吧！

解答要求
时间限制：1000ms, 内存限制：64MB
输入
输入一个包含四则运算、小括号的表达式，如：4+2，或((((2+3)*2+4.02)*4)+2)*4。输入包含多组数据，我们确保没有非法表达式。当一行中只有0时输入结束，相应的结果不要输出。

输出
对每个测试用例输出1行，即该表达式的值，精确到小数点后2位，如输入是1+2+3，则输出6.00。

样例
输入样例 1 复制

-2
5.6*(-2*(1+(-3)))
2*((4+2)*5)-7/11
1+2+3
0
输出样例 1

-2.00
22.40
59.36
6.00

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

static double operation(double d1, double d2, char *op)
{
    double d;

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

static double my_atof(const char* s)
{
    int i = 0, k = 1;
    double d;
    double n = 0.0, m = 0;
    int flag = 1;

    if(*s == '-'){
        flag = 0;
        i ++;
    } else {
        flag = 1;
    }
    if ((*(s + i) != '\0')) {
        while(*(s + i) >= '0' && *(s + i) <= '9'){
            n = n * 10.0 + (*(s + i) - '0');
            i ++ ;
        }
    }

    if (*(s + i) == '.') {
        i ++;
        while(*(s + i) >= '0' && *(s + i) <= '9') {
            m = m * 10.0 + (*(s + i) - '0');
            k *= 10.0;
            i ++;
        }
    }

    if(flag) {
        float t = 0.0;
        t = m / k;
        d = n + t;
    } else
        d = -1 * (n + m / k);

    return d;
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

static double calc_infix_expression_directly(char *expression)
{
    struct stack_s *data_as, *op_as;
    char *e = expression, pre_char = '+';
    char *op;
    double *d1, *d2, d;

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
                    if (*op != '(') {
                        stack_pop(data_as, (void **)&d2);
                        stack_pop(data_as, (void **)&d1);
                        d = operation(*d1, *d2, op);
                        free(d1);
                        free(d2);
                        d1 = malloc(sizeof(double));
                        *d1 = d;
                        stack_push(data_as, d1);
                    }
                } while (*op != '(');
                pre_char = *e;
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
                        stack_pop(data_as, (void **)&d2);
                        stack_pop(data_as, (void **)&d1);
                        stack_pop(op_as, (void **)&op);
                        d = operation(*d1, *d2, op);
                        free(d1);
                        free(d2);
                        d1 = malloc(sizeof(double));
                        *d1 = d;
                        stack_push(data_as, d1);
                        if (op_as->count == 0) break;
                        stack_peek(op_as, (void **)&op);
                    }
                }

                stack_push(op_as, e);
                pre_char = *e;
                e++;
            }
        } else {
            d = my_atof(e);
            d1 = malloc(sizeof(double));
            *d1 = d;
            stack_push(data_as, d1);

            if (*e == '-') e++;
            while(*e >= '0' && *e <= '9') {
                e++;
                if (*e == '.') e++;
            }

            pre_char = *(e - 1);
        }
    }

    while (op_as->count != 0) {
        stack_pop(data_as, (void **)&d2);
        stack_pop(data_as, (void **)&d1);
        stack_pop(op_as, (void **)&op);
        d = operation(*d1, *d2, op);
        free(d1);
        free(d2);
        d1 = malloc(sizeof(double));
        *d1 = d;
        stack_push(data_as, d1);
    }

    stack_pop(data_as, (void **)&d1);
    if (d1 != NULL) {
        d = *d1;
        free(d1);
    }

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



static int test_oj_40(TEST_ENTRY *entry, void *argc, void *argv)
{
    float result;
    //char *expression = "(1 + 2) * 3 * (122 - 120)";
    char *expression = "2*((4+2)*5)-7/11";
    void *data;
    double d = 5;

    //d = atof("5.6");

    //printf("%s = %f", "5.6", d);
    printf("5 / 10 = %.2f\n", d / 10);
    printf("sizeof float=%lu\n", sizeof(float));
#if 1
    dbg_str(DBG_DETAIL, "test_calc_infix_expression_directly");

    result = calc_infix_expression_directly(expression);
    printf("%s = %.2f\n", expression, result);
#endif

    return 1;
}
REGISTER_TEST_CMD(test_oj_40);
