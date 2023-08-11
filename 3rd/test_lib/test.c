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
	while(1) {
		i++;
		sum +=i;
		sleep(1);
	}
}
 
int test_lib_hello_world()
{
    printf("hello world\n");
    return 0xadad;
}

int test_lib_hello_world_with_pars(int par1, int value)
{
    printf("hello world, par1:%x, par2:%x\n", par1, value);
    return 0xadad;
}


void *func(void *para)
{
    printf("test_lib_hello_world addr: %p\n", test_lib_hello_world);
    printf("test_lib_hello_world_with_pars addr: %p\n", test_lib_hello_world_with_pars);
    printf("child process tid: %u\n", gettid());
	attach_test_func();
    return NULL;
}