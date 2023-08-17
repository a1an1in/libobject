/**
 * @file UnixAttacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */
#if (!defined(WINDOWS_USER_MODE))
#include "UnixAttacher.h"

extern void *my_malloc(int size);
extern int my_free(void *addr);

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
        dbg_str(DBG_VIP, "attacher write addr:%p, len:%d", addr, len);
        num = (len + sizeof(long) - 1) / sizeof(long);

        for (i = 0; i < num; i++) {
            dbg_str(DBG_VIP, "poke write addr:%p, value:%x, i:%d, num:%d, sizeof long:%d", 
                    addr + i * sizeof(long), *(p + i), i, num, sizeof(long));
            tmp = *(p + i);
            dbg_buf(DBG_VIP, "write:", (p + i), sizeof(long));
            EXEC(ptrace(PTRACE_POKEDATA, attacher->pid, addr + i * sizeof(long), *(p + i)));
        }

        // memset(value, 0, len);
        // for (i = 0; i < num; i++) {
        //     tmp = ptrace(PTRACE_PEEKDATA, attacher->pid, addr, NULL);
        //     dbg_buf(DBG_VIP, "peek data:", &tmp, sizeof(long));
        // }

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

/* 
 * description:
 * this funtion can only call remote function which's paramters has no pointer.
 * 
 * refer to: //https://blog.csdn.net/KaiKaiaiq/article/details/114378122
 **/
static long __call_address_with_value_pars(UnixAttacher *attacher, void *function_address, void *paramters, int num)
{
    int ret, stat;
    struct user_regs_struct regs, bak;

    TRY {
        dbg_str(DBG_VIP, "call_address_with_value_pars, func address:%p, sizeof(long):%d", function_address, sizeof(long));
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        memcpy(&bak, &regs, sizeof(regs));
        // printf("RIP: %llx,RSP: %llx\n", regs.rip, regs.rsp);
        if (num != 0) {
            EXEC(attacher->set_function_pars(attacher, &regs, paramters, num));
        }

        regs.rip = function_address;
        regs.rax = 0;
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid, NULL, &regs));
        EXEC(ptrace(PTRACE_CONT, attacher->pid, NULL, NULL));

        waitpid(attacher->pid, &stat, WUNTRACED);
        // printf("stat:%x\n", stat);
        while(stat != 0xb7f) {
            waitpid(attacher->pid, &stat, WUNTRACED);
            // printf("stat:%x\n", stat);
        }
        EXEC(ptrace(PTRACE_GETREGS, attacher->pid,NULL, &regs));
        EXEC(ptrace(PTRACE_SETREGS, attacher->pid,NULL, &bak));

        dbg_str(DBG_VIP, "call_address_with_value_pars, return value:%llx", regs.rax);
        return regs.rax;
    } CATCH(ret) {}

    return ret;
}

static void *__malloc(UnixAttacher *attacher, int size, void *value)
{
    int ret;
    void *addr;
    long long pars[1] = {size};

    TRY {
        THROW_IF(size == 0, 0);
        // addr = attacher->get_function_address(attacher, malloc, "libc.so");
        addr = attacher->get_function_address(attacher, my_malloc, "libobject-testlib.so");
        THROW_IF(addr == NULL, -1);
        addr = attacher->call_address_with_value_pars(attacher, addr, pars, 1);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher malloc addr:%p, size:%d", addr, size);
        if (value != NULL) {
            EXEC(attacher->write(attacher, addr, value, size));
        }
        return addr;
    } CATCH (ret) {}

    return ret;
}

static int __free(UnixAttacher *attacher, void *addr)
{
    int ret;
    void *free_addr;
    long long pars[1] = {addr};

    TRY {
        THROW_IF(addr == 0, -1);
        // free_addr = attacher->get_function_address(attacher, my_free, "libc.so");
        free_addr = attacher->get_function_address(attacher, my_free, "libobject-testlib.so");
        THROW_IF(free_addr == NULL, -1);
        dbg_str(DBG_VIP, "free func addr:%p, free addr:%p", free_addr, addr);
        ret = attacher->call_address_with_value_pars(attacher, free_addr, pars, 1);
        THROW(ret);
    } CATCH (ret) {}

    return ret;
}

/* name:call_address
 *
 * description:
 * this funtion can call function with remote address. if we know the remote address
 * we can call this method.
 **/
static long __call_address(UnixAttacher *attacher, void *function_address, attacher_paramater_t pars[], int num)
{
    long ret, stat, i;
    int pointer_flag = 0;
    void *paramters[ATTACHER_PARAMATER_ARRAY_MAX_SIZE];

    TRY {
        THROW_IF(num > ATTACHER_PARAMATER_ARRAY_MAX_SIZE, -1);

        /* retmote funtion args pre process*/
        for (i = 0; i < num; i++) {
            if (pars[i].size == 0) continue;
            /* We require size to be multiples of long because ptrace is written in units of long */
            THROW_IF((pars[i].size % sizeof(long) != 0), -1);
            dbg_str(DBG_VIP, "call address prepare paramater %d", i);
            paramters[i] = attacher->malloc(attacher, pars[i].size, pars[i].value);
            pointer_flag = 1;
        }

        if (pointer_flag == 0) {
            ret = attacher->call_address_with_value_pars(attacher, function_address, paramters, num);
            return ret;
        } else {
            ret = attacher->call_address_with_value_pars(attacher, function_address, paramters, num);
        }

        /* retmote funtion args post process */
        for (i = 0; i < num; i++) {
            if (pars[i].size == 0) continue;
            attacher->free(attacher, paramters[i]);
        }
        return ret;
    } CATCH(ret) {}

    return ret;
}

/* name:call_from_lib
 *
 * description:
 * if only we know the func name and which lib this func is, we can call this method. 
 **/

static long __call_from_lib(UnixAttacher *attacher, char *name, attacher_paramater_t pars[], int num, char *module_name)
{
    long ret;
    void *addr;

    TRY {
        /* get local fuction address */
        addr = dl_get_func_addr_by_name(name);
        THROW_IF(addr == NULL, -1);
        /* get remote fuction address */
        addr = attacher->get_function_address(attacher, addr, module_name);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "call from lib, func name:%s, func_addr:%p", name, addr);
        ret = attacher->call_address(attacher, addr, pars, num);
        return ret;
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
    Init_Vfunc_Entry( 8, UnixAttacher, malloc, __malloc),
    Init_Vfunc_Entry( 9, UnixAttacher, free, __free),
    Init_Vfunc_Entry(10, UnixAttacher, get_function_address, __get_function_address),
    Init_Vfunc_Entry(11, UnixAttacher, call_address_with_value_pars, __call_address_with_value_pars),
    Init_Vfunc_Entry(12, UnixAttacher, call_address, __call_address),
    Init_Vfunc_Entry(13, UnixAttacher, call_from_lib, __call_from_lib),
    Init_Vfunc_Entry(14, UnixAttacher, add_lib, __add_lib),
    Init_Vfunc_Entry(15, UnixAttacher, remove_lib, __remove_lib),
    Init_End___Entry(16, UnixAttacher),
};
REGISTER_CLASS("UnixAttacher", attacher_class_info);

#endif