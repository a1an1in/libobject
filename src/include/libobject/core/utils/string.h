#ifndef __CSTRING_H__
#define __CSTRING_H__

#include <libobject/basic_types.h>

long long str_hex_to_integer(char *str);
char *str_trim(char *str);
int strrcnt(char *s, char *end, char c);
int str_split(char *str, char *delim, char **out, int *cnt);
int str_remove_spaces_around_comma(char *str);

#endif

