#include <stdio.h>
#include <string.h>

static int test_count;
int test_hello_world()
{
    test_count++;
     printf("hello world, test_count:%d\n", test_count);
    return test_count;
}