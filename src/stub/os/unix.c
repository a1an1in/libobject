#if (!defined(WINDOWS_USER_MODE))
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>
#include <libobject/stub/stub.h>

static inline void *pageof(const void *p)
{
    long long pagesize = sysconf(_SC_PAGE_SIZE);
    return (void *)((unsigned long)p & ~(pagesize - 1));
}

int stub_add(stub_t *stub, void *src, void *dst)
{
    unsigned long dstaddr = (unsigned long)dst;
    // 系统页大小
    int pagesize = sysconf(_SC_PAGESIZE);                                       
    // JMP远跳只支持32位程序 64位程序地址占8个字节 寻址有问题
    unsigned char jmpcmd[STUB_REPLACE_CODE_SIZE] = {0};                         
    int ret;

    TRY {
        stub->fn = src;
        memcpy(stub->code_buf, src, STUB_REPLACE_CODE_SIZE);
        // 使用mprotect函数使该页的内存可读可写可执行
        EXEC(mprotect(pageof(src), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC));  
        /*
         *当JMP指令为 FF 25 00 00 00 00时，会取下面的8个字节作为跳转地址
         */
        jmpcmd[0] = 0xFF;                                                        
        /*
         * 因此可以使用14个字节作为指令 (FF 25 00 00 00 00) + dstaddr
         */
        jmpcmd[1] = 0x25;                                                        
        jmpcmd[2] = 0x00;                                                              
        jmpcmd[3] = 0x00;
        jmpcmd[4] = 0x00;
        jmpcmd[5] = 0x00;
        memcpy(&jmpcmd[6], &dstaddr, sizeof(dstaddr));                                 
        memcpy(src, jmpcmd, sizeof(jmpcmd));   
        __clear_cache(pageof(src), pageof(src) + pagesize);

        /*
         *__builtin___clear_cache(src, src + STUB_REPLACE_CODE_SIZE);
         */
        /*
         *使用mprotect函数使该页的内存可读可写可执行
         */
        EXEC(mprotect(pageof(src), pagesize, PROT_READ | PROT_EXEC));  
    } CATCH (ret) {}

    return ret;
}

int stub_remove(stub_t *stub)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    int ret;

    TRY {
        THROW_IF(stub == NULL, -1);
        THROW_IF(stub->fn == NULL, -1);

        EXEC(mprotect(pageof(stub->fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC));
        memcpy(stub->fn, stub->code_buf, STUB_REPLACE_CODE_SIZE);
        EXEC(mprotect(pageof(stub->fn), pagesize * 2, PROT_READ | PROT_EXEC));
        // memset(stub, 0, sizeof(struct stub_s));
    } CATCH (ret) {}

    return ret;
}

int stub_parse_context(void *exec_code_addr, void *rsp)
{
    stub_t *stub;
    stub_exec_area_t *area;
    stub_func_t func;
    void *p[20] = {0};

    area = exec_code_addr - 42;
    int i = 0, j = 0;
    void **par_addr = (void **)(rsp);
    stub = area->stub;

    dbg_str(DBG_DETAIL,"stub_parse_context, area:%p par_addr:%p, par_count:%d", area, par_addr, stub->para_count);
    
    while(i < stub->para_count) {
        /* 
         * 如果参数大于6个， 需要跳过rsp里面存放的第6和7个数据，
         * 里面存放的是bsp和返回地址.
         **/
        if (j == 6 || j == 7) {
            j++;
            continue;
        }
        p[i] = par_addr[j];
        // printf("p[%d]:%p\n", i, p[i]);
        i++;
        j++;
    }

    if (stub->pre != NULL) {
        func = (stub_func_t)stub->pre;
        func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
             p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
    }

    if (stub->new_fn != stub->fn) {
        func = (stub_func_t)stub->new_fn;
        func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
             p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
    } else {
        stub_remove(stub);
        func = (stub_func_t)stub->fn;
        func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
             p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
        stub_add(stub, stub->fn, stub->new_fn);
    }
    
    if (stub->post != NULL) {
        func = (stub_func_t)stub->post;
        func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
             p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
    }

    return 1;
}

int stub_config_exec_area(stub_t *stub)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    int ret;

    TRY {
        THROW_IF(stub->area == NULL, -1);
        EXEC(mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC));  
        stub->area->stub = stub;
        stub->area_flag = 1;
        mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_EXEC);  
    } CATCH (ret) {}

    return ret;
}

int stub_add_hooks(stub_t *stub, void *func, void *pre, void *new_fn, void *post, int para_count)
{
    /*
     * %rdi，%rsi，%rdx，%rcx，%r8，%r9 用作函数参数，依次对应第1参数，第2参数。。,
     * 代码块存放p0-p5 6个参数到栈里面的存放顺序是为了后面方便用数组来取。超过6个
     * 的参数函数调用前已经安顺序被放在rsp里面， 但是后面取的时候注意要去掉返回地址
     * 和bsp两个数据。
     */
    unsigned char code[54] = {
        0x55,                                       //push   %rbp
        0x48, 0x89, 0xe5,                           //mov    %rsp,%rbp
        0x48, 0x83, 0xec, 0x30,                     //sub    $0x30,%rsp
        0x4c, 0x89, 0x4d, 0xf8,                     //mov    %r9,-0x8(%rbp)，  把参数5 放到栈  
        0x4c, 0x89, 0x45, 0xf0,                     //mov    %r8,-0x10(%rbp)， 把参数4 放到栈
        0x48, 0x89, 0x4d, 0xe8,                     //mov    %rcx,-0x18(%rbp)  ...
        0x48, 0x89, 0x55, 0xe0,                     //mov    %rdx,-0x20(%rbp)
        0x48, 0x89, 0x75, 0xd8,                     //mov    %rsi,-0x28(%rbp)
        0x48, 0x89, 0x7d, 0xd0,                     //mov    %rdi,-0x30(%rbp)，把参数0 放到栈
        0x48, 0x89, 0xe6,                           //mov    %rsp,%rsi       ，把rsp地址传给stub_parse_context参数1
        0x48, 0x8d, 0x3d, 0x00, 0x00, 0x00, 0x00,	//lea    0x0(%rip),%rdi  , 把rip地址(call语句的地址）传给stub_parse_context参数0
        0xe8, 0xfd, 0x17, 0x00, 0x00,               //callq  0x401a8a <func5>
        0x48, 0x83, 0xc4, 0x30,                     //add    $0x30,%rsp
        0x5d,                                       //pop    %rbp
        0xc3,                                       //retq
    };
    int pagesize = sysconf(_SC_PAGESIZE);
    int ret;
    int call_inst_addr_offset = 43;
    int *addr = code + call_inst_addr_offset;
    int call_inst_addr_len = 4;

    TRY {
        *addr = ((long long)stub_parse_context - (long long)stub->area->exec_code) - call_inst_addr_offset - call_inst_addr_len;
        mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);  
        memcpy(stub->area->exec_code, code, sizeof(code));
        mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_EXEC);  
        // printf("offset:%x\n", *addr);

        stub->pre = pre;
        stub->new_fn = new_fn;
        stub->fn = func;
        stub->post = post;
        stub->para_count = para_count;
        dbg_str(DBG_DETAIL,"pre:%p, func:%p, post:%p, stubed_func:%p", 
                stub->pre, stub->new_fn, stub->post, stub->fn);
        EXEC(stub_add(stub, func, stub->area->exec_code));
    } CATCH (ret) {}

    return ret;
}

#endif
