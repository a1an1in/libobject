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

static void *__get_function_address(UnixAttacher *attacher, void *local_func_address, char *module_name)
{
    void *addr;
    int ret;

    TRY {
        addr = dl_get_remote_function_adress(attacher->pid, module_name, local_func_address);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher get_function_address, function remote address:%p", addr);
    } CATCH(ret) { addr = NULL; }

    return addr;
}

static void *__read(UnixAttacher *attacher, void *addr, uint8_t *value, int len)
{

}

static void *__write(UnixAttacher *attacher, void *addr, uint8_t *value, int len)
{
    int ret, num, i;
    long *p = (long *)value;
    long tmp;

    TRY {
        num = (len + sizeof(long) - 1) / sizeof(long);

        for (i = 0; i < num; i++) {
            dbg_str(DBG_VIP, "poke write addr:%p, value:%x, i:%d, num:%d, sizeof long:%d", 
                    addr + i * sizeof(long), *(p + i), i, num, sizeof(long));
            dbg_buf(DBG_VIP, "write:", (p + i), sizeof(long));
            EXEC(ptrace(PTRACE_POKEDATA, attacher->pid, addr + i * sizeof(long), *(p + i)));
        }

        memset(value, 0, len);
        for (i = 0; i < num; i++) {
            tmp = ptrace(PTRACE_PEEKDATA, attacher->pid, addr, NULL);
            dbg_buf(DBG_VIP, "peek data:", &tmp, sizeof(long));
        }

    } CATCH (ret) { }
    
    return ret;
}

static void *__set_function_pars(UnixAttacher *attacher, struct user_regs_struct *regs, void **paramters, int num)
{
    int ret, i;
    unsigned long long int *par_array[6] = {&regs->rdi, &regs->rsi, &regs->rdx, &regs->rcx, &regs->r8, &regs->r9};
    unsigned long long int *addr;

    TRY {
        /*
         * %rdi，%rsi，%rdx，%rcx，%r8，%r9 用作函数参数，依次对应第1参数，第2参数。。,
         * 代码块存放p0-p5 6个参数到栈里面的存放顺序是为了后面方便用数组来取。超过6个
         * 的参数函数调用前已经安顺序被放在rsp里面， 但是后面取的时候注意要去掉返回地址
         * 和bsp两个数据。
         */
        if (num > 6) {
            regs->rsp -= (num - 6) * sizeof(void *);
            addr = regs->rsp;
            dbg_str(DBG_VIP, "attacher call, new rsp:%p", addr);
            /* 预留返回地址， 再加上进入函数后首先会把rbp入栈， 函数取会用0x10(%rbp)的
             * 方式来访问存在栈中的第7个参数
             */
            regs->rsp -= sizeof(void *); 
        }
        for (i = 0; i < num; i++) {
            if (i < 6) {
                *(par_array[i]) =  paramters[i];
            } else {
                dbg_str(DBG_VIP, "write (addr + (num - i - 1) * 8):%p, num:%d, i:%d", addr + (num - i -1), num, i);
                EXEC(attacher->write(attacher, addr + i - 6, paramters + i, sizeof(void *)));
            }
        }
    } CATCH (ret) {}

    return ret;
}

//https://blog.csdn.net/KaiKaiaiq/article/details/114378122
static int __call(UnixAttacher *attacher, void *function_address, void *paramters, int num)
{
    int ret, stat;
    struct user_regs_struct regs, bak;

    TRY {
        dbg_str(DBG_VIP, "attacher call, func address:%p", function_address);
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        memcpy(&bak, &regs, sizeof(regs));
        printf("RIP: %llx,RSP: %llx\n", regs.rip, regs.rsp);
        if (num != 0) {
            EXEC(attacher->set_function_pars(attacher, &regs, paramters, num));
        }

        regs.rip = function_address;
        regs.rax = 0;
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid, NULL, &regs));
        EXEC(ptrace(PTRACE_CONT, attacher->pid, NULL, NULL));

        waitpid(attacher->pid, &stat, WUNTRACED);
        printf("stat:%x\n", stat);
        while(stat != 0xb7f) {
            waitpid(attacher->pid, &stat, WUNTRACED);
            printf("stat:%x\n", stat);
        }
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid,NULL, &bak));
        printf("return value:%llx\n", regs.rax);
        THROW(regs.rax);
    } CATCH(ret) {}

    return ret;
}

static int __add_lib(UnixAttacher *attacher, char *name)
{

}

static int __remove_lib(UnixAttacher *attacher, char *name)
{

}

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry( 0, Attacher, parent),
    Init_Nfunc_Entry( 1, UnixAttacher, construct, __construct),
    Init_Nfunc_Entry( 2, UnixAttacher, deconstruct, __deconstruct),
    Init_Vfunc_Entry( 3, UnixAttacher, attach, __attach),
    Init_Vfunc_Entry( 4, UnixAttacher, detach, __detach),
    Init_Vfunc_Entry( 5, UnixAttacher, set_function_pars, __set_function_pars),
    Init_Vfunc_Entry( 6, UnixAttacher, read, __read),
    Init_Vfunc_Entry( 7, UnixAttacher, write, __write),
    Init_Vfunc_Entry( 8, UnixAttacher, get_function_address, __get_function_address),
    Init_Vfunc_Entry( 9, UnixAttacher, call, __call),
    Init_Vfunc_Entry(10, UnixAttacher, add_lib, __add_lib),
    Init_Vfunc_Entry(11, UnixAttacher, remove_lib, __remove_lib),
    Init_End___Entry(12, UnixAttacher),
};
REGISTER_CLASS("UnixAttacher", attacher_class_info);

#endif