#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system.h>

#if (!defined(WINDOWS_USER_MODE))
static int test_fs_list(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    char **list;
    char *test_path = "./res";
    int count = 0, i, ret;

    TRY {
        count = fs_count_list(test_path);
        THROW_IF(count < 0, -1);

        printf("count:%d\n", count);
        list = allocator_mem_alloc(allocator, count * 20);
        count = fs_list(test_path, (char **)list, count, 20);
        THROW_IF(count < 0, -1);
        
        for (i = 0; i < count; i++) {
            printf("%s\n", (char *)list + i * 20);
        }
        THROW_IF(count != 9, -1);
    } CATCH (ret) {} FINALLY {
        allocator_mem_free(allocator, list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_list);

/* test modified time */
static int test_fs_get_mtime(TEST_ENTRY *entry)
{
    char time[1024];
    char *expect = "2022-11-14 04:07:22";
    int ret;

    TRY {
        ret = fs_get_mtime("./res/TIMES.TTF", time, 1024 );
        THROW_IF(ret < 0, -1);
        THROW_IF(strcmp(time, expect) != 0, -1);
        dbg_str(DBG_SUC, "time:%s", time);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_get_mtime);

static int test_fs_get_size(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        ret = fs_get_size("./res/TIMES.TTF");
        THROW_IF(ret < 0, -1);
        THROW_IF(ret != 1110872, -1);

        dbg_str(DBG_SUC, "file size:%d", ret);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_get_size);
#endif