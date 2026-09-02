/**
 * @file test_archive_cli.c
 * @Synopsis
 *     Archive_Command 命令行端到端测试:
 *     直接用 popen 运行真实命令
 *     "<当前 xtools 可执行> archive <子命令> ...",
 *     校验进程退出码、stdout 输出与落盘结果是否符合预期
 *     (覆盖命令行参数解析、魔数识别、分发、打包/解包正确性).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/mockery/mockery.h>

#define CMD_OUT "./tests/archive/output/cmd"
#define CMD_RES "./tests/archive/res/cmd"

/* 定位 xtools 可执行文件(测试用它再跑真实命令):
 * 1) 环境变量 XTOOLS_BIN 优先;
 * 2) 测试本身就跑在 xtools 进程里, Linux 下读 /proc/self/exe 得当前可执行路径,
 *    天然适配任意 sysroot 架构(如 aarch64);
 * 3) 其它平台按平台宏回退到默认 sysroot 路径. */
static const char *__xt_bin(void)
{
    static char path[2048];
    const char *p = getenv("XTOOLS_BIN");

    if (p != NULL && p[0] != '\0') return p;

#if defined(LINUX_USER_MODE)
    {
        ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (n > 0) {
            path[n] = '\0';
            return path;
        }
    }
#elif defined(MAC_USER_MODE)
    return "./sysroot/mac/bin/xtools";
#elif defined(WINDOWS_USER_MODE)
    return "./sysroot/windows/bin/xtools.exe";
#endif

    /* Linux 兜底按架构 */
#if defined(__aarch64__)
    return "./sysroot/linux/aarch64/bin/xtools";
#elif defined(__arm__)
    return "./sysroot/linux/arm/bin/xtools";
#else
    return "./sysroot/linux/x86_64/bin/xtools";
#endif
}

/* 运行命令并捕获 stdout 到 out(out_size); 返回进程退出码 */
static int __cli_run(const char *cmd, char *out, int out_size)
{
    FILE *fp;
    int rc, n = 0;

    fp = popen(cmd, "r");
    if (fp == NULL) return -1;
    if (out != NULL) {
        n = (int)fread(out, 1, out_size - 1, fp);
        out[n] = '\0';
    }
    rc = pclose(fp);
    return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

/* 断言命令退出码为 0 且 stdout 含 expect(可为 NULL); 不符则打日志并返回 0 */
static int __assert_cli(const char *cmd, const char *expect)
{
    char out[16384];
    int rc;

    rc = __cli_run(cmd, out, sizeof(out));
    if (rc != 0) {
        dbg_str(DBG_ERROR, "cli rc=%d, cmd: %s", rc, cmd);
        return 0;
    }
    if (expect != NULL && strstr(out, expect) == NULL) {
        dbg_str(DBG_ERROR, "cli output lacks '%s', cmd: %s\n---- output ----\n%s",
                expect, cmd, out);
        return 0;
    }
    return 1;
}

/* -w 通配符校验(list/extract 只保留匹配项), 放到每个格式 extract 校验之后 */
static int test_cli_wildcard(const char *archive_path)
{
    int ret = 0;
    char cmd[2048], out[8192];

    TRY {
        /* list -w test.txt: res/cmd 三个文件中只匹配 test.txt, 应 total 1 */
        snprintf(cmd, sizeof(cmd), "%s archive list %s -w test.txt", __xt_bin(), archive_path);
        THROW_IF(__assert_cli(cmd, "total 1 entries") != 1, -1);
        THROW_IF(__cli_run(cmd, out, sizeof(out)) != 0, -1);
        THROW_IF(strstr(out, "test2.txt") != NULL, -1);

        /* 先清空 outw, 保证"应不存在"断言不受上次残留影响 */
        fs_rmfile(CMD_OUT "/outw/test.txt");
        fs_rmfile(CMD_OUT "/outw/test2.txt");
        fs_rmfile(CMD_OUT "/outw/add.txt");
        fs_rmdir(CMD_OUT "/outw");

        /* extract -w test.txt: 只应解出 test.txt */
        snprintf(cmd, sizeof(cmd), "%s archive extract %s -o %s/outw -w test.txt", __xt_bin(), archive_path, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/outw/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/outw/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/outw/test2.txt") == 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/outw/add.txt") == 1, -1);
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/outw/test.txt");
        fs_rmfile(CMD_OUT "/outw/test2.txt");
        fs_rmfile(CMD_OUT "/outw/add.txt");
        fs_rmdir(CMD_OUT "/outw");
    }

    return ret;
}

/* ================= 各格式: create -> list -> extract -> 校验 ================= */

static int test_archive_cli_tar(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.tar %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/t.tar") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.tar", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "test.txt") != 1, -1);
        THROW_IF(__assert_cli(cmd, "test2.txt") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive extract %s/t.tar -o %s/out", __xt_bin(), CMD_OUT, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);

        THROW_IF(assert_file_equal(CMD_OUT "/out/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/test2.txt", CMD_RES "/test2.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/add.txt", CMD_RES "/add.txt") != 1, -1);

        /* -w 通配符 list/extract */
        EXEC(test_cli_wildcard(CMD_OUT "/t.tar"));
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.tar");
        fs_rmfile(CMD_OUT "/out/test.txt");
        fs_rmfile(CMD_OUT "/out/test2.txt");
        fs_rmfile(CMD_OUT "/out/add.txt");
        fs_rmdir(CMD_OUT "/out");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_tar);

static int test_archive_cli_zip(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.zip %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/t.zip") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.zip", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "test.txt") != 1, -1);
        THROW_IF(__assert_cli(cmd, "test2.txt") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive extract %s/t.zip -o %s/out", __xt_bin(), CMD_OUT, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);

        THROW_IF(assert_file_equal(CMD_OUT "/out/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/test2.txt", CMD_RES "/test2.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/add.txt", CMD_RES "/add.txt") != 1, -1);

        /* -w 通配符 list/extract */
        EXEC(test_cli_wildcard(CMD_OUT "/t.zip"));
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.zip");
        fs_rmfile(CMD_OUT "/out/test.txt");
        fs_rmfile(CMD_OUT "/out/test2.txt");
        fs_rmfile(CMD_OUT "/out/add.txt");
        fs_rmdir(CMD_OUT "/out");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_zip);

static int test_archive_cli_squashfs(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.sqfs %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/t.sqfs") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.sqfs", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "test.txt") != 1, -1);
        THROW_IF(__assert_cli(cmd, "test2.txt") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive extract %s/t.sqfs -o %s/out", __xt_bin(), CMD_OUT, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);

        THROW_IF(assert_file_equal(CMD_OUT "/out/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/test2.txt", CMD_RES "/test2.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/add.txt", CMD_RES "/add.txt") != 1, -1);

        /* -w 通配符 list/extract */
        EXEC(test_cli_wildcard(CMD_OUT "/t.sqfs"));
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.sqfs");
        fs_rmfile(CMD_OUT "/out/test.txt");
        fs_rmfile(CMD_OUT "/out/test2.txt");
        fs_rmfile(CMD_OUT "/out/add.txt");
        fs_rmdir(CMD_OUT "/out");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_squashfs);

static int test_archive_cli_7z(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.7z %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/t.7z") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.7z", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "test.txt") != 1, -1);
        THROW_IF(__assert_cli(cmd, "test2.txt") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive extract %s/t.7z -o %s/out", __xt_bin(), CMD_OUT, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);

        THROW_IF(assert_file_equal(CMD_OUT "/out/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/test2.txt", CMD_RES "/test2.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/add.txt", CMD_RES "/add.txt") != 1, -1);

        /* -w 通配符 list/extract */
        EXEC(test_cli_wildcard(CMD_OUT "/t.7z"));
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.7z");
        fs_rmfile(CMD_OUT "/out/test.txt");
        fs_rmfile(CMD_OUT "/out/test2.txt");
        fs_rmfile(CMD_OUT "/out/add.txt");
        fs_rmdir(CMD_OUT "/out");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_7z);

static int test_archive_cli_tgz(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.tgz %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);
        THROW_IF(fs_is_exist(CMD_OUT "/t.tgz") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.tgz", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "test.txt") != 1, -1);
        THROW_IF(__assert_cli(cmd, "test2.txt") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive extract %s/t.tgz -o %s/out", __xt_bin(), CMD_OUT, CMD_OUT);
        THROW_IF(__assert_cli(cmd, "extracted") != 1, -1);

        THROW_IF(assert_file_equal(CMD_OUT "/out/test.txt", CMD_RES "/test.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/test2.txt", CMD_RES "/test2.txt") != 1, -1);
        THROW_IF(assert_file_equal(CMD_OUT "/out/add.txt", CMD_RES "/add.txt") != 1, -1);

        /* -w 通配符 list/extract */
        EXEC(test_cli_wildcard(CMD_OUT "/t.tgz"));
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.tgz");
        fs_rmfile(CMD_OUT "/t.tar");     /* 解 gzip 时落盘的中间 tar */
        fs_rmfile(CMD_OUT "/out/test.txt");
        fs_rmfile(CMD_OUT "/out/test2.txt");
        fs_rmfile(CMD_OUT "/out/add.txt");
        fs_rmdir(CMD_OUT "/out");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_tgz);

/* ================= add(仅 tar 支持) ================= */

static int test_archive_cli_add(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    char cmd[2048];

    TRY {
        fs_mkdir(CMD_OUT, 0777);
        snprintf(cmd, sizeof(cmd), "%s archive create %s/t.tar %s", __xt_bin(), CMD_OUT, CMD_RES);
        THROW_IF(__assert_cli(cmd, "created") != 1, -1);

        /* add 一个不在源目录里的已有文件 */
        snprintf(cmd, sizeof(cmd), "%s archive add %s/t.tar ./tests/archive/res/test.txt", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "added") != 1, -1);

        snprintf(cmd, sizeof(cmd), "%s archive list %s/t.tar", __xt_bin(), CMD_OUT);
        THROW_IF(__assert_cli(cmd, "res/test.txt") != 1, -1);
    } CATCH (ret) { } FINALLY {
        fs_rmfile(CMD_OUT "/t.tar");
        fs_rmdir(CMD_OUT);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_add);

/* ================= 非法子命令应报错(退出码非 0) ================= */

static int test_archive_cli_bad_subcmd(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret, rc;
    char cmd[2048], out[4096];

    TRY {
        snprintf(cmd, sizeof(cmd), "%s archive no_such_subcmd x", __xt_bin());
        rc = __cli_run(cmd, out, sizeof(out));
        THROW_IF(rc == 0, -1);   /* 非法子命令必须返回非 0 */
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_archive_cli_bad_subcmd);
