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


static int __option_set_event_thread_service_callback(Option *option, void *opaque)
{
    dbg_str(DBG_SUC,"option_set_event_thread_service_callback:%s", STR2A(option->value));
    return 1;
}

static int __option_set_event_signal_service_callback(Option *option, void *opaque)
{
    dbg_str(DBG_SUC,"option_set_event_signal_service_callback:%s", STR2A(option->value));
    return 1;
}

static int __construct(Application *app, char *init_str)
{
    Command *command = (Command *)app;
    command->set(command, "/Command/name", "xtool");
    command->add_option(command, "--event-thread-service", "", "11110", "event-thread-service address", __option_set_event_thread_service_callback, NULL);
    command->add_option(command, "--event-signal-service", "", "11120", "event-signal-service address", __option_set_event_signal_service_callback, NULL);

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
    Command *subcmd;
    Command *default_subcmd;
    int ret = 0;

    TRY {
        for (i = 0; i < app_command_count; i++) {
            app->add_subcommand(app, app_commands[i]);
        }

        command->set_args(command, argc, (char **)argv);
        command->parse_args(command);
        EXEC(command->run_option_actions(command));

        /*
         * init producer at here, which means we can't use producer 
         * in constructor.
         */
        EXEC(command->run_command(command)); 

        default_subcmd = app->get_subcommand(app, "help");
        subcmd = command->selected_subcommand;
        if (subcmd == NULL) {
            subcmd = default_subcmd;
        }

        if (subcmd->run_option_actions != NULL) {
            EXEC(subcmd->run_option_actions(subcmd));
        }
        if (subcmd->run_argument_actions != NULL) {
            EXEC(subcmd->run_argument_actions(subcmd));
        }
        if (subcmd->run_command != NULL) {
            EXEC(subcmd->run_command(subcmd));
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "Application catch error: func:%s, error_file: %s, error_line:%d, error_code:%d",
                ERROR_FUNC(), __func__, ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __run_command(Application *app)
{
    Command *command = (Command *)app;
    Option *option;
    char *event_thread_service, *event_signal_service;
    int ret;

    TRY {
        dbg_str(DBG_SUC, "run Application command");
        option = command->get_option(command, "--event-thread-service");
        event_thread_service = STR2A(option->value);
        option = command->get_option(command, "--event-signal-service");
        event_signal_service = STR2A(option->value);

        EXEC(producer_init_default_instance(event_thread_service, event_signal_service));
        EXEC(event_base_init_default_instance());
    } CATCH (ret) {
    }

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
    Init_Nfunc_Entry(7, Application, run_command, __run_command),
    Init_End___Entry(8, Application),
};
REGISTER_CLASS("Application", application_class_info);

int app_register_cmd(char *cmd)
{
    dbg_str(ARG_DETAIL, "register app cmd: %s", cmd);
    app_commands[app_command_count++] = cmd;
}

int libobject_init()
{
    int ret;

    TRY {
        #if (defined(WINDOWS_USER_MODE))
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 1), &wsa_data)) {
            dbg_str(NET_ERROR, "WSAStartup error");
            return -1;
        }
        #endif
        EXEC(execute_ctor_funcs());
        EXEC(core_init_fs());
        // EXEC(producer_init_default_instance("11110", "11120"));
        // EXEC(event_base_init_default_instance());

        exception_init();
    } CATCH (ret) {
    }
    return ret;
}

int libobject_destroy()
{
    int ret;

    TRY {
        EXEC(event_base_destroy_default_instance());
        EXEC(producer_destroy_default_instance());

        //#if (defined(WINDOWS_USER_MODE))
        //    WSACleanup();
        //#endif
        EXEC(core_destroy_fs());
        EXEC(execute_dtor_funcs());
    } CATCH (ret) {
    }
    
    return ret;
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


