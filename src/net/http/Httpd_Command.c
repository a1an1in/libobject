/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/core/io/File.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Argument.h>
#include <libobject/net/http/Client.h>
#include <libobject/net/url/Url.h>
#include "Httpd_Command.h"

static int __handler_hello_world(Request *req, Response *res, void *opaque)
{
    char *body = "hello_world\n";

    res->set_header(res, "Content-Type", "application/json");
    res->set_body(res, body, strlen(body));

    return 0;
}

static int __option_version_callback(Option *option, void *opaque)
{
    Http_Server *server = (Http_Server *)opaque;

    dbg_str(DBG_SUC,"option_version_action_callback:%s", "version:0.0.1");
}

static int __option_root_callback(Option *option, void *opaque)
{
    Http_Server *server = (Http_Server *)opaque;

    dbg_str(DBG_SUC,"option_root_action_callback:%s", STR2A(option->value));

    return server->set(server, "/Http_Server/root", STR2A(option->value));
}

static int __option_host_callback(Option *option, void *opaque)
{
    Http_Server *server = (Http_Server *)opaque;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));

    return server->set(server, "/Http_Server/host", STR2A(option->value));
}

static int __option_service_callback(Option *option, void *opaque)
{
    Http_Server *server = (Http_Server *)opaque;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));

    return server->set(server, "/Http_Server/service", STR2A(option->value));
}

static int __construct(Httpd_Command *command, char *init_str)
{
    Http_Server *server;
    Command *c = (Command *)command;

    server = object_new(c->parent.allocator, "Http_Server", NULL);
    server->set(server, "/Http_Server/root", "./webroot");
    command->server = server;

    server->register_handler(server, "GET", "/api/hello_world", __handler_hello_world, command);

    c->add_option(c, "--version", "-v", "false", "display version", __option_version_callback, server);
    c->add_option(c, "--root", "-r", "", "designate config file path", __option_root_callback, server);
    c->add_option(c, "--host", "-h", "", "set server ip address", __option_host_callback, server);
    c->add_option(c, "--service", "-s", "", "set server ip port", __option_service_callback, server);
    
    c->set(c, "/Command/name", "httpd");
    c->set(c, "/Command/description", "nginx is mimic nginx and is used in much the same way.");

    return 0;
}

static int __deconstruct(Httpd_Command *command)
{
    if (command->server != NULL) {
        object_destroy(command->server);
    }

    return 0;
}

static int __run_command(Httpd_Command *command)
{
    Http_Server *server = command->server;
    Command *c = (Command *)command;

    server->set(server, "host", "127.0.0.1");
    server->set(server, "service", "8081");
    server->start(server);

    sleep(60 * 60);

    return 1;
}

static class_info_entry_t httpd_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Httpd_Command, construct, __construct),
    Init_Nfunc_Entry(2, Httpd_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Httpd_Command, run_command, __run_command),
    Init_End___Entry(4, Httpd_Command),
};
REGISTER_APP_CMD(Httpd_Command, httpd_command_class_info);
