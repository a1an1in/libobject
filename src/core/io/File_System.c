/**
 * @file File_System.c
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
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File_System.h>

static int __get_mtime(File_System *fs, char *path, char *time, int time_max_len)
{
    struct stat st;
    struct tm t;

    if (fs == NULL) return -1;
    if (stat(path, &st) == -1) return -1;
    
    strftime(time, time_max_len, "%Y-%m-%d %H:%M:%S", localtime_r(&st.st_mtime, &t));

    return 0;
}

static int __get_atime(File_System *fs, char *path, char *time, int time_max_len)
{
    struct stat st;
    struct tm t;

    if (fs == NULL) return -1;
    if (stat(path, &st) == -1) return -1;
    
    strftime(time, time_max_len, "%Y-%m-%d %H:%M:%S", localtime_r(&st.st_atime, &t));

    return 0;
}

static int __get_ctime(File_System *fs, char *path, char *time, int time_max_len)
{
    struct stat st;
    struct tm t;

    if (fs == NULL) return -1;
    if (stat(path, &st) == -1) return -1;
    
    strftime(time, time_max_len, "%Y-%m-%d %H:%M:%S", localtime_r(&st.st_ctime, &t));

    return 0;
}

static int __get_size(File_System *fs, char *path)
{
    struct stat st;

    if (fs == NULL) return -1;

    if (stat(path, &st) == -1) {
        return -1;
    }

    return st.st_size;
}

static int __get_stat(File_System *fs, char *path, struct stat *st)
{
    if (fs == NULL) return -1;
    dbg_str(DBG_VIP, "get_stat");

    if (stat(path, st) == -1) {
        return -1;
    }

    return 1;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Vfunc_Entry(1 , File_System, list_fixed, NULL),
    Init_Vfunc_Entry(2 , File_System, count_list, NULL),
    Init_Vfunc_Entry(3 , File_System, list, NULL),
    Init_Vfunc_Entry(4 , File_System, is_directory, NULL),
    Init_Vfunc_Entry(5 , File_System, get_size, __get_size),
    Init_Vfunc_Entry(6 , File_System, get_mtime, __get_mtime),
    Init_Vfunc_Entry(7 , File_System, get_atime, __get_atime),
    Init_Vfunc_Entry(8 , File_System, get_ctime, __get_ctime),
    Init_Vfunc_Entry(9 , File_System, get_stat, __get_stat),
    Init_Vfunc_Entry(10, File_System, mkdir, NULL),
    Init_Vfunc_Entry(11, File_System, rmdir, NULL),
    Init_End___Entry(12, File_System),
};
REGISTER_CLASS("File_System", file_system_class_info);