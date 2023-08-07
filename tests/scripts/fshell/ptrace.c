#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

/*
 * refer to:https://blog.csdn.net/wangquan1992/article/details/108471212
 */
static int test_attach(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;
    void *addr = NULL;

    TRY {
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        tpid = atoi(argv[1]);

        EXEC(ptrace(PTRACE_ATTACH, tpid, NULL, NULL));  
        wait(NULL);
        addr = dl_get_func_addr_by_name("attach_test_func");
        sleep(6);
        THROW_IF(addr == NULL, -1);
        
        EXEC(ptrace(PTRACE_GETREGS, tpid,NULL, &regs));
        printf("RIP: %llx,RSP: %llx\n", regs.rip, regs.rsp);

        EXEC(ptrace(PTRACE_DETACH, tpid, NULL, NULL));
        printf("detach ok...\n");
    } CATCH (ret) { }

    return ret;
}
REGISTER_TEST_CMD(test_attach);

#endif

