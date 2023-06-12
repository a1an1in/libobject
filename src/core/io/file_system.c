#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system.h>

#if (!defined(WINDOWS_USER_MODE))
File_System *globle_file_system;

int fs_init()
{
    allocator_t *allocator = allocator_get_default_instance();

#   if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) ||  \
        defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || \
        defined(MAC_USER_MODE))
    globle_file_system = object_new(allocator, "Unix_File_System", NULL);
#   elif defined(WINDOWS_USER_MODE)
    dbg_str(DBG_ERROR, "file system not support windows nows!");
    exit(1);
#   endif

    return 0;
}

int fs_destroy()
{
    return object_destroy(globle_file_system);
}

int fs_is_directory(char *name)
{
    return globle_file_system->is_directory(globle_file_system, name);
}

int fs_list(char *name, char **list, int count, int max_name_len)
{
    return globle_file_system->list(globle_file_system, name, list, count, max_name_len);
}

int fs_count_list(char *name)
{
    return globle_file_system->count_list(globle_file_system, name);
}

int fs_get_mtime(char *path, char *time, int time_max_len)
{
    return globle_file_system->get_mtime(globle_file_system, path, time, time_max_len);
}

int fs_get_size(char *path)
{
    return globle_file_system->get_size(globle_file_system, path);
}

#endif
