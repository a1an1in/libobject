#include <stdio.h>
#include <stdlib.h>
#if (!defined(WINDOWS_USER_MODE))
#include <dlfcn.h>
#endif
#include <string.h>

void *testlib_dlopen(char *name, int flag);

static char debug_info[1024] = "hello world";

int testlib_get_debug_info_address()
{
    printf("debug_info_address:%p\n", debug_info);
    return debug_info;
}

int testlib_print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 123;
    printf("func_post, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

int testlib_hello_world()
{
    char tmp[4] = {0};
    printf("hello world, sprintf:%p\n", sprintf);
    sprintf(tmp, "%d", 1);
    return 0xadad;
}

int testlib_hello_world_without_pointer_pars(int a, int b, int c, int d, int e, int f, long g, long h)
{
    printf("hello world, a:%x, b:%x, c:%x, d:%x, e:%x, f:%x, g:%lx, h:%lx\n", a, b, c, d, e, f, g, h);
    return 0xadad;
}

int testlib_hello_world_with_pointer_pars(char *par1, char *par2)
{
    printf("hello world, par1:%s, par2:%s\n", par1, par2);
    return 0xadad;
}

int testlib_hello_world_with_pointer_pars2(int par1, char *par2)
{
    printf("testlib_hello_world_with_pointer_pars2, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadae;
}

int testlib_hello_world_with_pointer_pars3(int par1, char *par2)
{
    printf("testlib_hello_world_with_pointer_pars3, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadaf;
}

void *testlib_malloc(int size)
{
    void *addr;
    addr = malloc(size);

    return addr;
}

int testlib_free(void *addr)
{
    free(addr);

    return 0;
}


void *testlib_dlopen(char *name, int flag)
{
    void *handle = NULL;
    int i;

    
#if (!defined(WINDOWS_USER_MODE))
    handle = dlopen(name, flag);
    if (handle == NULL) {
        printf("dlopen error %s\n", dlerror());
    } else {
        printf("testlib_dlopen %s, handle:%p\n", name, handle);
    }
#endif

    return handle;
}

int testlib_dlclose (void *handle)
{
    printf( "testlib_dlclose handle:%p\n", handle);
#if (!defined(WINDOWS_USER_MODE))
    return dlclose(handle);
#endif
}

 char *testlib_dlerror()
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

void *testlib_thread_callback(void *para)
{
    int i = 0;

#if (!defined(WINDOWS_USER_MODE))
    printf("child thread tid: %u\n", gettid());
    printf("dlopen function addr: %p\n", dlopen);
#endif
    printf("testlib_hello_world addr: %p\n", testlib_hello_world);
    printf("testlib_free function addr: %p\n", testlib_free);
    printf("testlib_malloc function addr: %p\n", testlib_malloc);
    printf("testlib_dlopen function addr: %p\n", testlib_dlopen);

    printf("sprintf function addr: %p\n", sprintf);
    
    printf("testlib_hello_world_with_pointer_pars function addr: %p\n", testlib_hello_world_with_pointer_pars);
    printf("testlib_hello_world_with_pointer_pars2 function addr: %p\n", testlib_hello_world_with_pointer_pars2);
    printf("testlib_hello_world_with_pointer_pars3 function addr: %p\n", testlib_hello_world_with_pointer_pars3);

    // testlib_dlopen("abc", 0);
	while (1) {
        i++;
        sleep(2);
        printf("test thread is running, loop index:%d\n", i);
        testlib_hello_world_with_pointer_pars2(1, "abc");
	}

    return NULL;
}

void *testlib_get_func_addr_by_name(char *name, char *lib_name)
{
    void *handle = NULL;
    void *addr = NULL;

    /* open the needed object */
    handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return NULL;
    }

    /* find the address of function and data objects */
    addr = dlsym(handle, name);
    printf("%s func addr=%p\n", name, addr);
    dlclose(handle);

    return addr;
}