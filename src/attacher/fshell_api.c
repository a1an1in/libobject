/**
 * @file api.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-04-03
 */

#include <stdlib.h>
#include <signal.h>
#include <libobject/core/utils/string.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/net/bus/bus.h>
#include "libobject/stub/stub.h"

#if (!defined(WINDOWS_USER_MODE))
#include <sys/user.h>
#include <sys/ptrace.h>
int fsh_attach(FShell *shell, char *pid)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;   

    TRY {
        THROW_IF(pid == NULL, -1);
        dbg_str(DBG_VIP, "fsh_attach pid:%s", pid);
        tpid = atoi(pid);
        EXEC(ptrace(PTRACE_ATTACH, tpid, NULL, NULL));  
        wait(NULL);
    } CATCH (ret) { }

    return ret;
}

int fsh_dettach(FShell *shell, char *pid)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;

    TRY {
        THROW_IF(pid == NULL, -1);
        dbg_str(DBG_VIP, "fsh_dettach pid:%s", pid);
        tpid = atoi(pid);
        EXEC(ptrace(PTRACE_DETACH, tpid, NULL, NULL));
    } CATCH (ret) { }

    return ret;
}
#endif