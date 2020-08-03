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
    command->set(command, "/Command/name", "main");

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
    int ret = 0;

    TRY {
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

        if (selected_subcommand->run_option_actions != NULL) {
            selected_subcommand->run_option_actions(selected_subcommand);
        }
        if (selected_subcommand->run_argument_actions != NULL) {
            selected_subcommand->run_argument_actions(selected_subcommand);
        }
        if (selected_subcommand->run_action != NULL) {
            selected_subcommand->run_action(selected_subcommand);
        }
    } CATCH {
        ret = -1;
        dbg_str(DBG_ERROR, "Application catch error: func:%s, error_file: %s, error_line:%d, error_code:%d",
                ERROR_FUNC(), __func__, ERROR_LINE(), ERROR_CODE());
    }

    return ret;
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

    TRY {
        app = object_new(NULL, "Application", NULL);
        app->run(app, argc, argv);
    } CATCH {
        ret = -1;
        dbg_str(DBG_ERROR, "main catch error: func:%s, error_file: %s, error_line:%d, error_code:%d",
                ERROR_FUNC(), ERROR_FILE(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        dbg_str(ARG_SUC, "exit app!");
        object_destroy(app);
        libobject_exit();
    }

    return ret;
}


