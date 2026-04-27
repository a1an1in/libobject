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
    char *dir_name[E_FORM_DATA_TYPE_UNKNOW] = { "images", "videos" };
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

int __handler_bad_request(Request *req, Response *res, void *opaque)
{
    char body[1024];

    res->set_header(res, "Content-Type", "text/html");
    res->set_status_code(res, 400);

    sprintf(body, "Your browser sent a bad request !!");

    res->set_body(res, body, strlen(body));

    return 0;
}

int __handler_not_found(Request *req, Response *res, void *opaque)
{
    char body[1024];

    res->set_header(res, "Content-Type", "text/html");
    res->set_status_code(res, 404);

    sprintf(body, "not found, 404 !!\r\n");

    res->set_body(res, body, strlen(body));

    return 0;
}

int __handler_get_directory_list(Request *req, Response *res, void *opaque)
{
#define MAX_FILE_PATH_LEN 1024
#define MAX_FILE_NAME_LEN 100
#define MAX_TIME_LEN 50
    allocator_t *allocator = req->obj.allocator;
    int i, count, ret = 0;
    char (*list)[MAX_FILE_NAME_LEN];
    char path[MAX_FILE_PATH_LEN];
    Http_Server *server = (Http_Server *)opaque;
    char time[MAX_TIME_LEN];
    int size;

    snprintf(path, MAX_FILE_PATH_LEN, "%s%s", 
             server->root->get_cstr(server->root),
             (char *)req->uri);

    dbg_str(DBG_VIP, "handler_directory_request, dir path:%s", path);

    count = fs_count_list(path);
    if (count < 0) return -1;

    list = allocator_mem_alloc(allocator, count * MAX_FILE_NAME_LEN);
    ret = fs_list_fixed(path, (char **)list, count, MAX_FILE_NAME_LEN);
    if (ret < 0) {
        goto end;
    }

    res->set_fmt_body(res, 1024, "<html>\r\n");
    res->set_fmt_body(res, 1024, "<head><title>Index of %s</title></head>\r\n", req->uri);
    res->set_fmt_body(res, 1024, "<body bgcolor=\"white\">\r\n");
    res->set_fmt_body(res, 1024, "<h1>%s</h1>\r\n", req->uri);
    res->set_fmt_body(res, 1024, "<hr>\r\n");
    res->set_fmt_body(res, 1024, "<table width=\"800\"> \r\n");

    for (i = 0; i < count; i++) {
        if (strcmp(list[i], ".") == 0 || strcmp(list[i], "..") == 0) {
            fs_get_mtime(list[i], time, MAX_TIME_LEN);
            size = fs_get_size(list[i]);
            res->set_fmt_body(res, 1024, "<tr> <td  width=\"40%\"><a href=\"%s/\">%s/</a></td> <td width=\"30%\">%s</td> <td width=\"30%\">%d</td></tr>", list[i], list[i], time, size);
            continue;
        }
        
        ret = strlen(req->uri);
        if (((char *)req->uri)[ret - 1] != '/') {
            snprintf(path, MAX_FILE_PATH_LEN, "%s%s/%s", server->root->get_cstr(server->root),
                    (char *)req->uri, list[i]);
        } else {
            snprintf(path, MAX_FILE_PATH_LEN, "%s%s%s", server->root->get_cstr(server->root),
                    (char *)req->uri, list[i]);
        }

        dbg_str(DBG_DETAIL, "path:%s", path);
        ret = fs_is_directory(path);
        fs_get_mtime(path, time, MAX_TIME_LEN);
        size = fs_get_size(path);
        if (ret == 1) {
            res->set_fmt_body(res, 1024, "<tr> <td  width=\"40%\"><a href=\"%s/\">%s/</a></td> <td width=\"30%\">%s</td> <td width=\"30%\">%d</td></tr>", list[i], list[i], time, size);
        } else {
            res->set_fmt_body(res, 1024, "<tr> <td  width=\"40%\"><a href=\"%s\">%s</a></td> <td width=\"30%\">%s</td> <td width=\"30%\">%d</td></tr>", list[i], list[i], time, size);
        }
    }
    res->set_fmt_body(res, 1024, "</table>\r\n");
    res->set_fmt_body(res, 1024, "<hr>\r\n");
    res->set_fmt_body(res, 1024, "</body>\r\n");
    res->set_fmt_body(res, 1024, "</html>\r\n");

end:
    allocator_mem_free(allocator, list);

    return ret;
#undef MAX_FILE_PATH_LEN
#undef MAX_FILE_NAME_LEN
#undef MAX_TIME_LEN
}

int http_server_register_builtin_handlers(Httpd_Command *command)
{
    Http_Server *server = command->server;

    server->register_handler(server, "GET", "/api/hello_world", __handler_hello_world, command);
    server->register_handler(server, "POST", "/api/upload", __handler_upload, command);

    return 1;
}

int http_server_deregister_buitin_handlers(Httpd_Command *command)
{
    Http_Server *server = command->server;

    server->deregister_handler(server, "GET", "/api/hello_world");
    server->deregister_handler(server, "POST", "/api/upload");

    return 1;
}