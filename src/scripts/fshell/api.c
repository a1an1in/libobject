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
#include <libobject/scripts/FShell.h>

FShell *g_shell;

int fsh_help()
{
    printf("fshell help\n");
}

int fsh_attach()
{
    printf("fshell attach\n");
}

int fsh_dettach()
{
    printf("fshell dettach\n");
}

int fsh_quit()
{
    FShell *shell = g_shell;

    printf("fshell quit\n");
    shell->close_flag = 1;

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

int fsh_add(int a, int b)
{
    printf("fsh add, a:%d, b:%d, count:%d\n", a, b, a + b);
    return a + b;
}

int fsh_exec(char *string)
{
    printf("fsh_exec:%s\n", string);
    return system(string);
}

int test_hello()
{
    printf("hello world\n");
    return 0;
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
    } CATCH (ret) {
    }

    return ret;
}