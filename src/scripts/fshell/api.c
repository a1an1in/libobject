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
#include "libobject/stub/stub.h"

extern FShell *g_shell;

int test_hello()
{
    printf("hello world\n");
    return 0;
}

int fsh_help()
{
    printf("fshell help\n");
}

#if (!defined(WINDOWS_USER_MODE))
#include <sys/user.h>
#include <sys/ptrace.h>
int fsh_attach(char *pid)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;   

    TRY {
        THROW_IF(pid == NULL, -1);
        dbg_str(DBG_VIP, "fsh_attach pid:%s", pid);
        tpid = atoi(pid);
        EXEC(ptrace(PTRACE_ATTACH, tpid, NULL, NULL));  
        wait(NULL);
    } CATCH (ret) { }

    return ret;
}

int fsh_dettach(char *pid)
{
    int ret;
    pid_t tpid;
    struct user_regs_struct regs;   

    TRY {
        THROW_IF(pid == NULL, -1);
        dbg_str(DBG_VIP, "fsh_dettach pid:%s", pid);
        tpid = atoi(pid);
        EXEC(ptrace(PTRACE_DETACH, tpid, NULL, NULL));
    } CATCH (ret) { }

    return ret;
}
#endif

int fsh_quit()
{
    FShell *shell = g_shell;

    shell->close_flag = 1;
    printf("fshell quit ok.\n");

    return 1; 
}

int fsh_printf(char *fmt, ...)
{
    va_list ap;

    printf("fsh_printf, fmt:%s\n", fmt);
    dbg_buf(DBG_SUC, "fmt:", fmt, strlen(fmt));
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    return 1;
}

int fsh_exec(char *string)
{
    int ret, len;

    TRY {
        THROW_IF(string == NULL, -1);
        if (string[0] == '"') {
            len = strlen(string);
            string[len - 1] = '\0';
            string = &string[1];
        }
        EXEC(system(string));
    } CATCH (ret) { }

    return ret;
}

int fsh_call(void *p1, void *p2, void *p3, void *p4, void *p5, 
             void *p6, void *p7, void *p8, void *p9, void *p10,
             void *p11, void *p12, void *p13, void *p14, void *p15, 
             void *p16, void *p17, void *p18, void *p19, void *p20)
{
    FShell *shell = g_shell;
    fshell_func_t func = NULL;
    int ret;

    TRY {
        printf("fsh call:%s\n", (char *)p1);
        EXEC(shell->get_func_addr(shell, NULL, (char *)p1, &func));
        EXEC(func(p2, p3, p3, p5, p6, p7, p8, p9, p10, p11, 
                  p12, p13, p14, p15, p16, p17, p18, p19, p20, 0));
    } CATCH (ret) { }

    return ret;
}

int fsh_test_add_v1(int a, int b)
{
    printf("fsh add, a:%d, b:%d, count:%d \n", a, b, a + b);
    return a + b;
}

int fsh_test_add_v2(int a, int b, int *r)
{
    printf("fsh add, a:%d, b:%d, count:%d \n", a, b, a + b);
    *r = a + b;
    return 1;
}

int fsh_alloc_stub(stub_t **stub)
{
    stub_t * s;
    int ret;

    TRY {
        THROW_IF(stub == NULL, -1);
        s = stub_alloc();
        THROW_IF(s == NULL, -1);
        *stub = s;
        dbg_str(DBG_SUC, "fsh_alloc_stub stub:%p", s);
    } CATCH (ret) {}

    return ret;
}

int fsh_free_stub(stub_t *stub)
{
    dbg_str(DBG_SUC, "fsh_free_stub stub:%p", stub);
    return stub_free(stub);
}

int fsh_add_stub_hooks(stub_t *stub, void *shell, void *func, void *pre, void *new_fn, void *post, int para_count)
{
    return stub_add_hooks(stub, func, pre, new_fn, post, para_count);
}

int fsh_remove_stub_hooks(stub_t *stub)
{
    return stub_remove_hooks(stub);
}