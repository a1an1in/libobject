#include <stdio.h>
#include <unistd.h> //for sleep
#include <stdlib.h> //for exit
#include <pthread.h>//for pthread
#include <errno.h>  //for errno
#include <dlfcn.h>
#include <sys/syscall.h> //for gettid

#define gettid() syscall(__NR_gettid)
extern void *testlib_thread_callback(void *para);

int main()
{ 
    pthread_t tid;
    void *handle;

    int ret = pthread_create(&tid, NULL, testlib_thread_callback, NULL);
    if (ret != 0) {   
        exit(errno);
    }   
    printf("parent process pid: %u\n", getpid());
 
    pthread_join(tid, NULL);

    return 0;
}