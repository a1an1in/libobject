#if (defined(WINDOWS_USER_MODE))
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <windows.h>

#define FLATJMPCODE_LENGTH 5            //x86 平坦内存模式下，绝对跳转指令长度
#define FLATJMPCMD_LENGTH  1            //机械码0xe9长度
#define FLATJMPCMD         0xe9         //相应汇编的jmp指令

typedef struct stub stub_t;
typedef int (*stub_func_t)(void * p1, void * p2, void * p3, void * p4, void * p5, 
                void * p6, void * p7, void * p8, void * p9, void * p10,
                void * p11, void * p12, void * p13, void * p14, void * p15, 
                void * p16, void * p17, void * p18, void * p19, void * p20);

typedef struct stub_exec_area {
    unsigned char exec_code[80];
    stub_t *stub;
} stub_exec_area_t;

struct stub {
    stub_exec_area_t *area;
    void *reg_bp;
    void *pre;
    void *post;
    void *new_fn;
    void *fn;
    unsigned int para_count;
    int area_flag;
    unsigned char inst_backup[FLATJMPCODE_LENGTH + FLATJMPCMD_LENGTH];
};

int stub_add(stub_t *stub, void *func, void *new_fn)
{
	DWORD oldProtect =0;
	DWORD TempProtectVar = 0;
	char newCode[6] = {0};                                 //用于读取函数原有内存信息
	HANDLE hProgress = GetCurrentProcess();                //获取进程伪句柄
	int SIZE = FLATJMPCODE_LENGTH + FLATJMPCMD_LENGTH;     //须要改动的内存大小

	if (!VirtualProtect(func, SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) { //改动内存为可读写
		return -1;
	}
	if (!ReadProcessMemory(hProgress, func, newCode, SIZE, NULL)) {         //读取内存
		return -1;
	}
	memcpy((void*)stub->inst_backup, (const void*)newCode, sizeof(stub->inst_backup));                //保存被打桩函数信息
	*(BYTE*)func = FLATJMPCMD;
	*(DWORD*)((BYTE*)func + FLATJMPCMD_LENGTH) = (DWORD)new_fn - (DWORD)func - FLATJMPCODE_LENGTH;   //桩函数注入 
	VirtualProtect(func, SIZE, oldProtect, &TempProtectVar);  //恢复保护属性

	return 1;
}

int stub_remove(stub_t *stub, void *func)
{
	int ret = -1;
	DWORD TempProtectVar;                //暂时保护属性变量
	MEMORY_BASIC_INFORMATION MemInfo;    //内存分页属性信息

	VirtualQuery(func, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION));

	if (VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_EXECUTE_READWRITE, &MemInfo.Protect)) { //改动页面为可写
		memcpy((void*)func, (const void*)stub->inst_backup, sizeof(stub->inst_backup));                      //恢复代码段
		VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, MemInfo.Protect, &TempProtectVar);          //改回原属性
		ret = 1;
	}

	return ret;
}

int stub_parse_context(void *exec_code_addr, void *reg_bp, void *p1, void *p2, void *p3, void *p4)
{
    struct stub *stub;
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
        stub_remove(stub, stub->fn);
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

struct stub *stub_alloc()
{
    struct stub *stub;

    stub = (struct stub *)malloc(sizeof(struct stub));
    stub->area_flag = 0;

    return stub;
}

int stub_alloc_exec_area(stub_t *stub)
{
    DWORD oldProtect =0;
	DWORD TempProtectVar = 0;

    stub->area = (stub_exec_area_t *)stub_placeholder;
    
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    stub->area->stub = stub;
    stub->area_flag = 1;
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), oldProtect, &TempProtectVar);  //恢复保护属性

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

int stub_add_hooks(struct stub *stub, void *func, void *pre, void *new_fn, void *post, int para_count)
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
    DWORD oldProtect =0;
	DWORD TempProtectVar = 0;
    int ret;
    int call_inst_addr_offset = 49;
    int *addr = code + call_inst_addr_offset;
    int call_inst_addr_len = 4;
    struct stub_info_s *stub_info;

    stub_alloc_exec_area(stub);
    *addr = ((long long)stub_parse_context - (long long)stub->area->exec_code) - call_inst_addr_offset - call_inst_addr_len;
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(stub->area->exec_code, code, sizeof(code));
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), oldProtect, &TempProtectVar);  //恢复保护属性
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

int stub_remove_hooks(struct stub *stub, void *func)
{
    stub_free_exec_area(stub);
    stub_remove(stub, func);

    return 0;
}


int func_a_stub(void)
{
	return 300;
}

int func_a(void)
{
	return 200;
}

char *g_str = "hello shellcode world\n";
int func4()
{
    asm (
        "sub $0x30,%%rsp\n\t"
        "mov %%r9d,0x28(%%rsp)\n\t"
        "mov %%r8d,0x20(%%rsp)\n\t"
        "mov %%rdx,0x18(%%rsp)\n\t"
        "mov %%rcx,0x10(%%rsp)\n\t"
        "mov 0x10(%%rsp),%%r8d\n\t"
        "mov 0x18(%%rsp),%%r9d\n\t"
        "lea (%%rip),%%rcx\n\t"
        "mov %%rbp,%%rdx \n\t"
        "callq %1\n\t"
        :
        :"m"(g_str), "m"(memset)
    );
}

int func5()
{
    return printf("hello shellcode world\n");
}

int func6()
{
    return func5();
}

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

int test_stub_windows()
{
    struct stub *stub;
    int g = 7;

    stub = stub_alloc();
    printf("The value:%d\r\n", func_a());
	stub_add(stub, (void *)func_a, (void *)func_a_stub);
	printf("The value:%d\r\n", func_a());
	stub_remove(stub, (void *)func_a);
	printf("The value:%d\r\n", func_a());

    stub_add_hooks(stub, (void *)func, (void *)func_pre, (void *)func2, (void *)print_outbound, 7);
    func(1, 2, 3, 4, 5, 6, &g);
    stub_remove_hooks(stub, (void *)func);
    func(1, 2, 3, 4, 5, 6, &g);
    stub_free(stub);

    return 0;
}

REGISTER_TEST_CMD(test_stub_windows);
#endif
