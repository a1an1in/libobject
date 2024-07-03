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
#include <string.h>
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File_System.h>
#include <libobject/core/Struct_Adapter.h>
#include <libobject/core/io/file_system_api.h>

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

    if (stat(path, st) == -1) {
        return -1;
    }

    return 1;
}

static int __list_fixed(File_System *fs, char *path, char **list, int count, int max_name_len)
{
    DIR *dir;
    struct dirent *ptr;
    allocator_t *allocator = fs->obj.allocator;
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

static int __list(File_System *fs, char *path, Vector *vector)
{
    DIR *dir;
    struct dirent *ptr;
    allocator_t *allocator = fs->obj.allocator;
    int i = 0, ret = 0, count = 0;
    fs_file_info_t *info;

    TRY {
        THROW_IF(fs == NULL, -1);
        THROW_IF((dir = opendir(path)) == NULL, -1);

        while ((ptr = readdir(dir)) != NULL) {
            count++;
            info = allocator_mem_alloc(allocator, sizeof(fs_file_info_t));
            info->file_name = allocator_mem_zalloc(allocator, strlen(ptr->d_name) + strlen(path) + 2);
            strcpy(info->file_name, path);
            strcat(info->file_name, ptr->d_name);
            vector->add(vector, info);
        }
        closedir(dir);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "fs list:%s error", path);
    }

    return count;
}

static int __is_directory(File_System *fs, char *path)
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

static int __count_list(File_System *fs, char *path)
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

/**
* 递归删除目录(删除该目录以及该目录包含的文件和目录)
*/
static int __rmdir(File_System *fs, char *dir)
{
    char cur_dir[] = ".";
	char up_dir[] = "..";
	char dir_name[1024];
	DIR *dirp;
	struct dirent *dp;
	struct stat dir_stat;
 
	// 参数传递进来的目录不存在，直接返回
	if ( 0 != access(dir, F_OK) ) {
		return 0;
	}
 
	// 获取目录属性失败，返回错误
	if (stat(dir, &dir_stat) < 0) {
		perror("get directory stat error");
		return -1;
	}
 
	if (S_ISREG(dir_stat.st_mode)) {	//普通文件直接删除
		remove(dir);
	} else if (S_ISDIR(dir_stat.st_mode)) {	//目录文件，递归删除目录中内容
		dirp = opendir(dir);
		while ((dp=readdir(dirp)) != NULL) {
			// 忽略 . 和 ..
			if ((0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name))) {
				continue;
			}
			
			snprintf(dir_name, sizeof(dir_name), "%s/%s", dir, dp->d_name);
			__rmdir(fs, dir_name);   // 递归调用
		}
		closedir(dirp);
        dbg_str(DBG_INFO, "rmdir %s", dir);
		rmdir(dir);		// 删除空目录
	} else {
		perror("unknow file type!");	
	}
	
	return 0;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Vfunc_Entry(1 , File_System, list_fixed, __list_fixed),
    Init_Vfunc_Entry(2 , File_System, count_list, __count_list),
    Init_Vfunc_Entry(3 , File_System, list, __list),
    Init_Vfunc_Entry(4 , File_System, is_directory, __is_directory),
    Init_Vfunc_Entry(5 , File_System, get_size, __get_size),
    Init_Vfunc_Entry(6 , File_System, get_mtime, __get_mtime),
    Init_Vfunc_Entry(7 , File_System, get_atime, __get_atime),
    Init_Vfunc_Entry(8 , File_System, get_ctime, __get_ctime),
    Init_Vfunc_Entry(9 , File_System, get_stat, __get_stat),
    Init_Vfunc_Entry(10, File_System, mkdir, NULL),
    Init_Vfunc_Entry(11, File_System, rmdir, __rmdir),
    Init_End___Entry(12, File_System),
};
REGISTER_CLASS(File_System, file_system_class_info);

typedef Struct_Adapter FS_File_Info_Struct_Adapter;
static class_info_entry_t fs_file_info_stuct_adapter[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, FS_File_Info_Struct_Adapter, new, fs_file_info_struct_custom_new),
    Init_Point_Entry(2, FS_File_Info_Struct_Adapter, free, fs_file_info_struct_custom_free),
    Init_Point_Entry(3, FS_File_Info_Struct_Adapter, to_json, fs_file_info_struct_custom_to_json),
    Init_End___Entry(4, FS_File_Info_Struct_Adapter),
};
REGISTER_CLASS(FS_File_Info_Struct_Adapter, fs_file_info_stuct_adapter)