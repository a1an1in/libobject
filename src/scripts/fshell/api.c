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

int fsh_hello()
{
    printf("hello world\n");
    return 0;
}

int fsh_add(int a, int b)
{
    dbg_str(DBG_DETAIL, "fsh add, a:%d, b:%d, count:%d", a, b, a + b);
    return a + b;
}

int fsh_exec(char *string)
{
    printf("fsh_exec:%s\n", string);
    return system(string);
}
