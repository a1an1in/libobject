/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include "Curl_Command.h"
#include <libobject/net/http/Client.h>
#include <libobject/core/io/File.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Argument.h>
#include <libobject/net/url/Url.h>

static char *__trim(char *str)
{
    char *p = str;
    char *p1;

    if (p) {
        p1 = p + strlen(str) - 1;
        while (*p && isspace(*p)) p++;
        while (p1 > p && isspace(*p1)) *p1-- = '\0';
    }

    return p;
}

static int __curl_request_callback(Response *resp, void *arg)
{
    Http_Client *client = (Http_Client *)arg;
    allocator_t *allocator = resp->obj.allocator;

    dbg_str(DBG_SUC,"curl request callback");
    dbg_str(DBG_SUC,"status code =%d", resp->status_code);
    dbg_str(DBG_DETAIL,"body:%s", resp->get_body(resp));
    resp->end_flag = 1;

    return 0;
}

static int __option_data_action_callback(Option *option, void *opaque)
{
    Request *req = (Request *)opaque;
    char *body;

    body = option->value->get_cstr(option->value);
    dbg_str(DBG_DETAIL,"set body, %s", body);
    req->set_body(req, body, strlen(body));
}

static int __option_header_action_callback(Option *option, void *opaque)
{
    Request *req = (Request *)opaque;
    Object_Chain *chain = req->chain;
    String *header;
    char *key, *value;
    int count, i;

    if (option->multi_value_flag == 1) {
        count = option->value->split(option->value, ";", -1);
        for (i = 0; i < count; i++) {
            header = chain->new(chain, "String", NULL);
            value = option->value->get_splited_cstr(option->value, i);
            header->assign(header, value);
            header->split(header, ":", 1);
            key = header->get_splited_cstr(header, 0);
            value = header->get_splited_cstr(header, 1);

            key   = __trim(key);
            value = __trim(value);
            req->set_header(req, key, value);
            dbg_str(DBG_DETAIL,"set header, key:%s, value:%s", key, value);
        }
    } else {
        header = chain->new(chain, "String", NULL);
        header->assign(header, option->value->get_cstr(option->value));
        header->split(header, ":", 1);
        key = header->get_splited_cstr(header, 0);
        value = header->get_splited_cstr(header, 1);
        req->set_header(req, key, value);
        dbg_str(DBG_DETAIL,"set header, key:%s, value:%s", key, value);
    }

    return 0;
}

static int __option_head_action_callback(Option *option, void *opaque)
{
    dbg_str(DBG_SUC,"option_head_action_callback");
}

static int __option_output_action_callback(Option *option, void *opaque)
{
    dbg_str(DBG_SUC,"option_output_action_callback");
}

static int __option_range_action_callback(Option *option, void *opaque)
{
    dbg_str(DBG_SUC,"option_range_action_callback");
}

static int __option_request_action_callback(Option *option, void *opaque)
{
    Request *req = (Request *)opaque;

    dbg_str(DBG_DETAIL,"set request method:%s", 
            option->value->get_cstr(option->value));
    req->set_method(req, option->value->get_cstr(option->value));
}

static int __argument_url_action_callback(Argument *arg, void *opaque)
{
    Url *url;
    Http_Client *client = (Http_Client *)opaque;
    Request *req;
    Object_Chain *chain;
    char *remote_service;
    char *remote_host;

    req = client->get_request(client);
    chain = req->chain;

    dbg_str(DBG_DETAIL,"url:%s", arg->value->get_cstr(arg->value));

    url = chain->new(chain, "Url", NULL);
    url->parse(url, arg->value->get_cstr(arg->value));
    url->info(url);

    remote_host = url->host->get_cstr(url->host);
    remote_service = url->port->get_cstr(url->port);
    if (strcmp(remote_service, "") == 0) {
        remote_host = "80";
    }
    dbg_str(DBG_DETAIL,"set remote_host:%s", remote_host);
    dbg_str(DBG_DETAIL,"set remote_service:%s", remote_service);

    client->set(client, "remote_host", remote_host);
    client->set(client, "remote_service", remote_service);

    req->set_uri(req, url->path->get_cstr(url->path));
    req->set_header(req, "Host", remote_host);

    return 0;
}

static int __construct(Command *command, char *init_str)
{
    Curl_Command *curl = (Curl_Command *)command;
    Http_Client *client;
    Request *req;

    client = object_new(command->parent.allocator, "Http_Client", NULL);
    curl->client = client;
    req = client->get_request(client);

    command->add_option(command, "--data", "-d", "", "http post方式传送数据", __option_data_action_callback, req);
    command->add_option(command, "--header", "-H", "", "自定义头信息传递给服务器", __option_header_action_callback, req);
    command->add_option(command, "--head", "-I", NULL, "只显示响应报文首部信息", __option_head_action_callback, req);
    command->add_option(command, "--output", "-o", "", "把输出写到该文件中", __option_output_action_callback, req);
    command->add_option(command, "--range", "-r", "", "检索来自HTTP/1.1或FTP服务器字节范围", __option_range_action_callback, req);
    command->add_option(command, "--request", "-X", "GET", "指定什么命令", __option_request_action_callback, req);
    command->add_argument(command, "", "url", __argument_url_action_callback, client);
    command->set(command, "/Command/name", "curl");
    command->set(command, "/Command/description", "curl is mimic CURL and is used in much the same way.");

    return 0;
}

static int __deconstruct(Command *command)
{
    Curl_Command *curl = (Curl_Command *)command;

    if (curl->output_file != NULL)
        object_destroy(curl->output_file);
    if (curl->client != NULL) {
        object_destroy(curl->client);
    }

    return 0;
}

static int __run_command(Command *command)
{
    Http_Client *client;
    Curl_Command *curl = (Curl_Command *)command;
    Request *req;
    Response *resp;
    int count;

    /*
     *command->run_option_actions(command);
     *command->run_argument_actions(command);
     */

    count = command->args->count(command->args);
    if (count != 1) {
        dbg_str(DBG_SUC,"curl command format err, need url");
        return -1;
    }

    dbg_str(DBG_SUC,"arg count =%d", count);

    client = curl->client;
    req = client->get_request(client);

    req->set_http_version(req, "HTTP/1.1");
    req->set_header(req, "User-Agent", "libobject-http-client");
    req->set_header(req, "Accept", "*/*");
    req->set_header(req, "Accept-Encoding", "identity");
    req->set_header(req, "Connection", "Keep-Alive");
    /*
     *req->write(req);
     */

    client->request(client, (int (*)(void *, void *))
                            __curl_request_callback, client);

    resp = client->get_response(client);
    while(resp != NULL && resp->end_flag != 1) {
        usleep(1000);
    }
    dbg_str(DBG_SUC,"status code =%d", resp->status_code);

    return 1;
}

static class_info_entry_t curl_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Curl_Command, construct, __construct),
    Init_Nfunc_Entry(2, Curl_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Curl_Command, run_command, __run_command),
    Init_Str___Entry(4, Curl_Command, output_file, NULL),
    Init_U32___Entry(5, Curl_Command, help, 0),
    Init_End___Entry(6, Curl_Command),
};
REGISTER_APP_CMD(Curl_Command, curl_command_class_info);
