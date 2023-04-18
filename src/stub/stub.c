/**
 * @file Stub.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-08-29
 */

#include "libobject/stub/stub.h"

int stub_placeholder()
{
    asm (
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
        :
        :
    );
    
    return 0;
}

stub_t *stub_alloc()
{
    stub_t *stub;
    int ret;

    TRY {
        stub = (stub_t *)malloc(sizeof(stub_t));
        stub->area_flag = 0;
        EXEC(stub_alloc_exec_area(stub));
    } CATCH (ret) {
        stub = NULL;
    }

    return stub;
}

int stub_free(stub_t *stub)
{
    if (stub->area_flag == 1) {
        stub_free_exec_area(stub);
    }
    free(stub);

    return 1;
}