/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Ctest_Command.h>
#include <libobject/argument/Application.h>

static int __construct(Command *command, char *init_str)
{

    command->add_option(command, "--all", "-a", NULL, "print all tests", NULL);
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
    Init_End___Entry(3, Ctest_Command),
};
REGISTER_CLASS("Ctest_Command", test_command_class_info);
REGISTER_APP_CMD("Ctest_Command");
