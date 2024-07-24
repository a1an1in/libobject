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
    res->set_status_code(res, 200);
    dbg_str(DBG_SUC,"run handler_hello_world");

    return 0;
}

static int __option_version_callback(Option *option, void *opaque)
{
    Http_Server *server = (Http_Server *)opaque;

    dbg_str(DBG_SUC,"option_version_action_callback:%s", "version:0.0.1");
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

static int __option_no_loop_callback(Option *option, void *opaque)
{
    Httpd_Command *c = (Httpd_Command *)opaque;

    c->loop_flag = 0;
    dbg_str(DBG_SUC,"run option_no_loop_callback");

    return 1;
}

static int __construct(Httpd_Command *command, char *init_str)
{
    Http_Server *server;
    Command *c = (Command *)command;

    server = object_new(c->parent.allocator, "Http_Server", NULL);
    server->set(server, "/Http_Server/root", "./webroot");
    command->server = server;
    command->loop_flag = 1;

    server->register_handler(server, "GET", "/api/hello_world", __handler_hello_world, command);

    c->add_option(c, "--version", "-v", "false", "display version", __option_version_callback, server);
    c->add_option(c, "--host", "-h", "", "set server ip address", __option_host_callback, server);
    c->add_option(c, "--service", "-s", "", "set server ip port", __option_service_callback, server);
    c->add_option(c, "--no-loop", "", "", "don't loop httpd", __option_no_loop_callback, command);
    
    c->set(c, "/Command/name", "httpd");
    c->set(c, "/Command/description", "nginx is mimic nginx and is used in much the same way.");

    return 0;
}

static int __deconstruct(Httpd_Command *command)
{
    object_destroy(command->server);

    return 0;
}

static int __run_command(Httpd_Command *command)
{
    Http_Server *server = command->server;
    Command *c = (Command *)command;
    char path[128] = {0};
    Application *app;
    int ret;

    TRY {
        app = get_global_application();
        snprintf(path, 128, "%s/%s", STR2A(app->root), "httpd/webroot");
        server->set(server, "/Http_Server/root", path);
        snprintf(path, 128, "%s/%s", STR2A(app->root), "httpd/plugins");
        dbg_str(DBG_SUC,"httpd root:%s", path);
        fs_mkdir(path, 0777);

        EXEC(command->load_plugins(command));

        // dbg_str(DBG_SUC,"httpd webroot:%s", STR2A(app->root));
        server->set(server, "host", "127.0.0.1");
        server->set(server, "service", "8081");
        server->start(server);

        if (command->loop_flag == 1) {
            sleep(60 * 60);
        }
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __load_plugins(Httpd_Command *command)
{
    Application *app;
    allocator_t *allocator;
    File *file;
    char path[128] = {0}, content[4096] = {0};
    char *out;
    Command *plugin;
    cjson_t *root, *item, *name, *config, *plugin_path;
    int ret, array_size = 0, i;

    TRY {
        app = get_global_application();
        snprintf(path, 128, "%s/%s/configs.json", STR2A(app->root), "httpd/plugins");

        if (!fs_is_exist(path)) { return 0; }
        dbg_str(DBG_SUC,"httpd root:%s", path);
        allocator = command->parent.parent.allocator;
        file = object_new(allocator, "File", NULL);
        file->open(file, path, "r+");
        ret = file->read(file, content, sizeof(content));
        memset(content + ret, 0, sizeof(content) - ret);

        root = cjson_parse(content);
        array_size = cjson_get_array_size(root);

        for (i = 0; i < array_size; i++) {
            item = cjson_get_object_item(root, i);
            name = cjson_get_object_item(item, "class_name");
            config = cjson_get_object_item(item, "config");
            plugin_path = cjson_get_object_item(item, "path");
            out = cjson_print(config);
            EXEC(app->load_plugin(app, name->valuestring, plugin_path->valuestring, out, command));
            free(out);
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(file);
        cjson_delete(root);
    }

    return ret;
}

static class_info_entry_t httpd_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Httpd_Command, construct, __construct),
    Init_Nfunc_Entry(2, Httpd_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Httpd_Command, run_command, __run_command),
    Init_Vfunc_Entry(4, Httpd_Command, load_plugins, __load_plugins),
    Init_End___Entry(5, Httpd_Command),
};
REGISTER_APP_CMD(Httpd_Command, httpd_command_class_info);
