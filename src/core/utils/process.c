#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libobject/core/try.h>

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE))
int process_execv(char *path, char *argv[]) 
{
    int ret;
    pid_t pid = fork(); // 创建子进程

    TRY {
        THROW_IF(pid == -1, -1);
    
        if (pid > 0) {
            // 父进程
            dbg_str(DBG_VIP, "process_execv pid:%d", pid);
            THROW(pid);
        } else if (pid == 0) {
            // 子进程
            EXEC(execv(path, argv)); // 替换进程执行ls命令
            // 如果execl返回说明执行失败
            THROW(-1);
        }
    } CATCH (ret) {}
 
    return ret;
}

int process_kill(int pid)
{
    int status;
    int ret;
    char tmp[10];
    pid_t child_pid = fork(); // 创建子进程

    TRY {
        THROW_IF(pid <= 0, -1);
        THROW_IF(child_pid == -1, -1);
    
        if (child_pid > 0) {
            // 父进程
            pid_t wpid = waitpid(child_pid, &status, 0); // 等待子进程结束
            THROW_IF(wpid == -1, -1);
        } else if (child_pid == 0) {
            // 子进程
            dbg_str(DBG_VIP, "process_kill pid:%d", pid);
            snprintf(tmp, 10, "%d", pid);
            EXEC(execlp("kill", "kill", "-9", tmp, NULL)); // 替换进程执行ls命令
            // 如果execl返回说明执行失败
            THROW(-1);
        }
    } CATCH (ret) {}
 
    return ret;
}
#endif