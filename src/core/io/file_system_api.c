#include <stdio.h>
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/String.h>
#include <libobject/core/io/file_system_api.h>

File_System *globle_file_system;

int fs_init()
{
    allocator_t *allocator = allocator_get_default_instance();

#   if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) ||  \
        defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || \
        defined(MAC_USER_MODE))
    globle_file_system = object_new(allocator, "Unix_File_System", NULL);
#   elif defined(WINDOWS_USER_MODE)
    globle_file_system = object_new(allocator, "Windows_File_System", NULL);
#   endif

    return 0;
}

int fs_destroy()
{
    return object_destroy(globle_file_system);
}

int fs_is_directory(char *name)
{
    return TRY_EXEC(globle_file_system->is_directory(globle_file_system, name));
}

/*
 * fs_list_fixed
 * to list the directory with max file name len
 * 
 * name: dir name
 * list: output buffer
 * count: list count
 * name_max_len: max file name len for the content.
 */
int fs_list_fixed(char *name, char **list, int count, int name_max_len)
{
    return TRY_EXEC(globle_file_system->list_fixed(globle_file_system, name, list, count, name_max_len));
}

static int __free_file_info_callback(allocator_t *allocator, fs_file_info_t *info)
{
    allocator_mem_free(allocator, info->file_name);
    allocator_mem_free(allocator, info);

    return 1;
}

int fs_list(char *name, Vector *vector)
{
    int ret;

    TRY {
        EXEC(vector->set_trustee(vector, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback));
        EXEC(ret = globle_file_system->list(globle_file_system, name, vector));
        THROW(ret);
    } CATCH (ret) {}

    return ret;
}

int fs_count_list(char *name)
{
    return TRY_EXEC(globle_file_system->count_list(globle_file_system, name));
}

int fs_get_mtime(char *path, char *time, int time_max_len)
{
    return TRY_EXEC(globle_file_system->get_mtime(globle_file_system, path, time, time_max_len));
}

int fs_get_atime(char *path, char *time, int time_max_len)
{
    return TRY_EXEC(globle_file_system->get_atime(globle_file_system, path, time, time_max_len));
}

int fs_get_ctime(char *path, char *time, int time_max_len)
{
    return TRY_EXEC(globle_file_system->get_ctime(globle_file_system, path, time, time_max_len));
}

int fs_get_size(char *path)
{
    return TRY_EXEC(globle_file_system->get_size(globle_file_system, path));
}

int fs_get_stat(char *path, struct stat *stat)
{
    return TRY_EXEC(globle_file_system->get_stat(globle_file_system, path, stat));
}

int fs_mkdir(char *path, mode_t mode)
{
    return TRY_EXEC(globle_file_system->mkdir(globle_file_system, path, mode));
}

int fs_rmdir(char *path)
{
    return TRY_EXEC(globle_file_system->rmdir(globle_file_system, path));
}

int fs_is_exist(char *path)
{
    if (access(path, 0) < 0) {
        return 0;
    } else {
        return 1;
    }
}

int fs_get_path_and_name(char *path, char **parent_dir, char **name)
{
    char *index;
    int len;

    if (parent_dir) *parent_dir = path;
    len = strlen(path);
    if (path[len] == '\\' || path[len] == '/') {
        path[len] = '\0';
    }

    if ((index = strrchr(path, '/')) != NULL) {
        *index = '\0';
        *name = index + 1;
        return 1;
    }

    if ((index = strrchr(path, '\\')) != NULL) {
        *index = '\0';
        *name = index + 1;
        return 1;
    } 

    return 0;
}

int fs_mkfile(char *path, mode_t mode)
{
    String *string;
    allocator_t *allocator = globle_file_system->obj.allocator;
    char *parent_dir, *name;
    FILE *f = NULL;
    int ret;

    TRY {
        SET_CATCH_STR_PARS(path, 0);
        string = object_new(allocator, "String", NULL);
        string->assign(string, path);
        EXEC(fs_get_path_and_name(string->get_cstr(string), &parent_dir, &name));
        dbg_str(DBG_VIP, "fs_mkfile dir:%s, name:%s", parent_dir, name);
        EXEC(fs_mkdir(parent_dir, mode));
        f = fopen(path, "w+");
        THROW_IF(f == NULL, -1);
    } CATCH (ret) {
        CATCH_SHOW_STR_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(string);
        if (f) fclose(f);
    }

    return ret;
}

int fs_rmfile(char *path)
{
    return remove(path);
}