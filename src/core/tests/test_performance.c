#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/regisTRY/regisTRY.h>
#include <libobject/core/utils/data_structure/array_stack.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/try.h>
#include <libobject/core/Map.h>

static int test_try_catch_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    struct timeval start, end, tv;
    int a;

    timeval_now(&start, NULL);
    TRY {
    } CATCH {
    }
    ENDTRY;
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);

    timeval_print(&tv);

    return 1;
}
REGISTER_TEST_FUNC(test_try_catch_performance);

static int func_wrapper_test(int height, int a, int b, int c)
{
    height--;
    if (height == 0) {
        return 1;
    } else {
        /*
         *printf("test\n");
         */
        func_wrapper_test(height, a, b, c);
    }
}

static int test_func_wrapper_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    struct timeval start, end, tv;
    int a, b, c;

    timeval_now(&start, NULL);
    func_wrapper_test(100, a, b, c);
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);
    timeval_print(&tv);

    return 1;
}
REGISTER_TEST_FUNC(test_func_wrapper_performance);

static int test_malloc_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    void *addr;
    struct timeval start, end, tv;

    timeval_now(&start, NULL);
    for (int i = 0; i < 100; i++) {
        addr = malloc(sizeof(int));
        free(addr);
    }
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);
    timeval_print(&tv);

    return 1;
}
REGISTER_TEST_FUNC(test_malloc_performance);

static int test_allocator_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    void *addr;
    struct timeval start, end, tv;
    allocator_t *allocator = allocator_get_default_alloc();

    timeval_now(&start, NULL);
    for (int i = 0; i < 100; i++) {
        addr = allocator_mem_alloc(allocator,sizeof(int));
        allocator_mem_free(allocator, addr);
    }
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);
    timeval_print(&tv);

    return 1;
}
REGISTER_TEST_FUNC(test_allocator_performance);

struct test{
    int a;
    int b;
};

static struct test *__init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}
static int test_rbtree_add_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    struct timeval start, end, tv;
    allocator_t *allocator = allocator_get_default_alloc();
    Map *map = object_new(allocator, "RBTree_Map", NULL);
    struct test t0;

    __init_test_instance(&t0, 0, 2);
    timeval_now(&start, NULL);
    map->add(map, "test0", &t0);
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);
    timeval_print(&tv);
    object_destroy(map);

    return 1;
}
REGISTER_TEST_FUNC(test_rbtree_add_performance);

static int test_printf_performance(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    struct timeval start, end, tv;

    timeval_now(&start, NULL);
    printf("hello\n");
    timeval_now(&end, NULL);
    timeval_sub(&end, &start, &tv);
    timeval_print(&tv);

    return 1;
}
REGISTER_TEST_FUNC(test_printf_performance);
