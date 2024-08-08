/**
 * @file Test_Http_Plugin.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-07-20
 */
#include <libobject/argument/Application.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Server.h>
#include <libobject/net/http/Httpd_Command.h>
#include "Test_Http_Plugin.h"

static int __handler_test_http_plugin(Request *req, Response *res, void *opaque)
{
    char *body = "hello_world\n";

    res->set_header(res, "Content-Type", "application/json");
    res->set_body(res, body, strlen(body));
    res->set_status_code(res, 200);
    dbg_str(DBG_SUC,"run handler_test_http_plugin");

    return 0;
}

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Http_Plugin *plugin = (Test_Http_Plugin *)command;

    object_destroy(plugin->option);

    return 0;
}

static void *__get_value(Command *command,char *command_name, char *flag_name)
{
}

static int __run_command(Command *command)
{
    allocator_t *allocator = command->parent.allocator;
    Httpd_Command *httpd = (Httpd_Command *)(command->opaque);
    Http_Server *server = httpd->server;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "test plugin run, help:%d!", ((Test_Http_Plugin *)command)->help);
        server->register_handler(server, "GET", "/api/test_http_plugin", __handler_test_http_plugin, command);
    } CATCH (ret) { }

    return 0;
}

static class_info_entry_t Test_Http_Plugin_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Http_Plugin, construct, __construct),
    Init_Nfunc_Entry(2, Test_Http_Plugin, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_Http_Plugin, get_value, __get_value),
    Init_Vfunc_Entry(4, Test_Http_Plugin, run_command, __run_command),
    Init_Str___Entry(5, Test_Http_Plugin, option, NULL),
    Init_U32___Entry(6, Test_Http_Plugin, help, 0),
    Init_End___Entry(7, Test_Http_Plugin),
};
PLUGIN_DEFINE_CLASS_REGISTER(Test_Http_Plugin, Test_Http_Plugin_class_info);
PLUGIN_DEFINE_CLASS_DEREGISTER(Test_Http_Plugin);