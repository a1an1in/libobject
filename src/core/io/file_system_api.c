#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/String.h>
#include <libobject/core/io/file_system_api.h>

File_System *globle_file_system;

static int __free_file_info_callback(allocator_t *allocator, fs_file_info_t *info)
{
    allocator_mem_free(allocator, info->file_name);
    allocator_mem_free(allocator, info);

    return 1;
}

static int __get_file_info_stat_callback(int index, fs_file_info_t *info)
{
    return fs_get_stat(info->file_name, &info->st);
}

static int __print_file_info_callback(int index, fs_file_info_t *info)
{
    dbg_str(DBG_INFO, "index:%d file name:%s atime:%lx, ctime:%lx, mtime:%lx size:%ld", 
            index, info->file_name, info->st.st_atime, info->st.st_ctime,
            info->st.st_mtime, info->st.st_size);

    return 1;
}

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
        if (name == NULL) return 1;
        *name = index + 1;
        return 1;
    }

    if ((index = strrchr(path, '\\')) != NULL) {
        *index = '\0';
        if (name == NULL) return 1;
        *name = index + 1;
        return 1;
    } 

    return 0;
}

int fs_get_relative_path(char *path, char *root, char **relative_path)
{
    int root_len;
    int ret;

    TRY {
        root_len = strlen(root);
        if (strncmp(path, root, root_len) != 0) THROW(-1);
        if (root[root_len -1] != '/') {
            THROW(-1);
        }
        *relative_path = path + root_len;
    } CATCH (ret) {}

    return ret;
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
        dbg_str(DBG_INFO, "fs_mkfile dir:%s, name:%s", parent_dir, name);
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

int fs_list(char *path, Vector *vector)
{
    int ret, len;
    char tmp[1024] = {0};

    TRY {
        strcpy(tmp, path);
        len = strlen(tmp);
        THROW_IF((fs_is_directory(path) == 0), 0);

        if (path[len - 1] != '/') {
            strcat(tmp, "/");
            dbg_str(DBG_INFO, "path:%s", tmp);
        }

        EXEC(vector->customize(vector, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback));
        EXEC(ret = globle_file_system->list(globle_file_system, tmp, vector));
        EXEC(vector->for_each(vector, __get_file_info_stat_callback));
        THROW(ret);
    } CATCH (ret) {}

    return ret;
}

int fs_print_file_info_list(Vector *vector)
{
    return vector->for_each(vector, __print_file_info_callback);
}

int fs_tree(char *path, Vector *vector, int depth)
{
    allocator_t *allocator = globle_file_system->obj.allocator;
    Vector *list = NULL;
    fs_file_info_t *info;
    int ret, count, i, len;

    TRY {
        THROW_IF(depth == 0, 0);
        if (depth > 0) depth--;
        EXEC(vector->customize(vector, VALUE_TYPE_STRUCT_POINTER, 
                               __free_file_info_callback));

        list = object_new(allocator, "Vector", NULL);
        EXEC(fs_list(path, list));
        count = list->count(list);
        THROW_IF(count == 2, 0);
        
        for (i = 0; i < count; i++) {
            EXEC(list->peek_at(list, i, &info));
            if (!S_ISDIR(info->st.st_mode)) continue;
            len = strlen(info->file_name);
            if (info->file_name[len - 1] == '.') continue;
            dbg_str(DBG_INFO, "filename:%s", info->file_name);
            EXEC(fs_tree(info->file_name, vector, depth));
        }
        EXEC(vector->add_vector(vector, list));
        THROW(vector->count(vector));
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}