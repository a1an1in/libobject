/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/cmds/Test_Sub2_Command.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/config.h>

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_sub2_command_class_info[] = {
    Init_Obj___Entry(0, Test_Sub1_Command, parent),
    Init_Vfunc_Entry(1, Test_Sub2_Command, get_value, __get_value),
    Init_Str___Entry(2, Test_Sub2_Command, option, NULL),
    Init_U32___Entry(3, Test_Sub2_Command, help, 0),
    Init_End___Entry(4, Test_Sub2_Command),
};
REGISTER_CLASS("Test_Sub2_Command", test_sub2_command_class_info);

static int test_sub2_command(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    Command *command;
    int ret = 0;
    char buf[2048] = {0};

    c = cfg_alloc(allocator); 

    cfg_config_num(c, "/Test_Sub2_Command",  "help", 1) ;  
    cfg_config_str(c, "/Test_Sub2_Command",  "option", "sub2 command option") ;
    cfg_config_num(c, "/Test_Sub1_Command",  "help", 0) ;  
    cfg_config_str(c, "/Test_Sub1_Command",  "option", "sub1 command option") ;

    dbg_str(DBG_DETAIL, "Test_Sub2_Command dump: %s", c->buf);

    command  = object_new(allocator, "Test_Sub2_Command", c->buf);

    object_dump(command, "Test_Sub2_Command", buf, 2048);
    dbg_str(DBG_DETAIL, "Test_Sub2_Command dump: %s", buf);

    object_destroy(command);
    cfg_destroy(c);

    return ret;

}
REGISTER_TEST_FUNC(test_sub2_command);
