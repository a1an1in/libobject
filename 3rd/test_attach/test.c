#include <stdio.h>
#include <unistd.h> //for sleep
#include <stdlib.h> //for exit
#include <pthread.h>//for pthread
#include <errno.h>  //for errno
#include <sys/syscall.h> //for gettid
#define gettid() syscall(__NR_gettid)
 
extern void *func(void *para);

int main(){ 
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, func, NULL);
    if(ret != 0)
    {   
        exit(errno);
    }   
    printf("\nparent process pid: %u\n", getpid());
 
    pthread_join(tid, NULL);
    return 0;
}