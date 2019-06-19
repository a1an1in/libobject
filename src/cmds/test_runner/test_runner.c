#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test_Runner.h>

int test_runner(int argc, char **argv)
{
    Test_Runner * runner;
    allocator_t *allocator = allocator_get_default_alloc();

    dbg_str(DBG_DETAIL,"test_runner in");
    runner = object_new(allocator, "Test_Runner", NULL);

    runner->start(runner);

    object_destroy(runner);
    dbg_str(DBG_DETAIL,"test_runner out");
    return 0;
}
