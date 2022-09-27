#if (!defined(WINDOWS_USER_MODE))
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/registry/registry.h>

#define JMP_OFFSET_LEN  5   //JMP指令的长度
#define STUB_REPLACE_CODE_SIZE 14U

typedef struct stub_s stub_t;
typedef int (*stub_func_t)(void * p1, void * p2, void * p3, void * p4, void * p5, 
                void * p6, void * p7, void * p8, void * p9, void * p10,
                void * p11, void * p12, void * p13, void * p14, void * p15, 
                void * p16, void * p17, void * p18, void * p19, void * p20);

typedef struct stub_exec_area {
    unsigned char exec_code[80];
    stub_t *stub;
} stub_exec_area_t;

struct stub_s {
    stub_exec_area_t *area;
    void *reg_bp;
    void *pre;
    void *post;
    void *new_fn;
    void *fn;
    unsigned int para_count;
    int area_flag;
    unsigned char code_buf[STUB_REPLACE_CODE_SIZE];
};

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
    } CATCH (ret) {
    }

    return ret;
}

int stub_remove(stub_t *stub)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    int ret;

    TRY {
        THROW_IF(stub->fn == NULL, -1);

        EXEC(mprotect(pageof(stub->fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC));
        memcpy(stub->fn, stub->code_buf, STUB_REPLACE_CODE_SIZE);
        EXEC(mprotect(pageof(stub->fn), pagesize * 2, PROT_READ | PROT_EXEC));
        memset(stub, 0, sizeof(struct stub_s));
    } CATCH (ret) {
    }

    return ret;
}


int stub_parse_context(void *exec_code_addr, void *reg_bp, void *p1, void *p2, void *p3, void *p4)
{
    stub_t *stub;
    stub_exec_area_t *area;
    stub_func_t func;
    void *p[20] = {0};

    area = exec_code_addr - 48;
    int i = 4;
    void **par_addr = (void **)(reg_bp + 2 * sizeof(void *));
    stub = area->stub;

    printf("exec_code_addr:%p, reg_bp:%p, p1:%p, p2:%p, p3:%p, p4:%p\n", 
           exec_code_addr, reg_bp, p1, p2, p3, p4);
    printf("area:%p par_addr:%p, par_count:%d\n", area, par_addr, stub->para_count);
    printf("pre:%p, func:%p, post:%p, stubed_func:%p\n", stub->pre, stub->new_fn, stub->post, stub->fn);
    p[0] = p1, p[1] = p2, p[2] = p3, p[3] = p4;

    while(i < stub->para_count) {
        p[i] = par_addr[i];
        printf("p[%d]:%x\n", i, p[i]);
        i++;
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

int stub_placeholder()
{
    asm (
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        :
        :
    );
    
    return 0;
}

stub_t *stub_alloc()
{
    stub_t *stub;

    stub = (stub_t *)malloc(sizeof(stub_t));
    stub->area_flag = 0;

    return stub;
}

int stub_alloc_exec_area(stub_t *stub)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小

    stub->area = (stub_exec_area_t *)stub_placeholder;
    
    mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);  
    stub->area->stub = stub;
    stub->area_flag = 1;
    mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_EXEC);  

    return 1;
}

int stub_free_exec_area(stub_t *stub)
{
    return 1;
}

int stub_free(stub_t *stub)
{
    if (stub->area_flag == 1) {
        stub_free_exec_area(stub);
    }
    free(stub);

    return 1;
}

int stub_add_hooks(stub_t *stub, void *func, void *pre, void *new_fn, void *post, int para_count)
{
    unsigned char code[80] = {
        0x55,                                       //push   %rbp
        0x48, 0x89, 0xe5,                           //mov    %rsp,%rbp
        0x48, 0x83, 0xec, 0x30,                     //sub    $0x30,%rsp
        0x44, 0x89, 0x4c, 0x24, 0x28,               //mov    %r9d,0x28(%rsp)
        0x44, 0x89, 0x44, 0x24, 0x20,               //mov    %r8d,0x20(%rsp)
        0x48, 0x89, 0x54, 0x24, 0x18,               //mov    %rdx,0x18(%rsp)
        0x48, 0x89, 0x4c, 0x24, 0x10,               //mov    %rcx,0x10(%rsp)
        0x44, 0x8b, 0x4c, 0x24, 0x18,	            //mov    0x18(%rsp),%r9d
        0x44, 0x8b, 0x44, 0x24, 0x10,	            //mov    0x10(%rsp),%r8d
        0x48, 0x89, 0xea,	                        //mov    %rbp,%rdx
        0x48, 0x8d, 0x0d, 0x00, 0x00, 0x00, 0x00,	//lea    0x0(%rip),%rcx        # 0x401a99 <func4+35>
        0xe8, 0xfd, 0x17, 0x00, 0x00,               //callq  0x401a8a <func5>
        0x48, 0x83, 0xc4, 0x30,                     //add    $0x30,%rsp
        0x5d,                                       //pop    %rbp
        0xc3,                                       //c3	 retq
    };
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    int ret;
    int call_inst_addr_offset = 49;
    int *addr = code + call_inst_addr_offset;
    int call_inst_addr_len = 4;

    stub_alloc_exec_area(stub);
    *addr = ((long long)stub_parse_context - (long long)stub->area->exec_code) - call_inst_addr_offset - call_inst_addr_len;
    mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);  
    memcpy(stub->area->exec_code, code, sizeof(code));
    mprotect(pageof(stub->area), pagesize, PROT_READ | PROT_EXEC);  
    printf("offset:%x\n", *addr);

    stub->pre = pre;
    stub->new_fn = new_fn;
    stub->fn = func;
    stub->post = post;
    stub->para_count = para_count;
    printf("pre:%p, func:%p, post:%p, stubed_func:%p\n", stub->pre, stub->new_fn, stub->post, stub->fn);
    stub_add(stub, func, stub->area->exec_code);

    return 0;
}

int stub_remove_hooks(stub_t *stub)
{
    stub_free_exec_area(stub);
    stub_remove(stub);

    return 0;
}
int test_funcA(int *a, int *b, int *c)
{
    printf("call funcA, a:%d, b:%d, c:%d\n", *a, *b, *c);
    *a = *b = *c = 1;
    return *a + *b + *c;
}

int test_funcB(int *a, int *b, int *c)
{
    int t;
    printf("call funcB, a:%d, b:%d, c:%d\n", *a, *b, *c);
    t = *a;
    *a = *c;
    *c = t;

    return *a + *b + *c;
}

int test_stub_add1()
{
    char ac[10] = {1};
    stub_t stub;
    int a = 1, b = 2, c = 3;
    int ret;

    TRY {
        stub_add(&stub, (void*)test_funcA, (void*)test_funcB);  // 添加动态桩 用B替换A
        ret = test_funcA(&a, &b, &c);
        THROW_IF(ret != (a + b + c), -1);
        THROW_IF(a != 3 || c != 1, -1);

        stub_remove(&stub);  // 添加动态桩 用B替换A
        ret = test_funcA(&a, &b, &c);
        THROW_IF(ret != 3, -1);
    } CATCH (ret) {
    }

    return ret;
}

int test_strcpy_value = -1;
static void *my_strcmp(void *s, void *d)
{
    dbg_str(DBG_ERROR, "run at here");
    test_strcpy_value = 128;
    dbg_str(DBG_ERROR, "test_strcpy_value:%d", test_strcpy_value);
    return NULL;
}

int test_stub_add2()
{
    char buf[20] = {0};
    stub_t stub;
    int ret;

    TRY {
        stub_add(&stub, (void*)strcmp, (void*)my_strcmp);
        strcmp(buf, "hello_world");
        THROW_IF(test_strcpy_value != 128, -1);
        stub_remove(&stub); 

        strcpy(buf, "hello_world");
        ret = strcmp(buf, "hello_world");
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
        stub_remove(&stub);  // 添加动态桩 用B替换A
        dbg_str(DBG_ERROR, "test_strcpy_value:%d", test_strcpy_value);
    }
    return ret;
}

int test_stub_add()
{
    int ret;

    TRY {
        EXEC(test_stub_add1());
        EXEC(test_stub_add2());
    } CATCH (ret) {
    }

    return ret;
}
REGISTER_TEST_CMD(test_stub_add);

int func_pre(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("func_pre, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int func(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int func2(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 8;
    printf("func2, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    printf("func_post, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int test_stub_add_hooks()
{
    stub_t *stub;
    int g = 7;

    /*
     *stub.area.stub = &stub;
     */
    stub = stub_alloc();
    dbg_str(DBG_DETAIL, "stub:%p", stub);

    stub_add_hooks(stub, (void *)func, (void *)func_pre, (void *)func2, (void *)print_outbound, 7);
    func(1, 2, 3, 4, 5, 6, &g);
    stub_remove_hooks(stub);
    func(1, 2, 3, 4, 5, 6, &g);

    return 0;
}

REGISTER_TEST_CMD(test_stub_add_hooks);

#endif
