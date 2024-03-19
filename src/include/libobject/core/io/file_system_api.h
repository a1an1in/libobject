#ifndef __FILESYSTEM_COMPAT_H__
#define __FILESYSTEM_COMPAT_H__

#include <libobject/core/io/File_System.h>
#include <libobject/core/Vector.h>

extern int fs_init();
extern int fs_destroy();
extern int fs_is_directory(char *name);
extern int fs_list_fixed(char *name, char **list, int count, int max_name_len);
extern int fs_count_list(char *name);
extern int fs_list(char *path, Vector *vector);
extern int fs_get_size(char *path);
extern int fs_get_mtime(char *path, char *time, int time_max_len);
extern int fs_get_atime(char *path, char *time, int time_max_len);
extern int fs_get_ctime(char *path, char *time, int time_max_len);
extern int fs_get_stat(char *path, struct stat *stat);
extern int fs_is_exist(char *path);
extern int fs_get_path_and_name(char *path, char **parent_dir, char **name);
extern int fs_get_relative_path(char *path, char *root, char **relative_path);
extern int fs_mkfile(char *path, mode_t mode);
extern int fs_rmfile(char *path);
extern int fs_mkdir(char *path, mode_t mode);
extern int fs_rmdir(char *path);
extern int fs_tree(char *path, Vector *vector, int depth);
extern int fs_file_info_struct_custom_to_json(cjson_t *root, void *element);
extern int fs_file_info_struct_custom_free(allocator_t *allocator, fs_file_info_t *info);
extern int fs_file_info_struct_custom_new(allocator_t *allocator, cjson_t *c, void **value);
extern int fs_file_info_struct_custom_print(int index, fs_file_info_t *info);

#endif
