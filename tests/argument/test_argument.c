/**
 * @file test_argument.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-05-14
 */
#include <libobject/argument/Application.h>
#include <libobject/argument/Test_Command.h>
#include <libobject/argument/Test_Sub1_Command.h>
#include <libobject/argument/Test_Sub2_Command.h>
#include <libobject/mockery/mockery.h>

static int test_marshal_command(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command = NULL;
    Command *subcmd1 = NULL;
    Command *subcmd2 = NULL;
    int ret = 0, help = 0;

    dbg_str(ARG_DETAIL, "allocator addr:%p", allocator);

    command = object_new(allocator, "Test_Command", NULL);

    dbg_str(ARG_DETAIL, "Test_Command dump: %s", command->to_json(command));

    object_destroy(command);

    ret = 1;

    return ret;

}
REGISTER_TEST_CMD(test_marshal_command);

static int test_unmarshal_command(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command = NULL;
    Command *subcmd1 = NULL;
    Command *subcmd2 = NULL;
    int ret = 0, help = 0;
    char *init_data = "\
        {\
            \"Test_Command\": {\
                \"Command\": {\
                    \"subcommands\": [{\
                        \"option\":	\"test sub1 command option\",\
                        \"help\":	1\
                    }, {\
                        \"option\":	\"test sub2 command option\",\
                        \"help\":	2\
                    }]\
                },\
                \"option\": \"test cmd option\",\
                \"help\": 0\
            }\
        }";

    dbg_str(ARG_DETAIL, "allocator addr:%p", allocator);

    command = object_new(allocator, "Test_Command", init_data);

    dbg_str(ARG_DETAIL, "Test_Command dump: %s", command->to_json(command));

    object_destroy(command);

    return 1;
}
REGISTER_TEST_CMD(test_unmarshal_command);

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