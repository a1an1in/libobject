/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Test_Sub2_Command.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/String.h>

static int __construct(Command *command, char *init_str)
{
    command->add_option(command, "--help", "-h", NULL, "help2", NULL, NULL);
    command->add_option(command, "--output-type", "-t", "json", "output file type", NULL, NULL);
    command->add_option(command, "--output-file", "-o", "test_report.json", "output file path", NULL, NULL);
    command->set(command, "/Command/name", "Test_Sub2");

    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Sub2_Command *test_sub2_command = (Test_Sub2_Command *)command;

    if (test_sub2_command->option != NULL)
        object_destroy(test_sub2_command->option);
    return 0;
}

static void * __get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_sub2_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Sub2_Command, construct, __construct),
    Init_Nfunc_Entry(2, Test_Sub2_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Test_Sub2_Command, get, NULL),
    Init_Nfunc_Entry(4, Test_Sub2_Command, set, NULL),
    Init_Vfunc_Entry(5, Test_Sub2_Command, get_value, __get_value),
    Init_Str___Entry(6, Test_Sub2_Command, option, NULL),
    Init_U32___Entry(7, Test_Sub2_Command, help, 0),
    Init_End___Entry(8, Test_Sub2_Command),
};
REGISTER_CLASS("Test_Sub2_Command", test_sub2_command_class_info);

static int test_sub2_command(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command = NULL;
    int ret = 0, help = 0;


    command = object_new(allocator, "Test_Sub2_Command", NULL);

    /*
     *command->set(command, "/Test_Sub1_Command/help", &help);
     *command->set(command, "/Test_Sub1_Command/option", "sub1 command option");
     */

    help = 1;
    command->set(command, "/Test_Sub2_Command/help", &help);
    command->set(command, "/Test_Sub2_Command/option", "sub2 command option");

    dbg_str(DBG_DETAIL, "Test_Sub2_Command dump: %s",
            command->to_json(command));

    object_destroy(command);

    return ret;

}
REGISTER_TEST_CMD(test_sub2_command);
