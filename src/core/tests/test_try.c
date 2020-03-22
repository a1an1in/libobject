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

static int test_try_catch3(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    exception_init();
    int ret;

#if 1
    TRY {
        TRY {
            THROW(-1, "recall B");
        } CATCH (ret) {
            printf("CATCH %d at level2\n", ret);
        }
        ENDTRY;
        printf("throw -2\n");
        THROW(-2, NULL);
    } CATCH_ERR (-1) {
        printf("CATCH -1 at level1\n");
    } CATCH (ret) {
        if (ret == -2) ret = 1;
        else ret = 0;
        printf("CATCH %d at level1\n", ret);
    }
    ENDTRY;
#endif

    return ret;
}
REGISTER_TEST_FUNC(test_try_catch3);


void *thread(void *args) {

    pthread_t selfid = pthread_self();

    TRY {
        THROW (-1, "throw -1");
    } CATCH_ERR (-1) {
        printf("CATCH -1 : %ld\n", (long)selfid);
    }
    ENDTRY;

    TRY {
        THROW(-2, "throw -2");
    } CATCH_ERR (-2){
        printf("CATCH -2 : %ld\n", (long)selfid);
    }
    ENDTRY;


    TRY {
        THROW(-3, "throw -3");
    } CATCH_ERR (-3){
        printf("CATCH -3 : %ld\n", (long)selfid);
    }
    ENDTRY;

    TRY {
        THROW(-4, "throw -4");
    } CATCH_ERR (-4){
        printf("CATCH -4 : %ld\n", (long)selfid);
    }
    ENDTRY;

    TRY {
        THROW(-1, "-1 Again");
        THROW(-2, "-2 Again");
        THROW(-3, "-3 Again");
        THROW(-4, "-4 Again");
    } CATCH_ERR (-1){
        printf("CATCH -1 again : %ld\n", (long)selfid);
    } CATCH_ERR (-2){
        printf("CATCH -2 again : %ld\n", (long)selfid);
    } CATCH_ERR (-3){
        printf("CATCH -3 again : %ld\n", (long)selfid);
    } CATCH_ERR (-4){
        printf("CATCH -4 again : %ld\n", (long)selfid);
    } FINALLY {
        printf("finaly: %ld\n", (long)selfid);
    };
    ENDTRY;

}

#define THREADS  50
static int test_try_catch4(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    exception_init();

    int i = 0;
    pthread_t threads[THREADS];

    for (i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread, NULL);
    }

    for (i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

}
REGISTER_TEST_FUNC(test_try_catch4);

static void try_catch_test_fuc()
{
    TRY {
        THROW(-5, "recall B");
    }
    ENDTRY;
}
static int test_try_catch5(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    int ret = -5;

#if 1
    try_catch_test_fuc();
#else
    TRY {
        try_catch_test_fuc();
    } CATCH (ret) {
        printf("CATCH error code, ret =%d\n", ret);
        if (ret == -5) ret = 1;
        else ret = 0;
    } FINALLY {
        printf("run at finally\n");
    };
    ENDTRY;
#endif

    return ret;
}
REGISTER_TEST_FUNC(test_try_catch5);

static int try_catch_test_fuc2()
{
    return -1;
}

static int test_try_catch6(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    EXEC(try_catch_test_fuc2());

    return 1;
}
REGISTER_TEST_FUNC(test_try_catch6);
