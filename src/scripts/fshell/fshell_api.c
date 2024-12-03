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

int fsh_hello(FShell *shell)
{
    printf("fshell hello world\n");
    return 0;
}

int fsh_help(FShell *shell)
{
    printf("fshell help\n");
}

int fsh_quit(FShell *shell)
{
    shell->close_flag = 1;
    printf("fshell quit ok.\n");

    return 1; 
}

int fsh_printf(FShell *shell, char *fmt, ...)
{
    va_list ap;

    printf("fsh_printf, fmt:%s\n", fmt);
    dbg_buf(DBG_VIP, "fmt:", fmt, strlen(fmt));
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    return 1;
}

int fsh_test_add_v1(FShell *shell, int a, int b)
{
    dbg_str(DBG_INFO, "fsh add, a:%d, b:%d, count:%d", a, b, a + b);
    return a + b;
}

int fsh_test_add_v2(FShell *shell, int a, int b, int *r)
{
    dbg_str(DBG_INFO, "fsh add, a:%d, b:%d, count:%d", a, b, a + b);
    *r = a + b;
    return 1;
}