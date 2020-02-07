/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/argument/Version_Command.h>
#include <libobject/version.h>

static int __run_action(Command *command)
{
    dbg_str(DBG_DETAIL, "libobject version:%d.%d.%d.%d", 
            LIBOBJECT_VERSION_MAJOR, LIBOBJECT_VERSION_MINOR,
            LIBOBJECT_VERSION_MAINTENANCE, LIBOBJECT_VERSION_BUILD);

    return 1;
}

static int __construct(Command *command, char *init_str)
{
    command->set(command, "/Command/name", "version");

    return 0;
}

static int __deconstruct(Command *command)
{
    return 0;
}

static class_info_entry_t version_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Version_Command, construct, __construct),
    Init_Nfunc_Entry(2, Version_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Version_Command, run_action, __run_action),
    Init_End___Entry(4, Version_Command),
};
REGISTER_APP_CMD("Version_Command", version_command_class_info);
