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

static int __option_version_action_callback(Option *option, void *opaque)
{
    Option *o = option;
    if (!o->value->equal(o->value, "ture")) {
        dbg_str(DBG_ERROR,"user set -v option");
    }
}

static int __option_run_action_callback(Option *option, void *opaque)
{
    Test_Runner *runner = (Test_Runner *)opaque;
    Command *command = (Command *)option->command;
    Option *o = option;

    dbg_str(DBG_SUC,"option_run_action_callback");

    if (!o->value->equal(o->value, "all")) {
        runner->set_white_list(runner, o->value->get_cstr(o->value));
        runner->set_white_list_flag = 1;
    }

}

static int __action(Command *command)
{
    Test_Runner *runner, **r;
    allocator_t *allocator = allocator_get_default_instance();
    Vector *failed_cases, *success_cases;
    int count;
    Argument *arg;
    Option *o;
    Vector *options = command->options;
    uint8_t i, option_count = command->options->count(command->options);

    dbg_str(DBG_DETAIL,"test_runner in");

    r = command->get(command, "/Command/opaque");
    if (r == NULL) {
        dbg_str(DBG_DETAIL,"not found test runner");
        return -1;
    } else {
        runner = *r;
    }

    command->run_option_actions(command);

    /*test cases is designated by args*/
    count = command->args->count(command->args);
    if (count == 1) {
        arg = command->get_argment(command, 0);
        if (arg != NULL) {
            dbg_str(DBG_SUC,"test_cases:%s", arg->value->get_cstr(arg->value));
            runner->set_white_list(runner, arg->value->get_cstr(arg->value));
            runner->set_white_list_flag = 1;
        }
    }

    if (runner->set_white_list_flag == 0) {
        debugger_set_all_businesses_level(debugger_gp, 1, 6);
    }

    runner->start(runner);

    failed_cases  = runner->result->failed_cases;
    success_cases = runner->result->success_cases;

    /*
     *printf("dump success_cases: %s\n",
     *        success_cases->to_json(success_cases));
     */
    printf("dump failed_cases: %s\n",
            failed_cases->to_json(failed_cases));

    dbg_str(DBG_DETAIL,"test_runner out");

    return 0;
}

static int __construct(Command *command, char *init_str)
{
    Test_Runner * runner;

    runner = object_new(command->parent.allocator, "Test_Runner", NULL);

    command->add_option(command, "--run", "-r", "all", "run test cases", __option_run_action_callback, runner);
    command->add_option(command, "--output-type", "-t", "json", "output file type", NULL, runner);
    command->add_option(command, "--output-file", "-o", "test_report.json", "output file path", NULL, runner);
    command->add_option(command, "--version", "-v", "false", "display version info", __option_version_action_callback, runner);
    command->add_argument(command, "", "test cases", NULL, NULL);
    command->set(command, "/Command/name", "ctest");
    command->set(command, "/Command/opaque", runner);

    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Runner * runner = (Test_Runner *)command->opaque;

    if (runner != NULL)
        object_destroy(runner);

    return 0;
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Ctest_Command, construct, __construct),
    Init_Nfunc_Entry(2, Ctest_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Ctest_Command, run_action, __action),
    Init_End___Entry(4, Ctest_Command),
};
REGISTER_APP_CMD("Ctest_Command", test_command_class_info);
