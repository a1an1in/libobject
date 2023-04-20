#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/stub/stub.h>

// test funcs
int test_funcA(int *a, int *b, int *c)
{
    printf("call funcA, a:%d, b:%d, c:%d\n", *a, *b, *c);
    *a = *b = *c = 1;
    return *a + *b + *c;
}

int test_funcB(int *a, int *b, int *c)
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

int test_stub_add1()
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

        stub_remove(stub);  // 添加动态桩 用B替换A
        ret = test_funcA(&a, &b, &c);
        THROW_IF(ret != 3, -1);
    } CATCH (ret) {
    } FINALLY {
        stub_free(stub);
    }

    return ret;
}

int test_strcpy_value = -1;
static void *my_strcmp(void *s, void *d)
{
    dbg_str(DBG_ERROR, "run at here");
    test_strcpy_value = 128;
    dbg_str(DBG_ERROR, "test_strcpy_value:%d", test_strcpy_value);
    return NULL;
}

int test_stub_add2()
{
    char buf[20] = {0};
    stub_t *stub;
    int ret;

    TRY {
        stub = stub_alloc();
        THROW_IF(stub == NULL, -1);
        stub_add(stub, (void*)strcmp, (void*)my_strcmp);
        strcmp(buf, "hello_world");
        THROW_IF(test_strcpy_value != 128, -1);
        stub_remove(stub); 

        strcpy(buf, "hello_world");
        ret = strcmp(buf, "hello_world");
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_strcpy_value:%d", test_strcpy_value);
    } FINALLY {
        stub_remove(stub); 
        stub_free(stub);
    }

    return ret;
}

int test_stub_add()
{
    int ret;

    TRY {
        EXEC(test_stub_add1());
        // EXEC(test_stub_add2());
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_FUNC(test_stub_add);

int print_inbound(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("inbound func of add_hooks_test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int add_hooks_test_func(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("original add_hooks_test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int target_func(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 8;
    printf("target add_hooks_test_func which replaced the original add_hooks_test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("outbound func of add_hooks_test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int test_stub_add_hooks()
{
    stub_t *stub;
    int g = 7;
    int ret;

    TRY {
        stub = stub_alloc();
        THROW_IF(stub == NULL, -1);
        dbg_str(DBG_DETAIL, "stub:%p, g addr:%p", stub, &g);

        add_hooks_test_func(1, 2, 3, 4, 5, 6, &g);
        EXEC(stub_add_hooks(stub, (void *)add_hooks_test_func, (void *)print_inbound, (void *)target_func, (void *)print_outbound, 7));
        add_hooks_test_func(1, 2, 3, 4, 5, 6, &g);
        stub_remove_hooks(stub);
        add_hooks_test_func(1, 2, 3, 4, 5, 6, &g);
    } CATCH (ret) { } FINALLY {
        stub_free(stub);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_stub_add_hooks);
