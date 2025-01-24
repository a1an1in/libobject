/**
 * @file UnixAttacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */
#if (!defined(WINDOWS_USER_MODE))
#include "UnixAttacher.h"

extern void *testlib_dlopen(char *name, int flag);

static int __construct(UnixAttacher *attacher, char *init_str)
{
    return 0;
}

static int __deconstruct(UnixAttacher *attacher)
{
    return 0;
}

static int __attach(UnixAttacher *attacher, int pid)
{
    int ret, stat;

    TRY {
        dbg_str(DBG_VIP, "attach pid:%d", pid);
        EXEC(ptrace(PTRACE_ATTACH, pid, NULL, NULL));  
        wait(NULL);
        ((Attacher *)attacher)->pid = pid;
    } CATCH (ret) {}

    return ret;
}

static int __detach(UnixAttacher *attacher)
{
    int ret;

    TRY {
        dbg_str(DBG_VIP, "dettach pid:%d", ((Attacher *)attacher)->pid);
        EXEC(ptrace(PTRACE_DETACH, ((Attacher *)attacher)->pid, NULL, NULL));
        ((Attacher *)attacher)->pid = 0;
    } CATCH (ret) {}

    return ret;
}

static void *__get_function_address(UnixAttacher *attacher, char *name, char *module_name)
{
    void *addr;
    int ret;

    TRY {
        /* get local fuction address */
        addr = dl_get_func_addr_by_name(name, module_name);
        THROW_IF(addr == NULL, -1);
        addr = dl_get_remote_function_adress(((Attacher *)attacher)->pid, module_name, addr);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher get_remote_function_address, function remote address:%p", addr);
    } CATCH (ret) { addr = NULL; }

    return addr;
}

static void *__read(UnixAttacher *attacher, void *addr, uint8_t *value, int len)
{
    int ret, num, i;
    long *p = (long *)value;
    long tmp;

    TRY {
        THROW_IF(addr == NULL || value == NULL, -1);
        dbg_str(DBG_VIP, "attacher read addr:%p, len:%d", addr, len);
        num = (len + sizeof(long) - 1) / sizeof(long);

        for (i = 0; i < num; i++) {
            tmp = ptrace(PTRACE_PEEKDATA, ((Attacher *)attacher)->pid, addr + i * sizeof(long), NULL);
            dbg_str(DBG_VIP, "peek addr:%p, value:%llx, i:%d, num:%d", 
                    addr + i * sizeof(long), tmp, i, num);
            dbg_str(DBG_VIP, "value address:%p", (long *)value + i * sizeof(long));
            *(long *)(value + i * sizeof(long)) = tmp;
        }
    } CATCH (ret) { }
    
    return ret;
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
            dbg_str(DBG_DETAIL, "poke write addr:%p, value:%x, i:%d, num:%d, sizeof long:%d", 
                    addr + i * sizeof(long), *(p + i), i, num, sizeof(long));
            tmp = *(p + i);
            dbg_buf(DBG_DETAIL, "write:", (p + i), sizeof(long));
            EXEC(ptrace(PTRACE_POKEDATA, ((Attacher *)attacher)->pid, addr + i * sizeof(long), *(p + i)));
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
            /* 预留返回地址， 再加上进入函数后首先会把rbp入栈， 函数会用0x10(%rbp)的
             * 方式来访问存在栈中的第7个参数
             */
            regs->rsp -= sizeof(void *); 
        } else {
            /* 这里必须要预留3 * sizeof(void *) 的大小， 原因还不清楚， 否则sprintf，
             * dlopen等函数调用会中断返回。 */
            regs->rsp -= 3 * sizeof(void *);  
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
        dbg_str(DBG_DETAIL, "call_address_with_value_pars, func address:%p, pars num:%d", 
                function_address, num);

        EXEC(ptrace(PTRACE_GETREGS, ((Attacher *)attacher)->pid, NULL, &regs));
        memcpy(&bak, &regs, sizeof(regs));
        EXEC(attacher->set_function_pars(attacher, &regs, paramters, num));

        regs.rip = function_address;
        regs.rax = 0;
        EXEC(ptrace(PTRACE_SETREGS, ((Attacher *)attacher)->pid, NULL, &regs));
        EXEC(ptrace(PTRACE_CONT, ((Attacher *)attacher)->pid, NULL, NULL));

        waitpid(((Attacher *)attacher)->pid, &stat, WUNTRACED);
        // printf("stat:%x\n", stat);
        while (stat != 0xb7f) {
            waitpid(((Attacher *)attacher)->pid, &stat, WUNTRACED);
            // printf("stat:%x\n", stat);
        }
        EXEC(ptrace(PTRACE_GETREGS, ((Attacher *)attacher)->pid, NULL, &regs));
        EXEC(ptrace(PTRACE_SETREGS, ((Attacher *)attacher)->pid, NULL, &bak));

        dbg_str(DBG_VIP, "call_address_with_value_pars function_address:%p, return value:%llx", function_address, regs.rax);
        return regs.rax;
    } CATCH (ret) {}

    return ret;
}

/* 
 * we can see from add_lib implementation, the attacher depends on
 * the dl library, if the exe not link dl, the attacher may not useable,
 * but we can solve this problem by inject lib to the exe file.
 **/
static int __add_lib(UnixAttacher *attacher, char *name)
{
    int ret;
    void *handle = NULL;
    Map *map = ((Attacher *)attacher)->map;
    attacher_paramater_t pars[2] = {{name, strlen(name)}, {RTLD_LOCAL | RTLD_LAZY, 0}};

    TRY {
        THROW_IF(name == NULL, -1);
        EXEC(map->search(map, name, &handle));
        THROW_IF(handle != NULL, 0);

        handle = attacher->call_from_lib(attacher, "testlib_dlopen", pars, 2, "libobject-testlib.so");
        // handle = attacher->call_from_lib(attacher, "dlopen", pars, 2, "libdl");
        dbg_str(DBG_VIP, "attacher add_lib, lib name:%s, flag:%x, handle:%p", 
                name, RTLD_LOCAL | RTLD_LAZY, handle);
        THROW_IF(handle == NULL, -1);
        map->add(map, name, handle);
    } CATCH (ret) {}

    return ret;
}

static int __run(UnixAttacher *attacher)
{
    int ret;
    
    ret = ptrace(PTRACE_CONT, ((Attacher *)attacher)->pid, NULL, NULL);
    printf("attacher run, ret:%d\n", ret);
    return ret;
}

static int __stop(UnixAttacher *attacher)
{
    int stat;
    printf("attacher stop\n");
    kill(((Attacher *)attacher)->pid, SIGSTOP); 
    waitpid(((Attacher *)attacher)->pid, &stat, 0);
    
    return 1;
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
    Init_Vfunc_Entry( 8, UnixAttacher, malloc, NULL),
    Init_Vfunc_Entry( 9, UnixAttacher, free, NULL),
    Init_Vfunc_Entry(10, UnixAttacher, get_remote_function_address, __get_function_address),
    Init_Vfunc_Entry(11, UnixAttacher, call_address_with_value_pars, __call_address_with_value_pars),
    Init_Vfunc_Entry(12, UnixAttacher, call_address, NULL),
    Init_Vfunc_Entry(13, UnixAttacher, call_from_lib, NULL),
    Init_Vfunc_Entry(14, UnixAttacher, add_lib, __add_lib),
    Init_Vfunc_Entry(15, UnixAttacher, remove_lib, NULL),
    Init_Vfunc_Entry(16, UnixAttacher, run, __run),
    Init_Vfunc_Entry(17, UnixAttacher, stop, __stop),
    Init_End___Entry(18, UnixAttacher),
};
REGISTER_CLASS(UnixAttacher, attacher_class_info);

#endif