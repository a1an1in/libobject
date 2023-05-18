#define WINDOWS_USER_MODE
#if (defined(WINDOWS_USER_MODE))
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <windows.h>
#include <libobject/stub/stub.h>

int stub_add(stub_t *stub, void *func, void *new_fn)
{
	DWORD oldProtect =0;
	DWORD TempProtectVar = 0;
	char newCode[6] = {0};                                 //用于读取函数原有内存信息
	HANDLE hProgress = GetCurrentProcess();                //获取进程伪句柄
	int SIZE = FLATJMPCODE_LENGTH + FLATJMPCMD_LENGTH;     //须要改动的内存大小
    int ret;

    TRY {
        if (!VirtualProtect(func, SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) { //改动内存为可读写
            THROW(-1);
        }
        if (!ReadProcessMemory(hProgress, func, newCode, SIZE, NULL)) {         //读取内存
            THROW(-1);
        }
        memcpy((void*)stub->inst_backup, (const void*)newCode, sizeof(stub->inst_backup));                //保存被打桩函数信息
        *(BYTE*)func = FLATJMPCMD;
        *(DWORD*)((BYTE*)func + FLATJMPCMD_LENGTH) = (DWORD)new_fn - (DWORD)func - FLATJMPCODE_LENGTH;   //桩函数注入 
        VirtualProtect(func, SIZE, oldProtect, &TempProtectVar);  //恢复保护属性
        stub->fn = func;
        stub->new_fn = new_fn;
        THROW_IF(((((char *)func)[0] & 0xff)  != FLATJMPCMD), -1);
    } CATCH (ret) {
    }

	return ret;
}

int stub_remove(stub_t *stub)
{
    DWORD oldProtect =0;
	DWORD TempProtectVar = 0;
	char newCode[6] = {0};                                 //用于读取函数原有内存信息
	HANDLE hProgress = GetCurrentProcess();                //获取进程伪句柄
	int SIZE = FLATJMPCODE_LENGTH + FLATJMPCMD_LENGTH;     //须要改动的内存大小
    int ret;

    TRY {
        if (!VirtualProtect(stub->fn, SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) { //改动内存为可读写
            THROW(-1);
        }
        if (!WriteProcessMemory (hProgress, stub->fn, (const void*)stub->inst_backup, SIZE, NULL)) {         //读取内存
            THROW(-1);
        }
        VirtualProtect(stub->fn, SIZE, oldProtect, &TempProtectVar);  //恢复保护属性
        THROW_IF((((char *)stub->fn)[0] != ((char *)stub->inst_backup)[0]), -1);
    } CATCH (ret) { }

    return ret;
}

/* windows 栈帧和linux有点不同， parse 函数传递了构建函数的rbp和4个参数， linux这里只传递了rsp, 然后取解析函数参数
 * stub_parse_context不会直接调用，而且即使都传递rsp取自己解析参数，两个平台的解析方法也有点不同
 * 1. exec_code_addr 到stub_exec_area_t的偏移不同。
 * 2. 函数参数保存在寄存器中的个数不同， windows大于4个保存在栈里面， linux大于6个。
 * 3. windows callq 会把bsp和下一个执行地址放入栈中， 还有会在栈中预留0x20大小的空间，
 *    所以给stub_parse_context传递的rsp要提前预留0x30的大小。
 */
int stub_parse_context(void *exec_code_addr, void *rsp)
{
    stub_t *stub;
    stub_exec_area_t *area;
    stub_func_t func;
    int i = 0, j = 0;
    int ret;
    void *p[20] = {0};

    area = exec_code_addr - 34;
    void **par_addr = (void **)(rsp + 6 * sizeof(void *));
    stub = area->stub;

    dbg_str(DBG_DETAIL,"stub_parse_context, area:%p par_addr:%p, par_count:%d", area, par_addr, stub->para_count);
    
    while(i < stub->para_count) {
        /* 
         * 如果参数大于6个， 需要跳过rsp里面存放的第6和7个数据，
         * 里面存放的是bsp和返回地址.
         **/
        if ((j >= 4) && (j <= 9)) {
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
        ret = func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
                   p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
    } else {
        stub_remove(stub);
        func = (stub_func_t)stub->fn;
        ret = func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
                   p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
        stub_add(stub, stub->fn, stub->new_fn);
    }
    
    if (stub->post != NULL) {
        func = (stub_func_t)stub->post;
        func(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], 
             p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
    }

    return ret;
}

int stub_config_exec_area(stub_t *stub)
{
    DWORD oldProtect =0;
	DWORD TempProtectVar = 0;
    
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    stub->area->stub = stub;
    printf("stub area addr:%p\n", stub->area);
    stub->area_flag = 1;
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), oldProtect, &TempProtectVar);  //恢复保护属性

    return 1;
}

int stub_add_hooks(stub_t *stub, void *func, void *pre, void *new_fn, void *post, int para_count)
{
    /*
     * Windows 64位下函数调用约定变为了快速调用约定，前4个参数采用rcx、rdx、r8、r9传递，
     * 多余的参数从右向左依次使用堆栈传递。
     */
    unsigned char code[60] = {
        0x55,                                       //push   %rbp
        0x48, 0x89, 0xe5,                           //mov    %rsp,%rbp
        0x48, 0x83, 0xec, 0x50,                     //sub    $0x50,%rsp
        0x4c, 0x89, 0x4d, 0xf8,                     //mov    %r9,-0x8(%rbp)
        0x4c, 0x89, 0x45, 0xf0,                     //mov    %r8,-0x10(%rbp)
        0x48, 0x89, 0x55, 0xe8,                     //mov    %rdx,-0x18(%rbp)
        0x48, 0x89, 0x4d, 0xe0,                     //mov    %rcx,-0x20(%rbp)
        0x48, 0x89, 0xe2,	                        //mov    %rsp,%rdx
        0x48, 0x8d, 0x0d, 0x00, 0x00, 0x00, 0x00,	//lea    0x0(%rip),%rcx  # 0x401a99 <func4+35>
        0xe8, 0x00, 0x00, 0x00, 0x00,               //callq  0x401a8a <func5>
        0x48, 0x83, 0xc4, 0x50,                     //add    $0x50,%rsp
        0x5d,                                       //pop    %rbp
        0xc3,                                       //c3	 retq
    };
    DWORD oldProtect = 0;
	DWORD TempProtectVar = 0;
    int call_inst_addr_offset = 35;
    int *addr = code + call_inst_addr_offset;
    int call_inst_addr_len = 4;
    struct stub_info_s *stub_info;
    int ret;

    *addr = ((long long)stub_parse_context - (long long)stub->area->exec_code) - call_inst_addr_offset - call_inst_addr_len;
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(stub->area->exec_code, code, sizeof(code));
    VirtualProtect(stub->area, sizeof(stub_exec_area_t), oldProtect, &TempProtectVar);  //恢复保护属性
    printf("exec code addrs:%p\n", stub->area->exec_code);
    printf("offset:%x\n", *addr);

    stub_add(stub, func, stub->area->exec_code);
    stub->pre = pre;
    stub->new_fn = new_fn;
    stub->fn = func;
    stub->post = post;
    stub->para_count = para_count;
    printf("pre:%p, func:%p, post:%p, stubed_func:%p\n", stub->pre, stub->new_fn, stub->post, stub->fn);

    return 0;
}

char *g_str = "hello shellcode world\n";
int func4()
{
    asm (
        "sub $0x20,%%rsp\n\t"
        "mov %%r9,-0x8(%%rbp)\n\t"
        "mov %%r8,-0x10(%%rbp)\n\t"
        "mov %%rdx,-0x18(%%rbp)\n\t"
        "mov %%rcx,-0x20(%%rbp)\n\t"
        "mov %%rsp, %%rdx\n\t"
        "lea 0x0(%%rip),%%rcx\n\t"
        "callq %1\n\t"
        "add $0x20,%%rsp\n\t"
        :
        :"m"(g_str), "m"(memset)
    );
}

#endif
