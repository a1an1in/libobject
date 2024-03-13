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
#include <libobject/core/io/file_system_api.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/version.h>

#if (defined(WINDOWS_USER_MODE))
#include <winsock2.h>
#endif

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

static int __option_version_callback(Option *option, void *opaque)
{
    if (option->set_flag == 1) {
        printf("version:%s\n", PROJECT_VERSION);
        exit(0);
    }
    return 1;
}

static int __option_help_callback(Option *option, void *opaque)
{
    Application *app = (Application *)opaque;

    if (option->set_flag == 1) {
        dbg_str(DBG_SUC,"xtools help");
        app->help(app);            
        printf("Run 'xtools COMMAND --help' for more information on a command.\n\n");
        exit(0);
    }
    
    return 1;
}

static int __option_log_level_callback(Option *option, void *opaque)
{
    Application *app = (Application *)opaque;
    uint32_t value, bussiness_num, level;
    int i, ret;

    TRY {
        RETURN_IF(option->set_flag != 1, 0);
        THROW_IF(option->value == NULL, -1);

        // value = atoi(STR2A(option->value));
        value = str_hex_to_int(STR2A(option->value));
        bussiness_num = (value >> 4) & 0xffffff;
        level = value & 0xf;

        dbg_str(DBG_VIP, "application set log level, value:%s, bussiness_num:%x, log level:%x", 
                STR2A(option->value), bussiness_num, level);

        if (bussiness_num == 0) {
            debugger_set_all_businesses_level(debugger_gp, 1, atoi(STR2A(option->value)));
            THROW(1);
        }

        for(i = 0; i < MAX_DEBUG_BUSINESS_NUM; i++) {
            if ((bussiness_num >> i) & 1) debugger_set_business(debugger_gp, i, 1, level);
        }
    } CATCH (ret) { }
    
    return ret;
}

static int __construct(Application *app, char *init_str)
{
    Command *command = (Command *)app;
    command->set(command, "/Command/name", "xtools");
    command->add_option(command, "--event-thread-service", "", "11110", "event-thread-service address",
                        __option_set_event_thread_service_callback, NULL);
    command->add_option(command, "--event-signal-service", "", "11120", "event-signal-service address",
                        __option_set_event_signal_service_callback, NULL);
    command->add_option(command, "--version", "-v", NULL, "show version of xtools",
                        __option_version_callback, NULL);
    command->add_option(command, "--help", "-h", NULL, "help for xtools",
                        __option_help_callback, app);
    command->add_option(command, "--log-level", "", "5", "setting log display level, the default value is 6.",
                        __option_log_level_callback, app);

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
        default_subcmd->opaque = app;
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
    } CATCH (ret) {}

    return ret;
}

static int __run_command(Application *app)
{
    Command *command = (Command *)app;
    Option *option;
    char *event_thread_service, *event_signal_service;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "run Application command");
        option = command->get_option(command, "--event-thread-service");
        event_thread_service = STR2A(option->value);
        option = command->get_option(command, "--event-signal-service");
        event_signal_service = STR2A(option->value);

        EXEC(producer_init_default_instance(event_thread_service, event_signal_service));
        EXEC(event_base_init_default_instance());
        EXEC(stub_admin_init_default_instance());
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
    Init_Nfunc_Entry(8, Application, help, NULL),
    Init_End___Entry(9, Application),
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
        EXEC(fs_init());
        EXEC(exception_init());
        debugger_set_all_businesses_level(debugger_gp, 1, 3);
    } CATCH (ret) {
    }
    return ret;
}

int libobject_destroy()
{
    int ret;

    TRY {
        EXEC(stub_admin_destroy_default_instance());
        EXEC(event_base_destroy_default_instance());
        EXEC(producer_destroy_default_instance());

        #if (defined(WINDOWS_USER_MODE))
            WSACleanup();
        #endif

        EXEC(fs_destroy());
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
    } CATCH (ret) {} FINALLY {
        object_destroy(app);
        libobject_destroy();
        dbg_str(DBG_VIP, "exit app!");
    }

    return ret;
}


