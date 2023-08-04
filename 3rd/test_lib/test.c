#include <stdio.h>

int test_lib_print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 123;
    printf("func_post, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d\n", a, b, c, d, e, f, *g);
    return 1;
}

void attach_test_func()
{
    int i = 0,sum = 0;
	while(1){
		i++;
		sum +=i;
		sleep(1);
	}
}
 
int test_lib_hello_world()
{
    return printf("hello world\n");
}

void *func(void *para)
{
    printf("test_lib_hello_world addr: %p\n", test_lib_hello_world);
    printf("child process tid: %u\n", gettid());
	attach_test_func();
    return NULL;
}