/**
 * @file UnixAttacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */
#if (!defined(WINDOWS_USER_MODE))
#include "UnixAttacher.h"

static int __construct(UnixAttacher *attacher, char *init_str)
{
    return 0;
}

static int __deconstruct(UnixAttacher *attacher)
{
    if (attacher->pid != 0) {
        attacher->detach(attacher);
    }

    return 0;
}

static int __attach(UnixAttacher *attacher, int pid)
{
    int ret, stat;

    TRY {
        dbg_str(DBG_VIP, "attach pid:%d", pid);
        EXEC(ptrace(PTRACE_ATTACH, pid, NULL, NULL));  
        wait(NULL);
        attacher->pid = pid;
    } CATCH(ret) {}

    return ret;
}

static int __detach(UnixAttacher *attacher)
{
    int ret;

    TRY {
        dbg_str(DBG_VIP, "dettach pid:%d", attacher->pid);
        EXEC(ptrace(PTRACE_DETACH, attacher->pid, NULL, NULL));
        attacher->pid = 0;
    } CATCH(ret) {}

    return ret;
}

//https://blog.csdn.net/KaiKaiaiq/article/details/114378122
static int __call(UnixAttacher *attacher, char *function_address, void *paramters, int num)
{
    int ret, stat;
    struct user_regs_struct regs, bak;

    TRY {
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        memcpy(&bak, &regs, sizeof(regs));
        printf("RIP: %llx,RSP: %llx\n", regs.rip, regs.rsp);
        regs.rip = 0x7fe678922237;
        // regs.rsp -= 16;
        regs.rax = 0;
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid,NULL, &regs));
        EXEC(ptrace(PTRACE_CONT, attacher->pid, NULL, NULL));

        waitpid(attacher->pid, &stat, WUNTRACED);
        printf("stat:%x\n", stat);
        while(stat != 0xb7f) {
            waitpid(attacher->pid, &stat, WUNTRACED);
            printf("stat:%x\n", stat);
        }
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid,NULL, &bak));
        printf("return value:%x\n", regs.rax);
        THROW(regs.rax);
    } CATCH(ret) {}

    return ret;
}

static int __get_function_address(UnixAttacher *attacher, char *function_name, char *module_name)
{

}

static int __add_lib(UnixAttacher *attacher, char *name)
{

}

static int __remove_lib(UnixAttacher *attacher, char *name)
{

}

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry(0, Attacher, parent),
    Init_Nfunc_Entry(1, UnixAttacher, construct, __construct),
    Init_Nfunc_Entry(2, UnixAttacher, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, UnixAttacher, attach, __attach),
    Init_Vfunc_Entry(4, UnixAttacher, detach, __detach),
    Init_Vfunc_Entry(5, UnixAttacher, call, __call),
    Init_Vfunc_Entry(6, UnixAttacher, get_function_address, __get_function_address),
    Init_Vfunc_Entry(7, UnixAttacher, add_lib, __add_lib),
    Init_Vfunc_Entry(8, UnixAttacher, remove_lib, __remove_lib),
    Init_End___Entry(9, UnixAttacher),
};
REGISTER_CLASS("UnixAttacher", attacher_class_info);

#endif