#include <stdio.h>
#include <stdlib.h>

int test_lib2_hello_world()
{
    printf("test lib2 hello world\n");
    return 0xadad;
}

int test_lib2_hello_world_with_pars(int par1, char *par2)
{
    printf("test_lib2_hello_world_with_pars, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadad;
}