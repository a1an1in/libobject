#ifndef __TEST_COMPAT_H__
#define __TEST_COMPAT_H__

/*
 * 测试用例的平台差异:
 *
 * system()/popen() 在 Windows 上走的是 cmd.exe, 不支持 POSIX 的
 * "> /dev/null 2>&1" 写法(会被当成重定向到一个叫 "/dev/null" 的文件),
 * 也不支持 "rm -rf" 这种命令, 所以这里统一用宏区分。
 */
#if (defined(WINDOWS_USER_MODE))
#define TEST_REDIR_NULL    " > NUL 2>&1"
#define TEST_REDIR_ERR     " 2>NUL"
#else
#define TEST_REDIR_NULL    " > /dev/null 2>&1"
#define TEST_REDIR_ERR     " 2>/dev/null"
#endif

/*
 * 外部工具未安装时的跳过判定, 给互测试用例用:
 * unix: system() 对不存在的命令返回 (127<<8);
 * windows: MinGW 的 system() 对不存在的命令返回 1 (与真实失败不可区分),
 * 因此 windows 侧的跳过判定改用 test_tool_exists 做存在性检查.
 */
#if (defined(WINDOWS_USER_MODE))
#define TEST_CMD_NOT_FOUND(s)   ((s) == -1 || (s) == 9009 || (s) == 127 || (s) == (127 << 8))
#else
#define TEST_CMD_NOT_FOUND(s)   ((s) == -1 || (s) == (127 << 8))
#endif

/* 工具是否存在: windows 用 where, unix 用 command -v */
static inline int test_tool_exists(const char *tool)
{
    char cmd[512];

#if (defined(WINDOWS_USER_MODE))
    snprintf(cmd, sizeof(cmd), "where %s > NUL 2>&1", tool);
#else
    snprintf(cmd, sizeof(cmd), "command -v %s > /dev/null 2>&1", tool);
#endif

    return system(cmd) == 0;
}

#endif
