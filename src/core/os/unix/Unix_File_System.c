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

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))

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

static int __list_fixed(Unix_File_System *fs, char *path, char **list, int count, int max_name_len)
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

static int __count_list(Unix_File_System *fs, char *path)
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

static int __list(Unix_File_System *fs, char *path, Vector *vector)
{
    dbg_str(DBG_VIP, "list %s", path);
    return 10;
}

static int __is_directory(Unix_File_System *fs, char *path)
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

static int __mkdir(Unix_File_System *fs, char *path, mode_t mode)
{
    String *string;
    allocator_t *allocator = fs->parent.obj.allocator;
    char *p, *tmp;
    int ret, cnt, i;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, path);  
        tmp = allocator_mem_alloc(allocator, strlen(path));

        cnt = string->split(string, "/", -1);

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            THROW_IF(p == NULL, -1);
            if (i == 0) {
                strcpy(tmp, p);
            } else{
                strcat(tmp, "/");
                strcat(tmp, p);
            }
            
            if (fs_is_exist(tmp)) {
                continue;
            }
            dbg_str(DBG_VIP, "mkdir %d:%s", i, tmp);
            EXEC(mkdir(tmp, mode));
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
        allocator_mem_free(allocator, tmp);
    }

    return ret;
}

/**
* 递归删除目录(删除该目录以及该目录包含的文件和目录)
*/
static int __rmdir(Unix_File_System *fs, char *dir)
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
        dbg_str(DBG_VIP, "rmdir %s", dir);
		rmdir(dir);		// 删除空目录
	} else {
		perror("unknow file type!");	
	}
	
	return 0;
}

static class_info_entry_t file_system_class_info[] = {
    Init_Obj___Entry(0, File_System, parent),
    Init_Vfunc_Entry(1, Unix_File_System, list_fixed, __list_fixed),
    Init_Vfunc_Entry(2, Unix_File_System, count_list, __count_list),
    Init_Vfunc_Entry(3, Unix_File_System, list, __list),
    Init_Vfunc_Entry(4, Unix_File_System, is_directory, __is_directory),
    Init_Vfunc_Entry(5, Unix_File_System, mkdir, __mkdir),
    Init_Vfunc_Entry(6, Unix_File_System, rmdir, __rmdir),
    Init_End___Entry(7, Unix_File_System),
};
REGISTER_CLASS("Unix_File_System", file_system_class_info);
#endif
