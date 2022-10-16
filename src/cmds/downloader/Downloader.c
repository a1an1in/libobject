/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include "Downloader.h"

static int __run_action(Downloader_Command *command)
{
    int argc, i;
    char **argv;
    Command *c = (Command *)command;

    dbg_str(ARG_DETAIL, "downloader command");

    argc = c->argc;
    argv = c->argv;

    for (i = 0; i < argc; i++) {
        dbg_str(ARG_DETAIL, "argv[%d]: %s", i, argv[i]);
    }

    dbg_str(ARG_DETAIL, "downloader command end");

    return 1;
}
static int __construct(Downloader_Command *command, char *init_str)
{
    command->set(command, "/Command/name", "downloader");

    return 0;
}

static int __deconstruct(Downloader_Command *command)
{
    return 0;
}

static class_info_entry_t downloader_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Downloader_Command, construct, __construct),
    Init_Nfunc_Entry(2, Downloader_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Downloader_Command, set, NULL),
    Init_Nfunc_Entry(4, Downloader_Command, get, NULL),
    Init_Vfunc_Entry(5, Downloader_Command, run_action, __run_action),
    Init_End___Entry(6, Downloader_Command),
};
REGISTER_APP_CMD("Downloader_Command", downloader_command_class_info);
