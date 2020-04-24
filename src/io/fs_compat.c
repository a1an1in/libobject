#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/io/fs_compat.h>

File_System *globle_file_system;

int fs_init()
{
    allocator_t *allocator = allocator_get_default_alloc();
#   if (defined(UNIX_USER_MODE)    ||\
        defined(LINUX_USER_MODE)   ||\
        defined(ANDROID_USER_MODE) ||\
        defined(IOS_USER_MODE)     ||\
        defined(MAC_USER_MODE))
    globle_file_system = object_new(allocator, "Unix_File_System", NULL);
#   elif defined(WINDOWS_USER_MODE)
    dbg_str(DBG_ERROR, "file system not support windows nows!");
    exit(1);
#   endif

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_FS, fs_init);

int fs_release()
{
    return object_destroy(globle_file_system);
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_FS, fs_release);

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

static int test_fs_list(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    char **list;
    int count = 0, i;

    count = fs_count_list(".");
    if (count < 0) {
        return 0;
    }
    list = allocator_mem_alloc(allocator, count * 20);
    count = fs_list(".", (char **)list, count, 20);
    if (count < 0) {
        return -1;
    }
    
    for (i = 0; i < count; i++) {
        printf("%s\n", (char *)list + i * 20);
    }

    allocator_mem_free(allocator, list);

    return 1;
}
REGISTER_TEST_FUNC(test_fs_list);

static int test_fs_get_mtime(TEST_ENTRY *entry)
{
    char time[1024];
    int ret;

    ret = fs_get_mtime(".", time, 1024 );
    if (ret < 0) {
        return 0;
    }
    
    dbg_str(DBG_ERROR, "time:%s", time);

    return 1;
}
REGISTER_TEST_FUNC(test_fs_get_mtime);

static int test_fs_get_size(TEST_ENTRY *entry)
{
    int ret;

    ret = fs_get_size("./README.md");
    if (ret < 0) {
        return 0;
    }
    
    dbg_str(DBG_ERROR, "README size:%d", ret);

    return 1;
}
REGISTER_TEST_FUNC(test_fs_get_size);
