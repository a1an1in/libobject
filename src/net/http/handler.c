/**
 * @file Handler.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/handler.h>
#include <libobject/net/http/Server.h>
#include <libobject/core/io/fs_compat.h>

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

    dbg_str(DBG_SUC, "handler_directory_request, dir path:%s", path);

    count = fs_count_list(path);
    if (count < 0) return -1;

    list = allocator_mem_alloc(allocator, count * MAX_FILE_NAME_LEN);
    ret = fs_list(path, (char **)list, count, MAX_FILE_NAME_LEN);
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


