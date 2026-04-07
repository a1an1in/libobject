/**
 * @file Http_Plugin_MP4.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-07-20
 */
#include <libobject/argument/Application.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Server.h>
#include <libobject/net/http/Httpd_Command.h>
#include "Http_Plugin_MP4.h"

static int __handler_Http_Plugin_MP4(Request *req, Response *res, void *opaque)
{
    char *body = "hello_world\n";

    res->set_header(res, "Content-Type", "application/json");
    res->set_body(res, body, strlen(body));
    res->set_status_code(res, 200);
    dbg_str(DBG_VIP,"run handler_Http_Plugin_MP4");

    return 0;
}

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Http_Plugin_MP4 *plugin = (Http_Plugin_MP4 *)command;
    Httpd_Command *httpd = (Httpd_Command *)(command->opaque);
    Http_Server *server = httpd->server;

    dbg_str(DBG_VIP, "http test plugin deregistered handlers from http server!");
    server->deregister_handler(server, "GET", "/api/Http_Plugin_MP4");
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
        dbg_str(DBG_VIP, "test plugin run, help:%d!", ((Http_Plugin_MP4 *)command)->help);
        server->register_handler(server, "GET", "/api/Http_Plugin_MP4", __handler_Http_Plugin_MP4, command);
        dbg_str(DBG_VIP, "http test plugin registered handlers to http server!");
    } CATCH (ret) { }

    return 0;
}

static class_info_entry_t Http_Plugin_MP4_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Http_Plugin_MP4, construct, __construct),
    Init_Nfunc_Entry(2, Http_Plugin_MP4, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Http_Plugin_MP4, get_value, __get_value),
    Init_Vfunc_Entry(4, Http_Plugin_MP4, run_command, __run_command),
    Init_Str___Entry(5, Http_Plugin_MP4, option, NULL),
    Init_U32___Entry(6, Http_Plugin_MP4, help, 0),
    Init_End___Entry(7, Http_Plugin_MP4),
};
REGISTER_PLUGIN_CLASS(Http_Plugin_MP4, Http_Plugin_MP4_class_info);