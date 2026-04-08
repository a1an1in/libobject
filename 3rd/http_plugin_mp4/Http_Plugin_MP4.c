/**
 * @file Http_Plugin_MP4.c
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
#include "Http_Plugin_MP4.h"

// 解析Range头
static int parse_range_header(const char *range_header, long long file_size,
                             long long *start, long long *end)
{
    const char *bytes_prefix = "bytes=";
    char *dash;
    char *endptr;
    long long start_val, end_val;
    
    if (!range_header || strncmp(range_header, bytes_prefix, 6) != 0) {
        return -1;
    }
    
    const char *range_str = range_header + 6;
    
    // 查找破折号
    dash = strchr(range_str, '-');
    if (!dash) {
        return -1;
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
        if (endptr != dash) return -1;
        end_val = file_size - 1;
    } else {
        // 格式: bytes=0-499
        start_val = strtoll(range_str, &endptr, 10);
        if (endptr != dash) return -1;
        end_val = strtoll(dash + 1, &endptr, 10);
    }
    
    // 验证范围
    if (start_val < 0 || end_val >= file_size || start_val > end_val) {
        return -1;
    }
    
    *start = start_val;
    *end = end_val;
    return 0;
}

// MP4 Range handler主函数 - 简单版本（发送10%内容）
static int __handler_mp4_range(Request *req, Response *res, void *opaque)
{
    char filename[1024];
    struct stat st;
    FILE *file = NULL;
    char *range_header = NULL;
    long long file_size, range_start = 0, range_end = 0;
    int ret = 0;
    char content_range[128];
    Http_Plugin_MP4 *plugin = (Http_Plugin_MP4 *)opaque;
    Httpd_Command *httpd = (Httpd_Command *)(plugin->parent.opaque);
    Http_Server *server = httpd->server;
    
    TRY {
        dbg_str(DBG_VIP, "mp4_range_handler called for URI: %s", req->uri);
        // 构建完整文件路径
        snprintf(filename, sizeof(filename), "%s%s",
                server->root->get_cstr(server->root),
                (char *)req->uri);
        
        // 获取文件信息
        EXEC(stat(filename, &st));
        file_size = st.st_size;
        dbg_str(DBG_VIP, "File size: %lld bytes", file_size);
        
        // 获取Range头
        req->headers->search(req->headers, "range", (void **)&range_header);
        THROW_IF(range_header == NULL, -1);
        dbg_str(DBG_VIP, "Range header: %s", range_header);
        
        // 解析Range头
        EXEC(parse_range_header(range_header, file_size, &range_start, &range_end));
        dbg_str(DBG_VIP, "Parsed range: %lld-%lld", range_start, range_end);
        
        // 打开文件
        file = fopen(filename, "rb");
        THROW_IF(file == NULL, -1);
        
        // 对于Range请求，定位到起始位置
        if (fseek(file, range_start, SEEK_SET) != 0) {
            fclose(file);
            THROW(-1);
        }
        
        // 设置206 Partial Content响应
        res->set_status_code(res, 206);
        // 设置Content-Range头
        snprintf(content_range, sizeof(content_range),
                "bytes %lld-%lld/%lld",
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

        dbg_str(DBG_VIP, "MP4 range request prepared successfully, file will be sent by Response");
        
        // 调用server->response发送响应（包括头文件和文件内容）
        server->response(server, req, res);
        
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "mp4_range_handler error: %d", ret);
        // Range解析错误或文件错误
        res->set_status_code(res, 416); // Range Not Satisfiable
        
        if (file_size > 0) {
            char error_range[64];
            snprintf(error_range, sizeof(error_range), "bytes */%lld", file_size);
            res->set_header(res, "Content-Range", error_range);
        }
        
        // 错误情况下关闭文件
        if (file) {
            fclose(file);
            res->file = NULL;
        }
        
        // 发送错误响应
        server->response(server, req, res);
    }
    
    return ret;
}

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Http_Plugin_MP4 *plugin = (Http_Plugin_MP4 *)command;
    Httpd_Command *httpd = (Httpd_Command *)(command->opaque);
    Http_Server *server = httpd->server;

    dbg_str(DBG_VIP, "http mp4 plugin deregistered handlers from http server!");

    // 注销MP4 Range handler
    server->deregister_handler(server, "GET", "mp4_range_handler");
    
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
    int ret;

    TRY {
        dbg_str(DBG_VIP, "mp4 plugin run, help:%d!", ((Http_Plugin_MP4 *)command)->help);
        // 注册MP4 Range handler（使用特殊路径标记）
        server->register_handler(server, "GET", "mp4_range_handler", __handler_mp4_range, command);
        
        dbg_str(DBG_VIP, "http mp4 plugin registered handlers to http server!");
    } CATCH (ret) { }

    return 0;
}

static class_info_entry_t Http_Plugin_MP4_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Http_Plugin_MP4, construct, __construct),
    Init_Nfunc_Entry(2, Http_Plugin_MP4, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Http_Plugin_MP4, get_value, __get_value),
    Init_Vfunc_Entry(4, Http_Plugin_MP4, run_command, __run_command),
    Init_Str___Entry(5, Http_Plugin_MP4, option, NULL),
    Init_U32___Entry(6, Http_Plugin_MP4, help, 0),
    Init_End___Entry(7, Http_Plugin_MP4),
};
REGISTER_PLUGIN_CLASS(Http_Plugin_MP4, Http_Plugin_MP4_class_info);