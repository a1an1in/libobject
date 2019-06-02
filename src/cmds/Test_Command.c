/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/cmds/Test_Command.h>
#include <libobject/cmds/Test_Sub1_Command.h>
#include <libobject/cmds/Test_Sub2_Command.h>

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Command *test_command = (Test_Command *)command;

    if (test_command->option != NULL)
        object_destroy(test_command->option);

    return 0;
}

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Command, construct, __construct),
    Init_Nfunc_Entry(2, Test_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_Command, get_value, __get_value),
    Init_Str___Entry(4, Test_Command, option, NULL),
    Init_U32___Entry(5, Test_Command, help, 0),
    Init_End___Entry(6, Test_Command),
};
REGISTER_CLASS("Test_Command", test_command_class_info);

static int test_command(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Command *command = NULL;
    Command *subcmd1 = NULL;
    Command *subcmd2 = NULL;
    int ret = 0, help = 0;

    dbg_str(DBG_DETAIL, "allocator addr:%p", allocator);

    command = object_new(allocator, "Test_Command", NULL);
    subcmd1 = object_new(allocator, "Test_Sub1_Command", NULL);
    subcmd2 = object_new(allocator, "Test_Sub2_Command", NULL);

    help = 0;
    command->set(command, "/Test_Command/help", &help);
    command->set(command, "/Test_Command/option", "test command option");

    help = 1;
    subcmd1->set(subcmd1, "/Test_Sub1_Command/help", &help);
    subcmd1->set(subcmd1, "/Test_Sub1_Command/option", "test sub1 command option");

    help = 2;
    subcmd2->set(subcmd2, "/Test_Sub2_Command/help", &help);
    subcmd2->set(subcmd2, "/Test_Sub2_Command/option", "test sub2 command option");

    command->add_subcommand(command, subcmd1);
    command->add_subcommand(command, subcmd2);

    /*
     *dbg_str(DBG_DETAIL, "Test_Sub1_Command dump: %s",
     *        subcmd1->to_json(subcmd1));
     *dbg_str(DBG_DETAIL, "Test_Sub2_Command dump: %s",
     *        subcmd2->to_json(subcmd2));
     */
    dbg_str(DBG_DETAIL, "Test_Command dump: %s",
            command->to_json(command));

    object_destroy(subcmd1);
    object_destroy(subcmd2);
    object_destroy(command);

    ret = 1;

    return ret;

}
REGISTER_TEST_CMD(test_command);

static int test_new_command_with_init_data(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Command *command = NULL;
    Command *subcmd1 = NULL;
    Command *subcmd2 = NULL;
    int ret = 0, help = 0;
    char *init_data = "{\
            \"Test_Command\":{\
                \"option\": \"test cmd option\",\
                \"help\": 1\
            }\
        }";

    dbg_str(DBG_DETAIL, "allocator addr:%p", allocator);

    command = object_new(allocator, "Test_Command", init_data);


    dbg_str(DBG_DETAIL, "Test_Command dump: %s",
            command->to_json(command));

    object_destroy(command);

    return 1;
}
REGISTER_TEST_CMD(test_new_command_with_init_data);
