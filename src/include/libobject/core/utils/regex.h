#ifndef __REGEX_WRAP__
#define __REGEX_WRAP__

#include <regex.h>

int
regcomp_wrap(regex_t *, const char *, int cflags);
size_t
regerror_wrap(int errcode, const regex_t *, char *, size_t errbuf_size);
int
regexec_wrap(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
void
regfree_wrap(regex_t *preg);

#endif
