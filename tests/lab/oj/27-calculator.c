/*
 * 题目描述
Solo小学一年级的时候做数学题很莫名奇妙，经常把算术表达式加上很多空格（如：7+?31????-2），让老师很是头大，于是老师决定雇用你编写一个程序来独立计算Solo的答案。Can you help the teacher?

解答要求
时间限制：5000ms, 内存限制：64MB
输入
输入只有一行，即一个长度不超过100的字符串S，表示Solo的算术表达式，且S只包含数字和”+”、”-”两种运算符，以及Solo加上的一大堆空格（我们保证输入都是合法的）。
注意：S中不一定包含运算符，且我们保证S中不会出现大于100000的数。

输出
输出表达式的运算结果。

样例
输入样例 1 复制

1+2 + 3 +   4
输出样例 1

10
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

struct stack_s {
    uint8_t step;
    uint8_t *cur;
    int size;
    uint8_t *top;
    int count;
};

static struct stack_s *stack_alloc()
{
    struct stack_s *as;

    if( (as = (struct stack_s *)malloc(sizeof(struct stack_s))) == NULL ) {
        dbg_str(DBG_DETAIL,"allocator_mem_alloc");
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
        dbg_str(DBG_WARNNING,"allocator_mem_alloc");
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
        dbg_str(DBG_WARNNING,"array stack is full");
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
        dbg_str(DBG_WARNNING,"array stack is null");
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
        dbg_str(DBG_WARNNING,"array stack is null");
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

static int test_stack(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret = 0;
    struct stack_s *as;
    int *p;
    void *a;

    as = stack_alloc();
    as->step = 8;
    stack_init(as);

    dbg_str(DBG_DETAIL,"as addr:%p",as);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)4 );
    stack_push(as, a);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)5 );
    stack_push(as, a);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)6 );
    stack_push(as, a);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)7);
    stack_push(as, a);


    stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);

    stack_destroy(as);

    return ret;
}
REGISTER_TEST_FUNC(test_stack);

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
            return 3;
        case ')':
            return 4;
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
                while (operator_priority(op) >= operator_priority(e)) {
                    stack_pop(data_as, (void **)&d2);
                    stack_pop(data_as, (void **)&d1);
                    stack_pop(op_as, (void **)&op);
                    d = operation((int)d1, (int)d2, op);
                    stack_push(data_as, d);
                    if (op_as->count == 0) break;
                    stack_peek(op_as, (void **)&op);
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


static int test_oj_27(TEST_ENTRY *entry, void *argc, void *argv)
{
    int result;
    //char *expression = "(1 + 2) * 3 * (122 - 120)";
    char *expression = "1+2 + 3 +   4";

    dbg_str(DBG_DETAIL, "test_calc_infix_expression_directly");

    result = calc_infix_expression_directly(expression);
    printf("%s = %d\n", expression, result);

    return 1;
}
REGISTER_TEST_CMD(test_oj_27);


