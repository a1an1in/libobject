#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/http/Server.h>
#include <libobject/mockery/mockery.h>
#include <libobject/argument/Command.h>
#include <libobject/net/http/Client.h>
#include <libobject/net/http/Curl_Command.h>
#include <libobject/net/http/Wget_Command.h>

static int test_http_deamon_built_in_api()
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command, *curl;
    Http_Client *client;
    Response *resp;
    char *argv[2] = {"httpd", "--no-loop"};
    char *curl_argv[4] = {"curl", "-X", "GET", "http://127.0.0.1:8081/api/hello_world"};
    int ret, count = 0;
    
    TRY {
        command = object_new(allocator, "Httpd_Command", NULL);
        command->set_args(command, 2, (char **)argv);
        command->parse_args(command);
        EXEC(command->run_option_actions(command));
        EXEC(command->run_command(command)); 

        curl = object_new(allocator, "Curl_Command", NULL);
        curl->set_args(curl, 4, (char **)curl_argv);
        curl->parse_args(curl);
        EXEC(curl->run_option_actions(curl));
        EXEC(curl->run_argument_actions(curl));
        EXEC(curl->run_command(curl));
        sleep(1);
        client = ((Curl_Command *)curl)->client;
        resp = client->get_response(client);
        THROW_IF(resp->status_code != 200, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(curl);
        object_destroy(command);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_http_deamon_built_in_api);


static int test_http_deamon_plugin_api()
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command, *curl;
    Http_Client *client;
    Response *resp;
    char *argv[2] = {"httpd", "--no-loop"};
    char *curl_argv[4] = {"curl", "-X", "GET", "http://127.0.0.1:8081/api/test_http_plugin"};
    int ret, count = 0;
    char path[128] = {0};
    
    TRY {
        snprintf(path, 128, "%s/%s/http_plugin.json", "~/.xtools", "httpd/plugins");
        if (fs_is_exist(path) != 1) {
            system("cp ./doc/http/http_plugin.json ~/.xtools/httpd/plugins/http_plugin.json");
        }
        command = object_new(allocator, "Httpd_Command", NULL);
        command->set_args(command, 2, (char **)argv);
        command->parse_args(command);
        EXEC(command->run_option_actions(command));
        EXEC(command->run_command(command)); 

        curl = object_new(allocator, "Curl_Command", NULL);
        curl->set_args(curl, 4, (char **)curl_argv);
        curl->parse_args(curl);
        EXEC(curl->run_option_actions(curl));
        EXEC(curl->run_argument_actions(curl));
        EXEC(curl->run_command(curl));
        sleep(1);
        client = ((Curl_Command *)curl)->client;
        resp = client->get_response(client);
        THROW_IF(resp->status_code != 200, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(curl);
        object_destroy(command);
    }

    return ret;
}
#if (!defined(WINDOWS_USER_MODE))
REGISTER_TEST_FUNC(test_http_deamon_plugin_api);
#endif

static int test_http_wget()
{
    allocator_t *allocator = allocator_get_default_instance();
    Command *command, *curl;
    Wget_Command *wget_command;
    Http_Client *client;
    Response *resp;
    char *wget_argv[2] = {"wget", "http://mirrors.hust.edu.cn/gnu/hello/hello-1.3.tar.gz"};
    int ret, count = 0;
    
    TRY {
        command = object_new(allocator, "Wget_Command", NULL);
        command->set_args(command, 2, (char **)wget_argv);
        command->parse_args(command);
        EXEC(command->run_option_actions(command));
        EXEC(command->run_command(command)); 

        wget_command = (Wget_Command *)command;
        client = wget_command->client;
        THROW_IF(client->resp->status_code != 200, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(command);
        fs_rmfile("./hello-1.3.tar.gz");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_http_wget);