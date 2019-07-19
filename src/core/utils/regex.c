#include <stdio.h>
#include <libobject/core/utils/regex.h>
int
regcomp_wrap(regex_t *preg, const char *pattern, int cflags)
{
    return regcomp(preg, pattern, cflags);
}

size_t
regerror_wrap(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
    return regerror(errcode, preg, errbuf, errbuf_size);
}

int
regexec_wrap(const regex_t * preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags)
{
    return regexec(preg, string, nmatch, pmatch, eflags);
}

void
regfree_wrap(regex_t *preg)
{
    return regfree(preg);
}
