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

static int __list_fixed(Windows_File_System *fs, char *path, char **list, int count, int max_name_len)
{
    DIR *dir;
    struct dirent *ptr;
    allocator_t *allocator = fs->parent.obj.allocator;
    int i = 0, ret = 0;

    if (fs == NULL) return -1;
    if ((dir = opendir(path)) == NULL) {
        dbg_str(DBG_ERROR, "opendir error:%s!", strerror(errno));
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL && i < count) {
        if (strlen(ptr->d_name) >= max_name_len) {
            dbg_str(DBG_ERROR, "file name is longer than the given buffer, file name:%s, max_name_len:%d, file_name_len:%d", 
                    ptr->d_name, max_name_len, strlen(ptr->d_name));
            return -1;
        }
        if (i >= count) {
            dbg_str(DBG_ERROR, "file count is more than the given buufer!");
            return -1;
        }
        strcpy(((char *)list + i * max_name_len), ptr->d_name);
        i++;
    }
    ret = i;
    closedir(dir);

    return ret;
}

static int __count_list(Windows_File_System *fs, char *path)
{
    DIR *dir;
    struct dirent *ptr;
    int i = 0, ret = 0;

    if (fs == NULL) return -1;
    if ((dir = opendir(path)) == NULL) {
        dbg_str(DBG_ERROR, "opendir error:%s!", strerror(errno));
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL) {
        i++;
    }
    ret = i;
    closedir(dir);

    return ret;
}

static int __list(Windows_File_System *fs, char *path, Vector *vector)
{

}

static int __is_directory(Windows_File_System *fs, char *path)
{
    struct stat st;

    if (fs == NULL) return -1;

    if (stat(path, &st) == -1) {
        return -1;
    }

    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        return 1;
    } else return 0;
}

static int __get_size(Windows_File_System *fs, char *path)
{
    struct stat st;

    if (fs == NULL) return -1;

    if (stat(path, &st) == -1) {
        return -1;
    }

    return st.st_size;
}

static int __get_mtime(Windows_File_System *fs, char *path, char *time, int time_max_len)
{
    struct stat st;
    struct tm t;

    if (fs == NULL) return -1;
    if (stat(path, &st) == -1) return -1;
    
    localtime_s(&t, &st.st_mtime);
    strftime(time, time_max_len, "%Y-%m-%d %H:%M:%S", &t);
    dbg_str(DBG_DETAIL, "get_mtime:%s", time);

    return 0;
}

static int __mkdir(Windows_File_System *fs, char *path, mode_t mode)
{
    return TRY_EXEC(mkdir(path, mode));
}

static int __rmdir(Unix_File_System *fs, char *path)
{
    return 0;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0 , File_System, parent),
    Init_Vfunc_Entry(1 , Windows_File_System, list_fixed, __list_fixed),
    Init_Vfunc_Entry(2 , Windows_File_System, count_list, __count_list),
    Init_Vfunc_Entry(3 , Windows_File_System, list, __list),
    Init_Vfunc_Entry(4 , Windows_File_System, is_directory, __is_directory),
    Init_Vfunc_Entry(5 , Windows_File_System, get_size, __get_size),
    Init_Vfunc_Entry(6 , Windows_File_System, get_mtime, __get_mtime),
    Init_Vfunc_Entry(7 , Windows_File_System, mkdir, __mkdir),
    Init_Vfunc_Entry(8 , Windows_File_System, rmdir, __rmdir),
    Init_End___Entry(9 , Windows_File_System),
};
REGISTER_CLASS("Windows_File_System", file_system_class_info);
#endif
