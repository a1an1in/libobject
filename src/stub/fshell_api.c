/**
 * @file api.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-04-03
 */

#include <stdlib.h>
#include <signal.h>
#include <libobject/core/utils/string.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/net/bus/bus.h>
#include "libobject/stub/stub.h"

/* test funcs */
int test_print_inbound(int a, int b, int c, int d, int e, int f, int *g)
{
    dbg_str(DBG_INFO, "inbound func of test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d", a, b, c, d, e, f, *g);
    return 1;
}

int test_func(int a, int b, int c, int d, int e, int f, int *g)
{
    dbg_str(DBG_INFO, "original test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d", a, b, c, d, e, f, *g);
    return 1;
}

int test_target_func(int a, int b, int c, int d, int e, int f, int *g)
{
    *g = 8;
    dbg_str(DBG_INFO, "target test_func which replaced the original test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d", a, b, c, d, e, f, *g);
    return 1;
}

int test_print_outbound(int a, int b, int c, int d, int e, int f, int *g)
{
    dbg_str(DBG_INFO, "outbound func of test_func, a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d", a, b, c, d, e, f, *g);
    return 1;
}

int fsh_add_stub_hooks(FShell *shell, stub_t *stub, char *func_name, char *pre_name, 
                       char *new_fn_name, char *post_name, int para_count)
{
    void *func, *pre, *new_fn, *post;
    int g = 7;
    int ret;

    TRY {
        THROW_IF(func_name == NULL, -1);

        dbg_str(DBG_INFO, "fsh_add_stub_hooks shell:%p, para count:%d", shell, para_count);
        EXEC(shell->get_func_addr(shell, NULL, func_name, &func));
        THROW_IF(func == NULL, -1);

        if (pre_name != NULL) {
            EXEC(shell->get_func_addr(shell, NULL, pre_name, &pre));
            THROW_IF(pre == NULL, -1);
        }

        if (new_fn_name != NULL) {
            EXEC(shell->get_func_addr(shell, NULL, new_fn_name, &new_fn));
            THROW_IF(new_fn == NULL, -1);
        }

        if (post_name != NULL) {
            EXEC(shell->get_func_addr(shell, NULL, post_name, &post));
            THROW_IF(post == NULL, -1);
        }
        dbg_str(DBG_INFO, "stub_add_hooks func:%p, pre:%p, new_fn:%p, post:%p, para_count:%d",
                func, pre, new_fn, post, para_count);
        
        EXEC(stub_add_hooks(stub, func, pre, new_fn, post, para_count));
    } CATCH (ret) {}

    return ret;
}

int fsh_remove_stub_hooks(FShell *shell, stub_t *stub)
{
    return stub_remove_hooks(stub);
}