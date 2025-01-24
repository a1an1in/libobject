#include <stdio.h>
#include <stdlib.h>

int testlib_test()
{
    printf("test lib2 hello world\n");
    return 0xadad;
}

int testlib_test_with_pars(int par1, char *par2)
{
    printf("testlib_test_with_pars, par1:%x, par2:%s\n", par1, par2);
    
    return 0xadad;
}