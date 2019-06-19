#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Vector.h>
#include <libobject/ctest/Test_Runner.h>

int test_runner(int argc, char **argv)
{
    Test_Runner * runner;
    allocator_t *allocator = allocator_get_default_alloc();
    Vector *failed_cases, *success_cases;

    dbg_str(DBG_DETAIL,"test_runner in");
    runner = object_new(allocator, "Test_Runner", NULL);

    runner->start(runner);

    failed_cases = runner->result->failed_cases;
    success_cases = runner->result->success_cases;

    printf("dump success_cases: %s\n",
            success_cases->to_json(success_cases));
    printf("dump failed_cases: %s\n",
            failed_cases->to_json(failed_cases));

    object_destroy(runner);
    dbg_str(DBG_DETAIL,"test_runner out");
    return 0;
}
