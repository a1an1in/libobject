#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libobject/core/utils/alloc/allocator.h>

#if (!defined(ANDROID_USER_MODE))
#include <execinfo.h>

#define MAX_BACKTRACE_SIZE 100
int print_backtrace(void)
{
    int j, nptrs;
    void *buffer[MAX_BACKTRACE_SIZE];
    char **strings;

    nptrs = backtrace(buffer, MAX_BACKTRACE_SIZE);
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        return -1;
    }

    for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);

    return 0;
}

int get_backtrace(char *bt, int max_len)
{
    int j, nptrs;
    void *buffer[MAX_BACKTRACE_SIZE];
    char **strings;

    nptrs = backtrace(buffer, MAX_BACKTRACE_SIZE);
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        return -1;
    }

    for (j = 0; j < nptrs; j++)
        snprintf(bt + strlen(bt), max_len - strlen(bt), "%s\n", strings[j]);

    free(strings);

    return 0;
}
#endif

