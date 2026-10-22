/**
 * @file Lock_Mutex.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-07
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#if (defined(WINDOWS_USER_MODE))

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "Windows_File_System.h"

/*
 * 递归创建目录。
 * windows 的路径分隔符有 '/' 和 '\\' 两种, 这里逐级截断路径来创建,
 * 盘符("C:")和 UNC 前缀("\\\\server\\share")不处理。
 */
static int __mkdir(Windows_File_System *fs, char *path, mode_t mode)
{
    allocator_t *allocator = fs->parent.obj.allocator;
    char *tmp;
    char home[512] = {0};
    int path_len, i, ret = 0;

    TRY {
        path_len = strlen(path);
        /* NOTE: must be zeroed, it is used with strcat()/strcpy() below */
        tmp = allocator_mem_zalloc(allocator, path_len + 1024);

        if (path[0] == '~') {
            /* "~" 展开成用户目录 */
            fs_gethome(home, sizeof(home));
            strcat(tmp, home);
            strcat(tmp, path + 1);
        } else {
            strcpy(tmp, path);
        }
        path_len = strlen(tmp);

        for (i = 1; i < path_len; i++) {
            if (tmp[i] != '/' && tmp[i] != '\\') {
                continue;
            }
            /* 盘符("C:\\xxx")的盘符后面那个分隔符跳过 */
            if (i == 2 && tmp[1] == ':') {
                continue;
            }
            /* UNC 前缀("\\\\server\\share")跳过 */
            if (i == 1 && tmp[1] == '\\') {
                continue;
            }
            /* 连续的分隔符跳过 */
            if (tmp[i - 1] == '/' || tmp[i - 1] == '\\') {
                continue;
            }

            tmp[i] = '\0';
            if (fs_is_exist(tmp) == 0) {
                dbg_str(DBG_VIP, "mkdir %s", tmp);
                EXEC(mkdir(tmp));
            }
            tmp[i] = '/';
        }

        if (fs_is_exist(tmp) == 0) {
            dbg_str(DBG_VIP, "mkdir %s", tmp);
            EXEC(mkdir(tmp));
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "mkdir %s", path);
    } FINALLY {
        allocator_mem_free(allocator, tmp);
    }

    return ret;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0 , File_System, parent),
    Init_Vfunc_Entry(1 , Windows_File_System, mkdir, __mkdir),
    Init_End___Entry(2 , Windows_File_System),
};
REGISTER_CLASS(Windows_File_System, file_system_class_info);
#endif
