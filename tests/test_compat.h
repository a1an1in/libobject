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

#endif
