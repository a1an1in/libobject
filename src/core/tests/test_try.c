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

static int test_try_catch1(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    exception_init();
    int ret;

#if 1
    TRY {
        TRY {
            THROW(-1, "recall B");
        } CATCH {
            printf("CATCH %d at level2\n", ret);
        }
        ENDTRY;
        printf("throw -2\n");
        THROW(-2, NULL);
    } CATCH {
        if (ret == -2) ret = 1;
        else ret = 0;
        printf("CATCH %d at level1\n", ret);
    }
    ENDTRY;
#endif

    return ret;
}
REGISTER_TEST_FUNC(test_try_catch1);

static void try_catch_test_fuc()
{
    TRY {
        THROW(-5, "recall B");
    }
    ENDTRY;
}
static int test_try_catch2(TEST_ENTRY *enTRY, void *argc, void *argv)
{
    int ret = -5;

    TRY {
        try_catch_test_fuc();
    } CATCH {
        printf("CATCH error code, ret =%d\n", ret);
        if (ret == -5) ret = 1;
        else ret = 0;
    } FINALLY {
        printf("run at finally\n");
    };
    ENDTRY;

    return ret;
}
REGISTER_TEST_FUNC(test_try_catch2);

