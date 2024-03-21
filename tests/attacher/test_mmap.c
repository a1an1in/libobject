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
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>

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

