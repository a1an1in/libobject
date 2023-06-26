#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <stdlib.h>     // for atoi
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>  //for errno
#include <sys/user.h> /* For user_regs_struct */
#include <dlfcn.h>
#include <string.h>
#include <sys/syscall.h> //for gettid
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#define gettid() syscall(__NR_gettid)

/*
 * refer to:https://blog.csdn.net/wangquan1992/article/details/108471212
 */
static int test_attach(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;   

    TRY {
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        tpid = atoi(argv[1]);

        EXEC(ptrace(PTRACE_ATTACH, tpid, NULL, NULL));  
        wait(NULL);
        
        EXEC(ptrace(PTRACE_GETREGS, tpid,NULL, &regs));
        printf("RIP: %llx,RSP: %llx\n", regs.rip, regs.rsp);

        EXEC(ptrace(PTRACE_DETACH, tpid, NULL, NULL));
        printf("detach ok...\n");
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_CMD(test_attach);