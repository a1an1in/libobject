#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int mockery(int argc, char **argv)
{
    dbg_str(DBG_DETAIL, "mockery start");

    execute_test_funcs();

    dbg_str(DBG_DETAIL, "mockery end");

    return 0;
}
