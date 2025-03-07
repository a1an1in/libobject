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

static void *__get_remote_builtin_function_address(UnixAttacher *attacher, char *name, char *module_name)
{
    void *addr;
    int ret;

    TRY {
        /* get local fuction address */
        addr = dl_get_func_addr_by_name(name, module_name);
        THROW_IF(addr == NULL, -1);
        addr = dl_get_remote_function_adress(((Attacher *)attacher)->pid, module_name, addr);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_INFO, "attacher get_remote_builtin_function_address, %s remote address:%p", name, addr);
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
        dbg_str(DBG_DETAIL, "attacher write addr:%p, len:%d", addr, len);
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

        dbg_str(DBG_INFO, "call_address_with_value_pars function_address:%p, return value:%llx", function_address, regs.rax);
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
    attacher_paramater_t pars[2] = {{name, strlen(name) + 1}, {RTLD_LOCAL | RTLD_NOW, 0}};

    TRY {
        THROW_IF(name == NULL, -1);
        EXEC(map->search(map, name, &handle));
        THROW_IF(handle != NULL, 0);

        if (attacher->count++ >= 1) { //后续的动态库加载使用attacher_dlopen， 如果有错误这个可以打印错误原因。
            handle = ((Attacher *)attacher)->call_name(attacher, "libattacher-builtin.so", "attacher_dlopen", pars, 2);
        } else { //加载第一个使用__libc_dlopen_mode。
            handle = ((Attacher *)attacher)->call_name(attacher, NULL, "__libc_dlopen_mode", pars, 2);
        }

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

DEFINE_CLASS(UnixAttacher,
    CLASS_OBJ___ENTRY(Attacher, parent),
    CLASS_NFUNC_ENTRY(construct, __construct),
    CLASS_NFUNC_ENTRY(deconstruct, __deconstruct),
    CLASS_VFUNC_ENTRY(attach, __attach),
    CLASS_VFUNC_ENTRY(detach, __detach),
    CLASS_VFUNC_ENTRY(set_function_pars, __set_function_pars),
    CLASS_VFUNC_ENTRY(read, __read),
    CLASS_VFUNC_ENTRY(write, __write),
    CLASS_VFUNC_ENTRY(malloc, NULL),
    CLASS_VFUNC_ENTRY(free, NULL),
    CLASS_VFUNC_ENTRY(get_remote_builtin_function_address, __get_remote_builtin_function_address),
    CLASS_VFUNC_ENTRY(call_address_with_value_pars, __call_address_with_value_pars),
    CLASS_VFUNC_ENTRY(call_address, NULL),
    CLASS_VFUNC_ENTRY(add_lib, __add_lib),
    CLASS_VFUNC_ENTRY(remove_lib, NULL),
    CLASS_VFUNC_ENTRY(run, __run),
    CLASS_VFUNC_ENTRY(stop, __stop)
);

#endif