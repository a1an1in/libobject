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

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);

void * get_func_addr_by_name(char *name)
{
    void *handle = NULL;
    void *addr = NULL;

    /* open the needed object */
    handle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return NULL;
    }
    dbg_str(DBG_DETAIL, "handle=%p", handle);

    /* find the address of function and data objects */
    addr = dlsym(handle, name);
    dbg_str(DBG_DETAIL, "addr=%p", addr);
    dlclose(handle);

    return addr;
}

int get_func_name_by_addr(void *addr, char *name, unsigned int name_len)
{
    void *handle = NULL;
    Dl_info dl;
    int ret = 0;

    /* open the needed object */
    handle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return -1;
    }

    /* find the address of function and data objects */
    ret = dladdr(addr, &dl);
    dlclose(handle);

    if (ret == 0) {
        return -1;
    }

    strncpy(name, dl.dli_sname, name_len - 1);

    return 0;
}

int test_get_func_addr(TEST_ENTRY *entry)
{
    char *func_name = "print_outbound";
    void *expect_addr = print_outbound;
    void *addr;
    int ret;

    TRY {
        addr = get_func_addr_by_name(func_name);
        THROW_IF(addr == NULL, -1);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_get_func_addr, addr=%p, %s:%p",
                addr, func_name, expect_addr);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_get_func_addr);

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
        addr = get_func_addr_by_name("attach_test_func");
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

static int test_hello_world(char *str)
{
    printf(str);

    return 0;
}


static int test_mmap_and_exec_shellcode(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;
    int (*addr)();
    uint8_t shellcode[] = {
        0xf3, 0x0f, 0x1e, 0xfa,                 //endbr64 
        0x55,                                   //push   %rbp
        0x48, 0x89, 0xe5,                       //mov    %rsp,%rbp
        0x48, 0x83, 0xec, 0x10,                 //sub    $0x10,%rsp
        0x48, 0x89, 0x7d, 0xf8,                 //mov    %rdi,-0x8(%rbp)
        0x48, 0x8b, 0x45, 0xf8,                 //mov    -0x8(%rbp),%rax
        0x48, 0x89, 0xc7,                       //mov    %rax,%rdi
        0xb8, 0x00, 0x00, 0x00, 0x00,           //mov    $0x0,%eax
        0xe8, 0x1d, 0x8a, 0xfd, 0xff,           //callq  0x264a0 <puts@plt>
        0xb8, 0x00, 0x00, 0x00, 0x00,           //mov    $0x0,%eax
        0xc9,                                   //leaveq
        0xc3,                                   //retq  
    };
    int *call_address_offset = shellcode + 29;

    TRY {
        dbg_str(DBG_VIP, "test_mmap ...");
        addr = (int (*)())mmap(NULL, 100, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        dbg_str(DBG_VIP, "mmap addr:%p, printf addr:%p", addr + 33, printf);
        dbg_str(DBG_VIP, "call addr:%p",  (long long)printf - (long long)addr - 33);
        *call_address_offset = (long long)printf - (long long)addr - 33;
        memcpy(addr, shellcode, sizeof(shellcode));
        
        addr("hello world\n");
    } CATCH (ret) { } FINALLY {
        munmap(addr, 100);
    }

    return ret;
}
REGISTER_TEST_CMD(test_mmap_and_exec_shellcode);


#endif

