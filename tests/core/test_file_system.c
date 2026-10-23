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

static int __test_fs_tree_count_dir(char *path, int *count)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    int ret = 0;

    TRY {
        list = object_new(allocator, "Vector", NULL);
        THROW_IF(list == NULL, -1);
        EXEC(ret = fs_list(path, list));
        THROW_IF(ret < 0, -1);
        *count = list->count(list);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

/*
 * 在 ~/.xtools 下面构造一棵受控的目录树, 不依赖仓库内容:
 *     <home>/.xtools/fs_tree_test/a, b            2 个文件
 *     <home>/.xtools/fs_tree_test/sub/c, d        子目录 2 个文件
 *
 * fs_tree 的约定: 对 fs_list 结果做聚合, 子目录递归展开。
 * 预期值全部用 fs_list 现算, 避免不同平台对 "."/".." 条目的
 * 处理差异和仓库文件增减带来的脆断言。
 *
 * 返回是否成功。
 */
static int __test_fs_tree_prepare(char *root, int root_len, char *sub, int sub_len)
{
    char home[128] = {0}, file[192] = {0};
    int ret;

    TRY {
        THROW_IF(fs_gethome(home, sizeof(home)) < 0, -1);
        snprintf(root, root_len, "%s/.xtools/fs_tree_test", home);
        snprintf(sub, sub_len, "%s/.xtools/fs_tree_test/sub", home);

        /* 清理上次可能的残留(目录/文件不存在时这些调用返回 -1, 忽略即可) */
        fs_rmfile(sub);
        fs_rmdir(sub);
        snprintf(file, sizeof(file), "%s/.keep", root);
        fs_rmfile(file);
        snprintf(file, sizeof(file), "%s/a", root);
        fs_rmfile(file);
        snprintf(file, sizeof(file), "%s/b", root);
        fs_rmfile(file);
        fs_rmdir(root);

        /* 借 fs_mkfile 把父目录链(~/.xtools 和 fs_tree_test)建出来 */
        snprintf(file, sizeof(file), "%s/.keep", root);
        fs_mkfile(file, 0777);
        fs_mkdir(sub, 0777);
        snprintf(file, sizeof(file), "%s/a", root);
        fs_mkfile(file, 0777);
        snprintf(file, sizeof(file), "%s/b", root);
        fs_mkfile(file, 0777);
        snprintf(file, sizeof(file), "%s/c", sub);
        fs_mkfile(file, 0777);
        snprintf(file, sizeof(file), "%s/d", sub);
        fs_mkfile(file, 0777);

        /* 验证目录树搭好了 */
        THROW_IF(fs_is_exist(sub) != 1, -1);
    } CATCH (ret) {}

    return ret;
}

/* 删除测试目录树(逐个文件 + rmdir, 平台安全), 供两个测试的 FINALLY 复用 */
static void __test_fs_tree_cleanup(char *root, char *sub)
{
    char file[192] = {0};

    snprintf(file, sizeof(file), "%s/.keep", root);
    fs_rmfile(file);
    snprintf(file, sizeof(file), "%s/a", root);
    fs_rmfile(file);
    snprintf(file, sizeof(file), "%s/b", root);
    fs_rmfile(file);
    snprintf(file, sizeof(file), "%s/c", sub);
    fs_rmfile(file);
    snprintf(file, sizeof(file), "%s/d", sub);
    fs_rmfile(file);
    fs_rmdir(sub);
    fs_rmdir(root);
}

static int test_fs_tree(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    char root[128] = {0}, sub[160] = {0};
    int top, beneath, count, ret;

    TRY {
        EXEC(ret = __test_fs_tree_prepare(root, sizeof(root), sub, sizeof(sub)));
        THROW_IF(ret < 0, -1);

        /* fs_list 算出两层各自的条目数, 作为 fs_tree 的预期值 */
        THROW_IF(__test_fs_tree_count_dir(root, &top) < 0, -1);
        THROW_IF(__test_fs_tree_count_dir(sub, &beneath) < 0, -1);

        list = object_new(allocator, "Vector", NULL);
        count = fs_tree(root, list, -1);
        fs_print_file_info_list(list);
        object_destroy(list);

        SET_CATCH_INT_PARS(count, top + beneath);
        THROW_IF(count < 0, -1);
        /* 深度不限时 = 顶层条目 + 子目录条目 */
        THROW_IF(count != top + beneath, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        __test_fs_tree_cleanup(root, sub);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fs_tree);

static int test_fs_tree_with_depth(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    char root[128] = {0}, sub[160] = {0};
    int top, count, ret;

    TRY {
        EXEC(ret = __test_fs_tree_prepare(root, sizeof(root), sub, sizeof(sub)));
        THROW_IF(ret < 0, -1);

        THROW_IF(__test_fs_tree_count_dir(root, &top) < 0, -1);

        list = object_new(allocator, "Vector", NULL);
        count = fs_tree(root, list, 1);
        fs_print_file_info_list(list);
        object_destroy(list);

        SET_CATCH_INT_PARS(count, top);
        THROW_IF(count < 0, -1);
        /* 深度为 1 时只统计顶层, 子目录不展开 */
        THROW_IF(count != top, -1);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        __test_fs_tree_cleanup(root, sub);
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