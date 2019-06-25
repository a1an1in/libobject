/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Vector.h>
#include <libobject/ctest/Test_Runner.h>
#include <libobject/cmds/Ctest_Command.h>
#include <libobject/argument/Application.h>

static int __action(Command *command)
{
    Test_Runner * runner;
    allocator_t *allocator = allocator_get_default_alloc();
    Vector *failed_cases, *success_cases;

    dbg_str(DBG_DETAIL,"test_runner in");
    runner = object_new(allocator, "Test_Runner", NULL);

    runner->start(runner);

    failed_cases  = runner->result->failed_cases;
    success_cases = runner->result->success_cases;

    printf("dump success_cases: %s\n",
            success_cases->to_json(success_cases));
    printf("dump failed_cases: %s\n",
            failed_cases->to_json(failed_cases));

    object_destroy(runner);
    dbg_str(DBG_DETAIL,"test_runner out");
    return 0;
}

static int __construct(Command *command, char *init_str)
{
    command->add_option(command, "--run", "-r", "all", "run test cases", NULL);
    command->add_option(command, "--output-type", "-t", "json", "output file type", NULL);
    command->add_option(command, "--output-file", "-o", "test_report.json", "output file path", NULL);

    command->set(command, "/Command/name", "CTest");

    return 0;
}

static int __deconstruct(Command *command)
{
    return 0;
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Ctest_Command, construct, __construct),
    Init_Nfunc_Entry(2, Ctest_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Ctest_Command, action, __action),
    Init_End___Entry(4, Ctest_Command),
};
REGISTER_CLASS("Ctest_Command", test_command_class_info);
REGISTER_APP_CMD("Ctest_Command");
