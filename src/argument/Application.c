/**
 * @file Application.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/argument/Application.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/libobject.h>

#define MAX_APP_COMMANDS_COUNT 1024
static char *app_commands[MAX_APP_COMMANDS_COUNT];
static int app_command_count;
#undef MAX_APP_COMMANDS_COUNT

static int __construct(Application *app, char *init_str)
{
    Command *command = (Command *)app;
    command->set(command, "/Command/name", "cean");
    return 0;
}

static int __deconstruct(Application *app)
{
    return 0;
}

static int __run(Application *app, int argc, char *argv[])
{
    char *json;
    int i = 0;
    Command *command = (Command *)app;
    Command *selected_subcommand;


    for (i = 0; i < app_command_count; i++) {
        app->add_subcommand(app, app_commands[i]);
    }

    command->set_args(command, argc, (char **)argv);
    command->parse_args(command);

    selected_subcommand = command->selected_subcommand;
    selected_subcommand->action(selected_subcommand);

    /*
     *json = app->to_json(app);
     *dbg_str(DBG_DETAIL, "app to json: %s", json);
     */
}

static class_info_entry_t application_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Application, construct, __construct),
    Init_Nfunc_Entry(2, Application, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Application, add_subcommand, NULL),
    Init_Vfunc_Entry(4, Application, to_json, NULL),
    Init_Nfunc_Entry(5, Application, run, __run),
    Init_End___Entry(6, Application),
};
REGISTER_CLASS("Application", application_class_info);

int app_register_cmd(char *cmd)
{
    dbg_str(DBG_SUC, "register app cmd: %s", cmd);
    app_commands[app_command_count++] = cmd;
}

int app(int argc, char *argv[])
{
    Application *app;

    libobject_init();

    app = object_new(NULL, "Application", NULL);
    app->run(app, argc, argv);
    object_destroy(app);

    libobject_exit();
}

