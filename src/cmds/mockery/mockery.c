#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int mockery(int argc, char **argv)
{
    dbg_str(DBG_DETAIL, "mockery start");

    if (argc > 0) {
        int targc = argc - 1;
        execute_test_designated_func(argv[0], (void *)&targc, argv + 1);
    } else {
        debugger_set_all_businesses_level(debugger_gp, 1, 6);
        execute_test_funcs();
    }

    dbg_str(DBG_DETAIL, "mockery end");

    return argc;
}
