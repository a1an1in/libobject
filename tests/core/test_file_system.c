#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/io/file_system_api.h>

static int test_fs_list_fixed(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    char **list;
    char *test_path = "./res";
    int count = 0, i, ret;

    TRY {
        count = fs_count_list(test_path);
        THROW_IF(count <= 0, -1);
        
        list = allocator_mem_alloc(allocator, count * 20);
        count = fs_list_fixed(test_path, (char **)list, count, 20);
        THROW_IF(count < 0, -1);
        
        for (i = 0; i < count; i++) {
            dbg_str(DBG_DETAIL,"%d: %s", i, (char *)list + i * 20);
        }

        SET_CATCH_INT_PARS(count, 0);
        THROW_IF(count != 10, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        allocator_mem_free(allocator, list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_list_fixed);

static int test_fs_list_dynamic(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    char *test_path = "./res/";
    int count = 0, i, ret;

    TRY {
        list = object_new(allocator, "Vector", NULL);
        
        count = fs_list(test_path, list);
        THROW_IF(count < 0, -1);
        fs_print_file_info_list(list);

        SET_CATCH_INT_PARS(count, 0);
        THROW_IF(count != 10, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_list_dynamic);

static int test_fs_tree(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    char *test_path = "./mk/";
    int count = 0, i, ret;

    TRY {
        list = object_new(allocator, "Vector", NULL);
        
        count = fs_tree(test_path, list, -1);
        THROW_IF(count < 0, -1);
        fs_print_file_info_list(list);

        SET_CATCH_INT_PARS(count, 0);
        THROW_IF(count != 10, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_tree);

static int test_fs_tree_with_depth(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    char *test_path = "./mk/";
    int count = 0, i, ret;

    TRY {
        list = object_new(allocator, "Vector", NULL);
        
        count = fs_tree(test_path, list, 1);
        THROW_IF(count < 0, -1);
        fs_print_file_info_list(list);

        SET_CATCH_INT_PARS(count, 0);
        THROW_IF(count != 10, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(list);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_tree_with_depth);

/* test modified time */
static int test_fs_get_mtime(TEST_ENTRY *entry)
{
    char time[1024];
    int ret;

    TRY {
        ret = fs_get_mtime("./res/TIMES.TTF", time, 1024 );
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "fs_get_mtime, time:%s", time);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_CMD(test_fs_get_mtime);

/* test change time */
static int test_fs_get_ctime(TEST_ENTRY *entry)
{
    char time[1024];
    int ret;

    TRY {
        ret = fs_get_ctime("./res/TIMES.TTF", time, 1024 );
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "fs_get_ctime, time:%s", time);
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_CMD(test_fs_get_ctime);

static int test_fs_get_stat(TEST_ENTRY *entry)
{
    char time[1024];
    struct stat stat;
    int ret;

    TRY {
        EXEC(fs_get_stat("./res/TIMES.TTF", &stat));
        THROW_IF(ret < 0, -1);
        dbg_str(DBG_INFO, "fs_get_stat, modify_time:%d", stat.st_mtime);
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_CMD(test_fs_get_stat);

static int test_fs_get_size(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        ret = fs_get_size("./res/TIMES.TTF");
        THROW_IF(ret < 0, -1);
        THROW_IF(ret != 1110872, -1);

        dbg_str(DBG_INFO, "file size:%d", ret);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_get_size);

static int test_fs_is_directory(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        ret = fs_is_directory("./res/TIMES.TTF");
        THROW_IF(ret == 1 || ret < 0, -1);
        ret = fs_is_directory("./res");
        THROW_IF(ret != 1, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_is_directory);

static int test_fs_is_exist(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        THROW_IF(fs_is_exist("./tests/core/res/") != 1, -1);
        THROW_IF(fs_is_exist("./tests/core/res/xxx") == 1, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_is_exist);

static int test_fs_get_path_and_name(TEST_ENTRY *entry)
{
    char name[1024] = "./tests/core/res/aa/bb/cc";
    char *dir, *current_name;
    int ret;

    TRY {
        EXEC(fs_get_path_and_name(name, &dir, &current_name));
        dbg_str(DBG_INFO, "fs_get_path_and_name %s :%s ", dir, current_name);
        THROW_IF(strcmp(dir, "./tests/core/res/aa/bb") != 0, -1);
        THROW_IF(strcmp(current_name, "cc") != 0, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_get_path_and_name);

static int test_fs_get_relative_path(TEST_ENTRY *entry)
{
    char name[1024] = "./tests/core/res/aa/bb/cc";
    char *root = "./tests/core/res/";
    char *relative_path;
    int ret;

    TRY {
        EXEC(fs_get_relative_path(name, root, &relative_path));
        dbg_str(DBG_INFO, "fs_get_relative_path %s :%s", name, relative_path);
        THROW_IF(strcmp(relative_path, "aa/bb/cc") != 0, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_get_relative_path);

static int test_fs_mk_and_rm_dir(TEST_ENTRY *entry)
{
    // char *make_dir_name = "./tests/core/res/aa/bb/cc";
    char *make_dir_name = "./tests/core/res/aa";
    char *del_dir_name = "./tests/core/res/aa";
    int ret;

    TRY {
        EXEC(fs_mkdir(make_dir_name, 0777));
        THROW_IF(fs_is_exist(make_dir_name) != 1, -1);
        EXEC(fs_rmdir(del_dir_name));
        THROW_IF(fs_is_exist(del_dir_name) == 1, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_mk_and_rm_dir);

static int test_fs_mk_and_rm_file(TEST_ENTRY *entry)
{
    char name[1024] = "./tests/core/res/aa/bb/cc/abc.txt";
    int ret;

    TRY {
        EXEC(fs_mkfile(name, 0777));
        THROW_IF(fs_is_exist(name) != 1, -1);
        EXEC(fs_rmfile(name));
        THROW_IF(fs_is_exist(name) == 1, -1);
        EXEC(fs_rmdir("./tests/core/res/aa"));
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_fs_mk_and_rm_file);