/**
 * @file Application.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/argument/Application.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include <libobject/core/io/fs_compat.h>
#include <libobject/concurrent/Producer.h>

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
            EXEC(selected_subcommand->run_option_actions(selected_subcommand));
        }
        if (selected_subcommand->run_argument_actions != NULL) {
            EXEC(selected_subcommand->run_argument_actions(selected_subcommand));
        }
        if (selected_subcommand->run_action != NULL) {
            EXEC(selected_subcommand->run_action(selected_subcommand));
        }
    } CATCH (ret) {
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


int libobject_init()
{
    #if (defined(WINDOWS_USER_MODE))
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 1), &wsa_data)) {
        dbg_str(NET_ERROR, "WSAStartup error");
        return -1;
    }
    #endif

    execute_ctor_funcs();

    core_init_fs();
    concurrent_init_producer();
    concurrent_init_event_base();
    exception_init();
}

int libobject_destroy()
{
    concurrent_destroy_event_base();
    concurrent_destroy_producer();

    //#if (defined(WINDOWS_USER_MODE))
    //    WSACleanup();
    //#endif
    core_destroy_fs();
    execute_dtor_funcs();
}

int app(int argc, char *argv[])
{
    Application *app;
    int ret = 0;

    TRY {
        EXEC(libobject_init());
        app = object_new(NULL, "Application", NULL);
        EXEC(app->run(app, argc, argv));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "main catch error: func:%s, error_file: %s, error_line:%d, error_code:%d",
                ERROR_FUNC(), ERROR_FILE(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(app);
        libobject_destroy();
        dbg_str(ARG_SUC, "exit app!");
    }

    return ret;
}


