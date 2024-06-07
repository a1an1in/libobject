#ifndef __STUB_H__
#define __STUB_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

#define JMP_OFFSET_LEN  5   //JMP指令的长度
/* 经过测试只带返回值的函数体和空参数的函数代码长度为14， 所以这段代码块
   替换目标函数是安全的(返回值和参数还有函数体完全为空的函数代码长度
   为10， 这种函数也不该存在而且也不符合正常的编码规范) */
#define STUB_REPLACE_CODE_SIZE 14U
#define FLATJMPCODE_LENGTH 5            //x86 平坦内存模式下，绝对跳转指令长度
#define FLATJMPCMD_LENGTH  1            //机械码0xe9长度
#define FLATJMPCMD         0xe9         //相应汇编的jmp指令

typedef struct stub_s stub_t;
typedef int (*stub_func_t)(void * p1, void * p2, void * p3, void * p4, void * p5, 
                           void * p6, void * p7, void * p8, void * p9, void * p10,
                           void * p11, void * p12, void * p13, void * p14, void * p15, 
                           void * p16, void * p17, void * p18, void * p19, void * p20);

typedef struct stub_exec_area {
    unsigned char exec_code[60];
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
    void *opaque;
#if (defined(WINDOWS_USER_MODE))
    unsigned char inst_backup[FLATJMPCODE_LENGTH + FLATJMPCMD_LENGTH];
#else
    unsigned char code_buf[STUB_REPLACE_CODE_SIZE];
#endif
};

stub_t *stub_alloc();
int stub_add(stub_t *stub, void *src, void *dst);
int stub_add_hooks(stub_t *stub, void *func, void *pre, void *new_fn, void *post, int para_count);
int stub_remove(stub_t *stub);
int stub_remove_hooks(stub_t *stub);
int stub_free(stub_t *stub);

#endif
