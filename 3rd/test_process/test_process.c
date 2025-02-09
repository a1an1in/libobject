#include <stdio.h>
#include <unistd.h> //for sleep
#include <stdlib.h> //for exit
#include <pthread.h>//for pthread
#include <errno.h>  //for errno
#include <dlfcn.h>
#include <sys/syscall.h> //for gettid

#define gettid() syscall(__NR_gettid)
extern int attacher_test_with_pointer_arg(int par1, char *par2);
extern int attacher_test_with_pointer_arg_prehook(int par1, char *par2);
extern attacher_test_with_pointer_arg_afterhook(int par1, char *par2);

int test_with_mixed_type_pars(int par1, char *par2)
{
    printf("test_with_mixed_type_pars, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadae;
}

void *test_thread_callback(void *para)
{
    int i = 0;

#if (!defined(WINDOWS_USER_MODE))
    printf("child thread tid: %u\n", gettid());
    printf("dlopen function addr: %p\n", dlopen);
#endif

    printf("sprintf function addr: %p\n", sprintf);
    printf("test_with_mixed_type_pars function addr: %p\n", test_with_mixed_type_pars);
    printf("attacher_test_with_pointer_arg function addr: %p\n", attacher_test_with_pointer_arg);

    // attacher_dlopen("abc", 0);
	while (1) {
        i++;
        sleep(2);
        printf("test thread is running, loop index:%d\n", i);
        test_with_mixed_type_pars(1, "abc");
	}

    return NULL;
}

int main()
{ 
    pthread_t tid;
    void *handle;

    int ret = pthread_create(&tid, NULL, test_thread_callback, NULL);
    if (ret != 0) {   
        exit(errno);
    }   
    printf("parent process pid: %u\n", getpid());
 
    pthread_join(tid, NULL);

    return 0;
}