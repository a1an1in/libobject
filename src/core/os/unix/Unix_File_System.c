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

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/String.h>
#include "Unix_File_System.h"

extern int fs_gethome(char *path, int max_len);

static int __mkdir(Unix_File_System *fs, char *path, mode_t mode)
{
    String *string;
    allocator_t *allocator = fs->parent.obj.allocator;
    char *p, *tmp;
    int path_len;
    int ret, cnt, i;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, path);
        path_len = strlen(path);
        tmp = allocator_mem_zalloc(allocator, path_len * 2);

        cnt = string->split(string, "/", -1);
        if (path[0] == '/') {  strcat(tmp, "/"); }

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            THROW_IF(p == NULL, -1);
            if (i == 0 && strncmp(p, ".", strlen(p)) == 0) {
                strcpy(tmp, p);
            } else if (i == 0 && strncmp(p, "~", strlen(p)) == 0) {
                fs_gethome(tmp, path_len);
            } else {
                strcat(tmp, "/");
                strcat(tmp, p);
            }
            
            if (fs_is_exist(tmp)) {
                continue;
            }
            dbg_str(DBG_INFO, "mkdir %d:%s", i, tmp);
            EXEC(mkdir(tmp, mode));
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
        allocator_mem_free(allocator, tmp);
    }

    return ret;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0, File_System, parent),
    Init_Vfunc_Entry(1, Unix_File_System, mkdir, __mkdir),
    Init_End___Entry(2, Unix_File_System),
};
REGISTER_CLASS(Unix_File_System, file_system_class_info);
#endif
