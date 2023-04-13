/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/argument/Help_Command.h>

static int __run_command(Command *command)
{
    int argc, i;
    char **argv;
    Application *app = (Application *)command->opaque;

    printf("help command\n");

    // argc = command->argc;
    // argv = command->argv;

    // for (i = 0; i < argc; i++) {
    //     printf("argv[%d]: %s\n", i, argv[i]);
    // }
    app->help(app);

    return 1;
}
static int __construct(Command *command, char *init_str)
{
    command->set(command, "/Command/name", "help");
    command->set(command, "/Command/description", "show xtools help information.");

    return 0;
}

static int __deconstruct(Command *command)
{
    return 0;
}

static class_info_entry_t help_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Help_Command, construct, __construct),
    Init_Nfunc_Entry(2, Help_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Help_Command, run_command, __run_command),
    Init_End___Entry(4, Help_Command),
};
REGISTER_APP_CMD("Help_Command", help_command_class_info);
