#if (!defined(MAC_USER_MODE))
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include <libobject/mockery/mockery.h>
#include <libobject/stub/stub.h>

extern int str_hex_to_integer(char *str);

// test funcs
static int test_funcA(int *a, int *b, int *c)
{
    printf("call funcA, a:%d, b:%d, c:%d\n", *a, *b, *c);
    *a = *b = *c = 1;
    return *a + *b + *c;
}

static int test_funcB(int *a, int *b, int *c)
{
    int t;
    printf("call funcB, a:%d, b:%d, c:%d\n", *a, *b, *c);
    t = *a;
    *a = *c;
    *c = t;

    return *a + *b + *c;
}

int test_null_fun() 
{
    return 1;
}

static int test_stub_add_stub_only1()
{
    char ac[10] = {1};
    stub_t *stub;
    int a = 1, b = 2, c = 3;
    int ret;

    TRY {
        stub = stub_alloc();
        THROW_IF(stub == NULL, -1);
        stub_add(stub, (void*)test_funcA, (void*)test_funcB);  // 添加动态桩 用B替换A
        ret = test_funcA(&a, &b, &c);
        THROW_IF(ret != (a + b + c), -1);
        THROW_IF(a != 3 || c != 1, -1);

        EXEC(stub_remove(stub));  // 添加动态桩 用B替换A
        ret = test_funcA(&a, &b, &c);
        THROW_IF(ret != 3, -1);
    } CATCH (ret) {
    } FINALLY {
        stub_free(stub);
    }

    return ret;
}


static int my_str_hex_to_integer(char *s)
{
    return 0xdead;
}

static int test_stub_add_stub_only2()
{
    char buf[20] = {0};
    stub_t *stub;
    int ret;

    TRY {
        stub = stub_alloc();
        THROW_IF(stub == NULL, -1);
        stub_add(stub, (void*)str_hex_to_integer, (void*)my_str_hex_to_integer);
        ret = str_hex_to_integer("0x1f");
        stub_remove(stub); 
        THROW_IF(ret != 0xdead, -1);
    } CATCH (ret) {} FINALLY {
        stub_remove(stub); 
        stub_free(stub);
    }

    return ret;
}

static int test_stub_add_stub_only(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_stub_add_stub_only1());
        EXEC(test_stub_add_stub_only2());
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_FUNC(test_stub_add_stub_only);

static int print_inbound(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("inbound func of test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

static int test_func(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("original test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

static int target_func(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 8;
    printf("target test_func which replaced the original test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

static int print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("outbound func of test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

static int test_stub_add_hooks(TEST_ENTRY *entry)
{
    stub_t *stub;
    int g = 7;
    int ret;

    TRY {
        stub = stub_alloc();
        THROW_IF(stub == NULL, -1);
        dbg_str(DBG_DETAIL, "stub:%p, g addr:%p", stub, &g);

        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 7, -1);
        EXEC(stub_add_hooks(stub, (void *)test_func, (void *)print_inbound, (void *)target_func, (void *)print_outbound, 7));
        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 8, -1);
        stub_remove_hooks(stub);
        g = 7;
        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 7, -1);
    } CATCH (ret) { } FINALLY {
        stub_free(stub);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_stub_add_hooks);
#endif
