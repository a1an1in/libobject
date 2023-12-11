#ifndef __FILESYSTEM_COMPAT_H__
#define __FILESYSTEM_COMPAT_H__

#include <libobject/core/io/File_System.h>

extern int fs_init();
extern int fs_release();
extern int fs_is_directory(char *name);
extern int fs_list(char *name, char **list, int count, int max_name_len);
extern int fs_count_list(char *name);
extern int fs_get_size(char *path);
extern int fs_get_mtime(char *path, char *time, int time_max_len);
extern int fs_init();
extern int fs_destroy();
extern int fs_is_exist(char *path);
extern int fs_get_path_and_name(char *path, char **parent_dir, char **name);
extern int fs_mkfile(char *path, mode_t mode);
extern int fs_rmfile(char *path);
extern int fs_mkdir(char *path, mode_t mode);
extern int fs_rmdir(char *path);

#endif
