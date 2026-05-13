#include <math.h>
#include <string.h>
#include <libobject/core/io/File.h>
#include <libobject/core/io/file_system_api.h>
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
    } else if (strstr(STR2A(str), "video") != NULL) {
        return E_FORM_DATA_TYPE_VIDEO;
    } else {
        return E_FORM_DATA_TYPE_UNKNOW;
    }
}

/*
 * 从 Content-Type header 中提取 multipart boundary 字符串
 * Content-Type 格式: multipart/form-data; boundary=----WebKitFormBoundary
 * 返回 boundary 值（不含前面的 --），如 "----WebKitFormBoundary"
 * 如果提取失败，返回 NULL
 */
static char *__get_multipart_boundary(Map *headers)
{
    char *content_type = NULL;
    char *boundary_start;

    headers->search(headers, "content-type", (void **)&content_type);
    if (content_type == NULL) return NULL;

    boundary_start = strstr(content_type, "boundary=");
    if (boundary_start == NULL) return NULL;

    boundary_start += 9; // 跳过 "boundary="
    return boundary_start;
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

/*
 * 构造完整的 boundary 分隔符用于搜索文件结束位置
 * 在 multipart 中，每个 part 结束的标记是 \r\n--{boundary}
 * 最后一个 part 结束的标记是 \r\n--{boundary}--
 * 这里构造 \r\n--{boundary} 用于定位 part 边界
 * boundary 参数来自 Content-Type header 中的 boundary= 值
 * 返回在 needle_buf 中，返回拼接后的长度
 */
static int __build_boundary_needle(char *boundary, char *needle_buf, int needle_len)
{
    int blen = strlen(boundary);
    int total = 4 + blen; // "\r\n--" + boundary

    if (total >= needle_len) return -1;

    memcpy(needle_buf, "\r\n--", 4);
    memcpy(needle_buf + 4, boundary, blen);
    needle_buf[total] = '\0';

    return total;
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
    char boundary[256] = {0};
    char needle[512] = {0};
    int needle_len, len, start = 0, ret = 1;
    String *str;
    File *file;
    Form_Data_Type form_data_type;
    char *b;

    TRY {
        // 从 Content-Type 中提取 boundary
        b = __get_multipart_boundary(request->headers);
        THROW_IF(b == NULL, -1);
        snprintf(boundary, 256, "%s", b);
        needle_len = __build_boundary_needle(boundary, needle, 512);
        THROW_IF(needle_len < 0, -1);
        dbg_str(NET_SUC, "multipart boundary=%s, needle_len=%d", boundary, needle_len);

        file = chain->new(chain, "File", NULL);
        while (buffer->get_len(buffer) > 2) {
            len = buffer->get_needle_offset(buffer, "\r\n", 2);
            if (len < 0) break;

            str = chain->new(chain, "String", NULL);
            THROW_IF(str == NULL, -1);
            EXEC(buffer->read_to_string(buffer, str, len + 2));
            str_to_lower(STR2A(str));

            if (str->equal(str, "\r\n") == 1) {
                // 使用完整的 boundary 分隔符搜索，避免二进制数据中的 \r\n-- 误匹配
                len = buffer->get_needle_offset(buffer, needle, needle_len);
                snprintf(path + strlen(path), MAX_FILE_NAME_LEN, "/%s", filename);
                dbg_str(NET_SUC, "write file len:%d, path:%s", len, path);
                EXEC(file->open(file, path, "w+"));
                file->write(file, buffer->addr + buffer->r_offset, len);
                file->close(file);
                buffer->r_offset += (len + needle_len);
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

/*
 * 从 multipart/form-data 中读取文件，保存到指定路径
 * 与 __read_form_data 的区别：
 * 1. 不依赖 content-type 判断文件类型
 * 2. 直接保存到 upload_path 目录
 */
static int __read_form_data_to_path(Request *request, char *upload_path)
{
    Object_Chain *chain = request->chain;
    Buffer *buffer = request->body;
    char *regex = "filename=\"([a-z0-9A-Z_.,!&=-]+)\"";
    char filename[MAX_FILE_NAME_LEN] = {0};
    char path[MAX_FILE_NAME_LEN] = {0};
    char boundary[256] = {0};
    char needle[512] = {0};
    int needle_len, len, start = 0, ret = 1;
    String *str;
    File *file;
    char *b;

    TRY {
        // 从 Content-Type 中提取 boundary
        b = __get_multipart_boundary(request->headers);
        THROW_IF(b == NULL, -1);
        snprintf(boundary, 256, "%s", b);
        needle_len = __build_boundary_needle(boundary, needle, 512);
        THROW_IF(needle_len < 0, -1);
        dbg_str(NET_SUC, "multipart boundary=%s, needle_len=%d", boundary, needle_len);

        file = chain->new(chain, "File", NULL);
        THROW_IF(file == NULL, -1);

        if (!fs_is_exist(upload_path)) {
            EXEC(fs_mkdir(upload_path, 0777));
        }

        while (buffer->get_len(buffer) > 2) {
            len = buffer->get_needle_offset(buffer, "\r\n", 2);
            if (len < 0) break;

            str = chain->new(chain, "String", NULL);
            THROW_IF(str == NULL, -1);
            EXEC(buffer->read_to_string(buffer, str, len + 2));
            str_to_lower(STR2A(str));

            if (str->equal(str, "\r\n") == 1) {
                // 使用完整的 boundary 分隔符搜索，避免二进制数据中的 \r\n-- 误匹配
                len = buffer->get_needle_offset(buffer, needle, needle_len);
                snprintf(path, MAX_FILE_NAME_LEN, "%s/%s", upload_path, filename);
                dbg_str(NET_SUC, "write file len:%d, path:%s", len, path);
                EXEC(file->open(file, path, "w+"));
                file->write(file, buffer->addr + buffer->r_offset, len);
                file->close(file);
                buffer->r_offset += (len + needle_len);
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

/*
 * 上传文件到 URI 指定的路径
 * URI 格式: POST /api/upload/{user}/{resource}/
 * 例如: POST /api/upload/alan/res/  -> 文件保存到 {server->root}/alan/res/{filename}
 */
static int __handler_upload_to_path(Request *req, Response *res, void *opaque)
{
    Object_Chain *chain = res->chain;
    Http_Server *server = (Http_Server *)req->server;
    int ret = 1, start = 0, len = 0;
    char regex[1024] = {0};
    char body[1024] = {0};
    char upload_path[MAX_FILE_NAME_LEN] = {0};
    char relative_path[MAX_FILE_NAME_LEN] = {0};
    String *str;

    TRY {
        THROW_IF(__is_body_multipart_form_data(req) == 0, -1);

        // 从 URI 中提取路径参数：/api/upload/{user}/{resource}/
        // 例如 /api/upload/alan/res/  -> 提取 alan/res
        str = chain->new(chain, "String", NULL);
        str->assign(str, (char *)req->uri);
        str->replace(str, "/api/upload/", "", -1);
        // 去掉末尾的 /
        if (STR2A(str)[str->get_len(str) - 1] == '/') {
            str->value[str->get_len(str) - 1] = '\0';
        }
        snprintf(relative_path, MAX_FILE_NAME_LEN, "%s", STR2A(str));

        // 设置保存路径为 server->root/{relative_path}/
        snprintf(upload_path, MAX_FILE_NAME_LEN, "%s/%s",
                 STR2A(server->root), relative_path);

        dbg_str(DBG_VIP, "upload to path:%s", upload_path);

        EXEC(__read_form_data_to_path(req, upload_path));

        // 从完整路径中提取文件名（最后一个 / 之后的部分）
        char *filename = strrchr(STR2A(req->file->name), '/');
        if (filename != NULL) {
            filename++;
        } else {
            filename = STR2A(req->file->name);
        }

        snprintf(body, 1024, "{\"retCode\":%d, \"url\":\"/%s/%s\"}",
                 1, relative_path, filename);
        res->set_body(res, body, strlen(body));
        res->set_status_code(res, 200);
        dbg_str(DBG_VIP, "upload success, path:%s/%s", relative_path, filename);
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

    server->register_handler(server, "GET", "/api/hello_world", NULL, __handler_hello_world, NULL, command);
    server->register_handler(server, "POST", "/api/upload", NULL, __handler_upload, NULL, command);
    server->register_handler(server, "POST", "/api/upload/*", NULL, __handler_upload_to_path, NULL, command);

    return 1;
}

int http_server_deregister_buitin_handlers(Httpd_Command *command)
{
    Http_Server *server = command->server;

    server->deregister_handler(server, "GET", "/api/hello_world");
    server->deregister_handler(server, "POST", "/api/upload");
    server->deregister_handler(server, "POST", "/api/upload/*");

    return 1;
}