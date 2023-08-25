#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

void *my_dlopen(char *name, int flag);

static char debug_info[1024] = "hello world";

int test_lib_get_debug_info_address()
{
    printf("debug_info_address:%p\n", debug_info);
    return debug_info;
}

int test_lib_print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 123;
    printf("func_post, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int test_lib_hello_world()
{
    printf("hello world\n");
    return 0xadad;
}

int test_lib_hello_world_without_pointer_pars(int a, int b, int c, int d, int e, int f, long g, long h)
{
    printf("hello world, a:%x, b:%x, c:%x, d:%x, e:%x, f:%x, g:%lx, h:%lx\n", a, b, c, d, e, f, g, h);
    return 0xadad;
}

int test_lib_hello_world_with_pointer_pars(char *par1, char *par2)
{
    printf("hello world, par1:%s, par2:%s\n", par1, par2);
    return 0xadad;
}

int test_lib_hello_world_with_pointer_pars2(int par1, char *par2)
{
    printf("test_lib_hello_world_with_pointer_pars2, par1:%x, par2:%s\n", par1, par2);

    return 0xadad;
}

void *my_malloc(int size)
{
    void *addr;
    addr = malloc(size);
    // sprintf(debug_info, "test my_dlopen\n");
    printf("my_malloc addr:%p\n", addr);

    return addr;
}

int my_free(void *addr)
{
    printf("my_free addr:%p\n", addr);
    free(addr);

    return 0;
}

void *my_dlopen(char *name, int flag)
{
    void *handle = NULL;
    int i;

    printf("my_dlopen name:%s, strlen:%ld, flag:%x\n", name, strlen(name), flag);
    
    handle = dlopen(name, flag);
    if (handle == NULL) {
        printf("dlopen error %s\n", dlerror());
    } else {
        printf("my_dlopen handle:%p\n", handle);
    }

    return handle;
}

int my_dlclose (void *handle)
{
    printf( "my_dlclose handle:%p\n", handle);
    return dlclose(handle);
}

 char *my_dlerror()
 {
    char *addr;

    addr = dlerror();
    printf("addr :%s\n", addr);
    perror("perror:");
    printf("debug_info :%s\n", debug_info);

    return addr;
 }

void *subprocess_callback(void *para)
{
    int i = 0, sum = 0;

    printf("child process tid: %u\n", gettid());
    printf("test_lib_hello_world addr: %p\n", test_lib_hello_world);
    printf("my_free function addr: %p\n", my_free);
    printf("my_malloc function addr: %p\n", my_malloc);
    printf("my_dlopen function addr: %p\n", my_dlopen);
    printf("dlopen function addr: %p\n", dlopen);
    
    printf("test_lib_hello_world_with_pointer_pars function addr: %p\n", test_lib_hello_world_with_pointer_pars);

    // my_dlopen("abc", 0);
	while (1) {
        i++;
        sum +=i;
        sleep(100);
        printf("subprocess is running ...\n");
        test_lib_hello_world_with_pointer_pars2(1, "abc");
	}

    return NULL;
}