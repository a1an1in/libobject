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
#include <libobject/core/try.h>

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
    Command *default_subcommand;


    for (i = 0; i < app_command_count; i++) {
        app->add_subcommand(app, app_commands[i]);
    }

    command->set_args(command, argc, (char **)argv);
    command->parse_args(command);

    default_subcommand = app->get_subcommand(app, "help");

    selected_subcommand = command->selected_subcommand;
    if (selected_subcommand == NULL) {
        selected_subcommand = default_subcommand;
    }
    selected_subcommand->run_action(selected_subcommand);

    return 0;
}

static class_info_entry_t application_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Application, construct, __construct),
    Init_Nfunc_Entry(2, Application, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Application, add_subcommand, NULL),
    Init_Vfunc_Entry(4, Application, get_subcommand, NULL),
    Init_Vfunc_Entry(5, Application, to_json, NULL),
    Init_Nfunc_Entry(6, Application, run, __run),
    Init_End___Entry(7, Application),
};
REGISTER_CLASS("Application", application_class_info);

int app_register_cmd(char *cmd)
{
    dbg_str(ARG_DETAIL, "register app cmd: %s", cmd);
    app_commands[app_command_count++] = cmd;
}

int app(int argc, char *argv[])
{
    Application *app;
    int ret = 0;

    libobject_init();
    exception_init();

    app = object_new(NULL, "Application", NULL);

    TRY {
        app->run(app, argc, argv);
    } CATCH (ret) {
        dbg_str(ARG_ERROR, "app run errorno: %d, error_func:%s, error_file: %s, error_line:%d",
                ret, ERROR_FUNC(), ERROR_FILE(), ERROR_LINE());
    } FINALLY {
        dbg_str(ARG_SUC, "exit app!");
        object_destroy(app);
        libobject_exit();
    }
    ENDTRY;
}


