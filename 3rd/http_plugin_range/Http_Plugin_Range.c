/**
 * @file Http_Plugin_Range.c
 * @Synopsis  HTTP MP4插件，支持Range请求用于视频播放
 * @author alan lin
 * @version 
 * @date 2026-04-07
 */
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <libobject/argument/Application.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Server.h>
#include <libobject/net/http/Httpd_Command.h>
#include <libobject/net/http/Request.h>
#include <libobject/net/http/Response.h>
#include "Http_Plugin_Range.h"

// 解析Range头
static int parse_range_header(const char *range_header, long long file_size,
                             long long *start, long long *end)
{
    const char *bytes_prefix = "bytes=";
    char *dash;
    char *endptr;
    long long start_val, end_val;
    
    if (!range_header || strncmp(range_header, bytes_prefix, 6) != 0) {
        return -1; // 格式错误
    }
    
    const char *range_str = range_header + 6;
    
    // 查找破折号
    dash = strchr(range_str, '-');
    if (!dash) {
        return -1; // 格式错误
    }
    
    // 解析起始位置
    if (dash == range_str) {
        // 格式: bytes=-500 (最后500字节)
        start_val = file_size - strtoll(dash + 1, &endptr, 10);
        if (start_val < 0) start_val = 0;
        end_val = file_size - 1;
    } else if (*(dash + 1) == '\0') {
        // 格式: bytes=500- (从500到末尾)
        start_val = strtoll(range_str, &endptr, 10);
        if (endptr != dash) return -1; // 格式错误
        end_val = file_size - 1;
    } else {
        // 格式: bytes=0-499
        start_val = strtoll(range_str, &endptr, 10);
        if (endptr != dash) return -1; // 格式错误
        end_val = strtoll(dash + 1, &endptr, 10);
    }
    
    // 验证范围
    if (start_val < 0 || end_val >= file_size || start_val > end_val) {
        return -2; // Range无法满足
    }
    
    *start = start_val;
    *end = end_val;
    return 0;
}

static int __handler_range(Request *req, Response *res, void *opaque)
{
    char filename[1024];
    struct stat st;
    FILE *file = NULL;
    char *range_header = NULL;
    long long file_size, range_start = 0, range_end = 0;
    char content_range[128];
    Http_Plugin_Range *plugin = (Http_Plugin_Range *)opaque;
    Httpd_Command *httpd = (Httpd_Command *)(plugin->parent.opaque);
    Http_Server *server = httpd->server;
    int ret = 0;
    
    TRY {
        // 构建完整文件路径
        snprintf(filename, sizeof(filename), "%s%s",
                 server->root->get_cstr(server->root),
                 (char *)req->uri);
        
        // 获取文件信息
        EXEC(stat(filename, &st));
        file_size = st.st_size;
        
        // 获取Range头
        req->headers->search(req->headers, "range", (void **)&range_header);
        THROW_IF(range_header == NULL, -1);
        ret = parse_range_header(range_header, file_size, &range_start, &range_end); // 解析Range头
        // 检查Range解析结果
        if (ret == -2) {
            // Range无法满足 - 正常返回416响应
            dbg_str(DBG_VIP, "socket fd:%d, range not satisfiable: %s", req->socket->fd, range_header);
            res->set_status_code(res, 416); // Range Not Satisfiable
            
            snprintf(content_range, sizeof(content_range), "bytes */%lld", file_size);
            res->set_header(res, "Content-Range", content_range);
            res->set_header(res, "Content-Type", "text/plain; charset=utf-8");
            THROW(0); // 正常处理完成, 不用关闭文件， 后续异步传输使用。 
        }
        THROW_IF(ret != 0, -1);
        dbg_str(DBG_VIP, "socket fd:%d, url:%s, file size:%lld bytes, range header:%s, range:%lld-%lld", 
                req->socket->fd, req->uri, file_size, range_header, range_start, range_end);
        
        // 打开文件
        file = fopen(filename, "rb");
        THROW_IF(file == NULL, -1);
        if (fseek(file, range_start, SEEK_SET) != 0) { // 对于Range请求，定位到起始位置
            THROW(-1);
        }
        
        // 设置206 Partial Content响应
        res->set_status_code(res, 206);
        // 设置Content-Range头
        snprintf(content_range, sizeof(content_range), "bytes %lld-%lld/%lld",
                 range_start, range_end, file_size);
        res->set_header(res, "Content-Range", content_range);
        // 设置Content-Type
        res->set_header(res, "Content-Type", "video/mp4");
        // 设置Accept-Ranges（告诉客户端支持范围请求）
        res->set_header(res, "Accept-Ranges", "bytes");
        // 设置Content-Length（整个范围的长度）
        long long total_range_size = range_end - range_start + 1;
        res->content_len = total_range_size;
        // 将文件指针保存到Response中，让Response负责发送和关闭文件
        res->file = file;
        dbg_str(DBG_VIP, "socket fd:%d, range request prepared successfully, file will be sent by async", req->socket->fd);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "range_handler error: %d", ret);
        // 其他错误情况（文件错误、格式错误等）
        res->set_status_code(res, 400); // Bad Request
        // 错误情况下关闭文件
        if (file) {
            fclose(file);
            res->file = NULL;
        }
    } FINALLY {
        server->response(server, req, res); // 调用server->response发送响应（包括头文件和文件内容）
    }
    
    return ret;
}

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Http_Plugin_Range *plugin = (Http_Plugin_Range *)command;
    Httpd_Command *httpd = (Httpd_Command *)(command->opaque);
    Http_Server *server = httpd->server;

    dbg_str(DBG_VIP, "http mp4 plugin deregistered handlers from http server!");

    // 注销MP4 Range handler
    server->deregister_handler(server, "GET", "range_handler");
    
    object_destroy(plugin->option);

    return 0;
}

static void *__get_value(Command *command,char *command_name, char *flag_name)
{
    return NULL;
}

static int __run_command(Command *command)
{
    allocator_t *allocator = command->parent.allocator;
    Httpd_Command *httpd = (Httpd_Command *)(command->opaque);
    Http_Server *server = httpd->server;
    int ret = 0;

    TRY {
        // 注册MP4 Range handler（使用特殊路径标记）
        server->register_handler(server, "GET", "range_handler", NULL, __handler_range, NULL, command);
        
        dbg_str(DBG_VIP, "http mp4 plugin registered handlers to http server!");
    } CATCH (ret) { }

    return ret;
}

static class_info_entry_t Http_Plugin_MP4_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Http_Plugin_Range, construct, __construct),
    Init_Nfunc_Entry(2, Http_Plugin_Range, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Http_Plugin_Range, get_value, __get_value),
    Init_Vfunc_Entry(4, Http_Plugin_Range, run_command, __run_command),
    Init_Str___Entry(5, Http_Plugin_Range, option, NULL),
    Init_U32___Entry(6, Http_Plugin_Range, help, 0),
    Init_End___Entry(7, Http_Plugin_Range),
};
REGISTER_PLUGIN_CLASS(Http_Plugin_Range, Http_Plugin_MP4_class_info);