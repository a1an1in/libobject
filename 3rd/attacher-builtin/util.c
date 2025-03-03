#include <stdio.h>
#include <stdlib.h>
#if (!defined(WINDOWS_USER_MODE))
#include <dlfcn.h>
#endif
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static char debug_info[1024] = "hello world";

int attacher_get_debug_info_address()
{
    printf("debug_info_address:%p\n", debug_info);
    return debug_info;
}

int attacher_print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 123;
    printf("func_post, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int attacher_test_without_arg()
{
    char tmp[4] = {0};
    printf("hello world, sprintf:%p\n", sprintf);
    sprintf(tmp, "%d", 1);
    return 0xadad;
}

int attacher_test_without_pointer_arg(int a, int b, int c, int d, int e, int f, long g, long h)
{
    printf("hello world, a:%x, b:%x, c:%x, d:%x, e:%x, f:%x, g:%lx, h:%lx\n", a, b, c, d, e, f, g, h);
    return 0xadad;
}

int attacher_test_with_pointer_arg(int par1, char *par2)
{
    printf("attacher_test_with_pointer_arg, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadae;
}

int attacher_test2_with_pointer_arg(int par1, char *par2)
{
    printf("attacher_test2_with_pointer_arg, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadaf;
}

int attacher_test_with_pointer_arg_prehook(int par1, char *par2)
{
    printf("attacher_test_with_pointer_arg_prehook, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadae;
}

int attacher_test_with_pointer_arg_posthook(int par1, char *par2)
{
    printf("attacher_test_with_pointer_arg_posthook, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadae;
}

void *attacher_malloc(int size)
{
    void *addr;
    addr = malloc(size);

    return addr;
}

int attacher_free(void *addr)
{
    free(addr);

    return 0;
}

void *attacher_dlopen(char *name, int flag)
{
    void *handle = NULL;
    int i;

#if (!defined(WINDOWS_USER_MODE))
    handle = dlopen(name, flag);
    if (handle == NULL) {
        printf("dlopen error %s\n", dlerror());
    } else {
        printf("attacher_dlopen %s, handle:%p\n", name, handle);
    }
    fflush(stdout); 
#endif

    return handle;
}

int attacher_dlclose (void *handle)
{
    printf( "attacher_dlclose handle:%p\n", handle);
#if (!defined(WINDOWS_USER_MODE))
    return dlclose(handle);
#endif
}

 char *attacher_dlerror()
 {
    char *addr;

#if (!defined(WINDOWS_USER_MODE))
    addr = dlerror();
    printf("addr :%s\n", addr);
    perror("perror:");
    printf("debug_info :%s\n", debug_info);
#endif
    return addr;
 }

/* 如果要获取进程自带函数或者运行后load且attacher没有的库里面的地址， 需要调用该函数获取 */
void *attacher_get_func_addr_by_name(char *name, char *lib_name)
{
    void *handle = NULL;
    void *addr = NULL;

#if (!defined(WINDOWS_USER_MODE))
    /* open the needed object */
    handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return NULL;
    }

    /* find the address of function and data objects */
    addr = dlsym(handle, name);
    printf("attacher_get_func_addr_by_name, func name:%s, addr:%p\n", name, addr);
    dlclose(handle);
#endif

    return addr;
}

int attacher_redirect_stdout_to_file(const char *filename) 
{
    FILE *f = NULL;
    int file_fd;

    // 打开文件以写入。如果文件不存在，将会被创建。
    f = fopen(filename, "w");
    if (f == NULL) {
        perror("Failed to open file");
        return -1;
    }
    file_fd = fileno(f);
 
    // 使用dup2将文件描述符复制到标准输出（文件描述符1）
    if (dup2(file_fd, STDOUT_FILENO) == -1) {
        perror("Failed to redirect stdout");
        close(file_fd); // 关闭打开的文件描述符
        return -1;
    }

    printf("attacher_redirect_stdout_to_file, filename:%s\n", filename);
    fflush(stdout); 
 
    // 关闭原始的文件描述符，因为我们不再需要它
    close(file_fd);

    return 1;
}

//将标准输出设置为行缓冲模式（_IOLBF），此时每次输出遇到换行符 \n 时自动刷新缓冲区‌
int attacher_set_stdout_rbf_mode()
{
    setvbuf(stdout, NULL, _IOLBF, 0); //设置为行缓冲模式

    return 1;
}