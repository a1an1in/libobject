#include <math.h>
#include <libobject/core/io/File.h>
#include <libobject/net/http/Client.h>
#include <libobject/net/url/Url.h>
#include <libobject/net/http/Server.h>
#include <libobject/net/http/Httpd_Command.h>

static int __handler_hello_world(Request *req, Response *res, void *opaque)
{
    char *body = "hello_world\n";

    res->set_header(res, "Content-Type", "application/json");
    res->set_body(res, body, strlen(body));
    res->set_status_code(res, 200);
    dbg_str(DBG_VIP,"run hello_world handler!");

    return 0;
}

static Form_Data_Type __get_multipart_form_data_type(String *str)
{
    if (strstr(STR2A(str), "image") != NULL) {
        return E_FORM_DATA_TYPE_IMAGE;
    } else if (strstr(STR2A(str), "image") != NULL) {
        return E_FORM_DATA_TYPE_VIDEO;
    } else {
        return E_FORM_DATA_TYPE_UNKNOW;
    }
}

static int __is_body_multipart_form_data(Request *request)
{
    Map *headers = request->headers;
    char *value = NULL;
    int ret = 0;

    TRY {
        headers->search(headers, "content-type", (void **)&value);
        THROW_IF(value == NULL, 0);
        THROW_IF((strstr(value, "multipart/form-data") == NULL), 0);
        THROW(1);
    } CATCH (ret) {
    }

    return ret;
}

static int __read_form_data(Request *request)
{
    Object_Chain *chain = request->chain;
    Buffer *buffer = request->body;
    Http_Server *server = (Http_Server *)request->server;
    char *dir_name[E_FORM_DATA_TYPE_UNKNOW] = { "image", "video" };
    char *regex = "filename=\"([a-z0-9A-Z_.,!&=-]+)\"";
    char filename[MAX_FILE_NAME_LEN] = {0};
    char path[MAX_FILE_NAME_LEN] = {0};
    int len, start = 0, ret = 1;
    String *str;
    File *file;
    Form_Data_Type form_data_type;

    TRY {
        file = chain->new(chain, "File", NULL);
        while (buffer->get_len(buffer) > 2) {
            len = buffer->get_needle_offset(buffer, "\r\n", 2);
            if (len < 0) break;

            str = chain->new(chain, "String", NULL);
            THROW_IF(str == NULL, -1);
            EXEC(buffer->read_to_string(buffer, str, len + 2));
            str_to_lower(STR2A(str));

            if (str->equal(str, "\r\n") == 1) {
                len = buffer->get_needle_offset(buffer, "\r\n--", 4);
                snprintf(path + strlen(path), MAX_FILE_NAME_LEN, "/%s", filename);
                dbg_str(NET_SUC, "write file len:%d, path:%s", len, path);
                EXEC(file->open(file, path, "w+"));
                file->write(file, buffer->addr + buffer->r_offset, len);
                file->close(file);
                buffer->r_offset += (len + 4);
                request->file = file;
                continue;
            }
            str->replace(str, "\r\n", "", -1);

            if (strstr(STR2A(str), "content-disposition") != NULL) {
                EXEC(str->get_substring(str, regex, 0, &start, &len));
                THROW_IF(start > str->get_len(str), -1);
                str->value[start + len] = '\0';
                snprintf(filename, MAX_FILE_NAME_LEN, "%s", str->value + start);
                dbg_str(NET_SUC, "form data filename:%s", filename);
            } else if (strstr(STR2A(str), "content-type") != NULL) {
                dbg_str(NET_SUC, "Line:%s", STR2A(str));
                form_data_type = __get_multipart_form_data_type(str);
                THROW_IF(form_data_type == E_FORM_DATA_TYPE_UNKNOW, -1);
                snprintf(path, MAX_FILE_NAME_LEN, "%s/%s", STR2A(server->root), dir_name[form_data_type]);
                dbg_str(NET_SUC, "path:%s", path);
                ret = file->is_exist(file, path);
                if (ret == 0) {
                    EXEC(file->mkdir(file, path, 0777));
                }
            } else {
                dbg_str(NET_SUC, "ignore Line:%s", STR2A(str));
            }
        }
    } CATCH (ret) {
        dbg_str(NET_ERROR, "len:%d, filename:%s", len, filename);
        dbg_str(NET_ERROR, "str:%s", STR2A(str));
    }
    return ret;
}

static int __handler_upload(Request *req, Response *res, void *opaque)
{
    Object_Chain *chain = res->chain;
    Http_Server *server = (Http_Server *)req->server;
    int ret = 1, i, num, start = 0, len = 0;
    char regex[1024] = {0};
    char body[1024] = {0};
    File *file;
    String *str;

    TRY {
        THROW_IF(__is_body_multipart_form_data(req) == 0, -1); //如果没有form_data, 直接结束read_body
        EXEC(__read_form_data(req)); /* 如果有form_data需要解析 */

        snprintf(regex, 1024, "%s(.*)+", STR2A(server->root));
        str = chain->new(chain, "String", NULL);
        str->assign(str, STR2A(req->file->name));
        str->get_substring(str, regex, 0, &start, &len);
        THROW_IF(len == 0, -1);
        snprintf(body, 1024, "{\"retCode\":%d, \"url\":\"%s\"}", 1, str->value + start);
        res->set_body(res, body, strlen(body));
        res->set_status_code(res, 200);
        dbg_str(DBG_VIP, "upload sucess, file name:%s", str->value + start);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "upload failed");
        res->set_status_code(res, 500);
    }

    return ret;
}

int http_server_register_builtin_handlers(Httpd_Command *command)
{
    Http_Server *server = command->server;

    server->register_handler(server, "GET", "/api/hello_world", __handler_hello_world, command);
    server->register_handler(server, "POST", "/api/upload", __handler_upload, command);

    return 1;
}

int http_server_deregister_handlers(Httpd_Command *command)
{
    Http_Server *server = command->server;

    server->deregister_handler(server, "GET", "/api/hello_world");
    server->deregister_handler(server, "POST", "/api/upload");

    return 1;
}