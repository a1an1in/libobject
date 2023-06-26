#include <stdio.h>
#include <unistd.h> //for sleep
#include <stdlib.h> //for exit
#include <pthread.h>//for pthread
#include <errno.h>  //for errno
#include <sys/syscall.h> //for gettid
#define gettid() syscall(__NR_gettid)
 
void func_test(){
    int i = 0,sum = 0;
	while(1){
		i++;
		sum +=i;
		sleep(1);
	}
}
 
void *func(void *para){
    printf("child process tid: %u\n", gettid());
	func_test();
    return NULL;
}
 
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