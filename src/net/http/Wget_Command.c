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
#include <libobject/net/url/Url.h>
#include <libobject/mockery/mockery.h>
#include <libobject/net/http/Wget_Command.h>

//test command: ./sysroot/mac/bin/xtools wget http://mirror.hust.edu.cn/gnu/hello/hello-1.3.tar.gz

static int __get_output_document_name(Command *command, String *file);

static int __wget_request_callback(Response *resp, void *arg)
{
    Http_Client *client = (Http_Client *)arg;
    allocator_t *allocator = resp->obj.allocator;
    File *file;
    String *output_document;
    Command *command = client->command;
    String *path;

    output_document = object_new(command->parent.allocator, "String", NULL);
    __get_output_document_name(command, output_document);
    dbg_str(DBG_DETAIL,"output_document:%s", output_document->get_cstr(output_document));

    path = object_new(allocator, "String", NULL);
    path->assign(path, "./");
    path->append(path, output_document->get_cstr(output_document), -1);

    dbg_str(DBG_VIP,"request callback run, arg=%p", arg);
    dbg_str(DBG_VIP,"status code =%d", resp->status_code);
    dbg_str(DBG_VIP,"output file =%s", output_document->get_cstr(output_document));
    printf("status code =%d, output file =%s\n", resp->status_code, STR2A(output_document));

    file = object_new(allocator, "File", NULL);
    file->open(file, path->get_cstr(path), "w+");   
    file->write(file, resp->get_body(resp), resp->body->get_len(resp->body)); 
    file->close(file);

    resp->end_flag = 1;

    object_destroy(path);
    object_destroy(file);
    object_destroy(output_document);

}

static int __construct(Command *command, char *init_str)
{
    command->add_option(command, "--output-document", "-O", "", "output document name", NULL, NULL);
    command->add_option(command, "--directory-prefix", "-P", "./", "output directory", NULL, NULL);
    command->add_option(command, "--limit-rate", "", "10M", "limit download rate", NULL, NULL);
    command->add_option(command, "--continue", "-c", "true", "resumes transmission at break-points", NULL, NULL);
    command->add_option(command, "--input-file", "-i", "", "input url file", NULL, NULL);
    command->add_argument(command, "", "url", NULL, NULL);
    command->set(command, "/Command/name", "wget");
    command->set(command, "/Command/description", "wget is used to download resource of specified url.");

    return 0;
}

static int __deconstruct(Command *command)
{
    Wget_Command *test_command = (Wget_Command *)command;

    object_destroy(test_command->output_file);
    object_destroy(test_command->client);

    return 0;
}

static int __get_output_document_name(Command *command, String *file)
{
    Wget_Command *wget_command = (Wget_Command *)command;
    Option *o;
    Argument *arg;
    String *s;
    int count;

    o = command->get_option(command, "--output-document");
    if (o != NULL && o->value != NULL) {
        if (strcmp(o->value->get_cstr(o->value), "") != 0) {
            file->assign(file, o->value->get_cstr(o->value));
            return 1;
        }
    }

    arg = command->get_argment(command, 0);
    if (arg != NULL) {
        s = object_new(command->parent.allocator, "String", NULL);
        s->assign(s, arg->value->get_cstr(arg->value));
        count = s->split(s, "/", -1);
        file->assign(file, s->get_splited_cstr(s, count - 1));
        object_destroy(s);
        return 1;
    }

    return 0;
}

static int __run_command(Command *command)
{
    int count;
    Argument *arg;
    allocator_t *allocator = command->parent.allocator;
    Http_Client *client;
    Wget_Command *wget_command = (Wget_Command *)command;
    Request *req;
    Response *response;
    char *remote_host;
    char *remote_service = "80";
    Url *url;
    char *uri;

    count = command->args->count(command->args);
    dbg_str(DBG_VIP,"arg count =%d", count);
    
    if (count == 1) {
        arg = command->get_argment(command, 0);
        if (arg != NULL) {
            dbg_str(DBG_VIP,"url:%s", arg->value->get_cstr(arg->value));
            url = object_new(allocator, "Url", NULL);
            url->parse(url, arg->value->get_cstr(arg->value));
            url->info(url);
            remote_host = url->host->get_cstr(url->host);
            uri = url->path->get_cstr(url->path);
        }
    } else {
        dbg_str(DBG_VIP,"wget command format err, need url");
        return -1;
    }

    client = object_new(allocator, "Http_Client", NULL);
    wget_command->client = client;
    req = client->get_request(client);
    client->set(client, "remote_host", remote_host);
    client->set(client, "remote_service", remote_service);
    client->set(client, "command", command);

    req->set_method(req, "GET");
    req->set_uri(req, uri);
    req->set_http_version(req, "HTTP/1.1");
    req->set_header(req, "Host", remote_host);
    req->set_header(req, "User-Agent", "libobject-http-client");
    req->set_header(req, "Accept", "*/*");
    req->set_header(req, "Accept-Encoding", "identity");
    req->set_header(req, "Connection", "Keep-Alive");

    dbg_str(DBG_VIP,"request opaque=%p",  client);
    client->request(client, (int (*)(void *, void *))__wget_request_callback, client);

    response = client->get_response(client);
    while(response != NULL && response->end_flag != 1) {
        usleep(5000);
    }

    if (url != NULL) {
        object_destroy(url);
    }

    return 1;
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Wget_Command, construct, __construct),
    Init_Nfunc_Entry(2, Wget_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Wget_Command, run_command, __run_command),
    Init_Str___Entry(4, Wget_Command, output_file, NULL),
    Init_U32___Entry(5, Wget_Command, help, 0),
    Init_End___Entry(6, Wget_Command),
};
REGISTER_APP_CMD(Wget_Command, test_command_class_info);
