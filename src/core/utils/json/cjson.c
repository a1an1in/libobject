/*
 *warnning:
 *this lib is totally porting from cJson. Inorder key keep c style of my libs,
 I transfered previous code style in cJson.
 * */

/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cjson_t */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <libobject/core/utils/json/cjson.h>

/* Determine the number of bits that an integer has using the preprocessor */
#if INT_MAX == 32767
    /* 16 bits */
    #define INTEGER_SIZE 0x0010
#elif INT_MAX == 2147483647
    /* 32 bits */
    #define INTEGER_SIZE 0x0100
#elif INT_MAX == 9223372036854775807
    /* 64 bits */
    #define INTEGER_SIZE 0x1000
#else
    #error "Failed to determine the size of an integer"
#endif

typedef struct
{
    char *buffer;
    int length;
    int offset;
}printbuffer;

static const char *global_ep;
static void *(*cjson_malloc)(size_t sz) = malloc;
static void (*cjson_free)(void *ptr) = free;

/* Predeclare these prototypes. */
static const char *parse_value(cjson_t *item, const char *value, const char **ep);
static char *print_value(const cjson_t *item, int depth, int fmt, printbuffer *p);
static const char *parse_array(cjson_t *item, const char *value, const char **ep);
static char *print_array(const cjson_t *item, int depth, int fmt, printbuffer *p);
static const char *parse_object(cjson_t *item, const char *value, const char **ep);
static char *print_object(const cjson_t *item, int depth, int fmt, printbuffer *p);

const char *cjson_get_error_ptr(void)
{
    return global_ep;
}

/* case insensitive strcmp */
static int cjson_strcasecmp(const char *s1, const char *s2)
{
    if (!s1) {
        return (s1 == s2) ? 0 : 1; /* both NULL? */
    }
    if (!s2) {
        return 1;
    }
    for(; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2)
    {
        if (*s1 == 0) {
            return 0;
        }
    }

    return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

static char* cjson_strdup(const char* str)
{
    size_t len;
    char* copy;

    len = strlen(str) + 1;
    if (!(copy = (char*)cjson_malloc(len))) {
        return 0;
    }
    memcpy(copy, str, len);

    return copy;
}

void cjson_init_hooks(cjson_Hooks* hooks)
{
    if (!hooks) {
        /* Reset hooks */
        cjson_malloc = malloc;
        cjson_free = free;
        return;
    }

    cjson_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
    cjson_free = (hooks->free_fn) ? hooks->free_fn : free;
}

/* Internal constructor. */
static cjson_t *cjson_new_item(void)
{
    cjson_t* node = (cjson_t*)cjson_malloc(sizeof(cjson_t));
    if (node) {
        memset(node, 0, sizeof(cjson_t));
    }

    return node;
}

/* Delete a cjson_t structure. */
void cjson_delete(cjson_t *c)
{
    cjson_t *next;
    while (c) {
        next = c->next;
        if (!(c->type & CJSON_ISREFERENCE) && c->child) {
            cjson_delete(c->child);
        }
        if (!(c->type & CJSON_ISREFERENCE) && c->valuestring) {
            cjson_free(c->valuestring);
        }
        if (!(c->type & CJSON_STRINGISCONST) && c->string) {
            cjson_free(c->string);
        }
        cjson_free(c);
        c = next;
    }
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(cjson_t *item, const char *num)
{
    double n = 0;
    double sign = 1;
    double scale = 0;
    int subscale = 0;
    int signsubscale = 1;

    /* Has sign? */
    if (*num == '-') {
        sign = -1;
        num++;
    }
    /* is zero */
    if (*num == '0') {
        num++;
    }
    /* Number? */
    if ((*num >= '1') && (*num <= '9')) {
        do {
            n = (n * 10.0) + (*num++ - '0');
        }
        while ((*num >= '0') && (*num<='9'));
    }
    /* Fractional part? */
    if ((*num == '.') && (num[1] >= '0') && (num[1] <= '9')) {
        num++;
        do {
            n = (n  *10.0) + (*num++ - '0');
            scale--;
        } while ((*num >= '0') && (*num <= '9'));
    }
    /* Exponent? */
    if ((*num == 'e') || (*num == 'E')) {
        num++;
        /* With sign? */
        if (*num == '+') {
            num++;
        } else if (*num == '-') {
            signsubscale = -1;
            num++;
        }
        /* Number? */
        while ((*num>='0') && (*num<='9')) {
            subscale = (subscale * 10) + (*num++ - '0');
        }
    }

    /* number = +/- number.fraction * 10^+/- exponent */
    n = sign * n * pow(10.0, (scale + subscale * signsubscale));

    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = CJSON_NUMBER;

    return num;
}

/* calculate the next largest power of 2 */
static int pow2gt (int x)
{
    --x;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
#if INTEGER_SIZE & 0x1110 /* at least 16 bit */
    x |= x >> 8;
#endif
#if INTEGER_SIZE & 0x1100 /* at least 32 bit */
    x |= x >> 16;
#endif
#if INT_SIZE & 0x1000 /* 64 bit */
    x |= x >> 32;
#endif

    return x + 1;
}

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static char* ensure(printbuffer *p, int needed)
{
    char *newbuffer;
    int newsize;
    if (!p || !p->buffer) {
        return 0;
    }
    needed += p->offset;
    if (needed <= p->length) {
        return p->buffer + p->offset;
    }

    newsize = pow2gt(needed);
    newbuffer = (char*)cjson_malloc(newsize);
    if (!newbuffer) {
        cjson_free(p->buffer);
        p->length = 0;
        p->buffer = 0;

        return 0;
    }
    if (newbuffer) {
        memcpy(newbuffer, p->buffer, p->length);
    }
    cjson_free(p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer */
static int update(const printbuffer *p)
{
    char *str;
    if (!p || !p->buffer) {
        return 0;
    }
    str = p->buffer + p->offset;

    return p->offset + strlen(str);
}

/* Render the number nicely from the given item into a string. */
static char *print_number(const cjson_t *item, printbuffer *p)
{
    char *str = 0;
    double d = item->valuedouble;
    /* special case for 0. */
    if (d == 0) {
        if (p) {
            str = ensure(p, 2);
        } else {
            str = (char*)cjson_malloc(2);
        }
        if (str) {
            strcpy(str,"0");
        }
    } else if ((fabs(((double)item->valueint) - d) <= DBL_EPSILON) &&
               (d <= INT_MAX) && (d >= INT_MIN))
    {/* value is an int */
        if (p) {
            str = ensure(p, 21);
        } else {
            /* 2^64+1 can be represented in 21 chars. */
            str = (char*)cjson_malloc(21);
        }
        if (str) {
            sprintf(str, "%d", item->valueint);
        }
    } else { /* value is a floating point number */
        if (p) {
            /* This is a nice tradeoff. */
            str = ensure(p, 64);
        } else {
            /* This is a nice tradeoff. */
            str=(char*)cjson_malloc(64);
        }
        if (str) {
            /* This checks for NaN and Infinity */
            if ((d * 0) != 0) {
                sprintf(str, "null");
            } else if ((fabs(floor(d) - d) <= DBL_EPSILON) && (fabs(d) < 1.0e60)) {
                sprintf(str, "%.0f", d);
            } else if ((fabs(d) < 1.0e-6) || (fabs(d) > 1.0e9)) {
                sprintf(str, "%e", d);
            } else {
                sprintf(str, "%f", d);
            }
        }
    }
    return str;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(const char *str)
{
    unsigned h = 0;
    /* first digit */
    if ((*str >= '0') && (*str <= '9')) {
        h += (*str) - '0';
    } else if ((*str >= 'A') && (*str <= 'F')) {
        h += 10 + (*str) - 'A';
    } else if ((*str >= 'a') && (*str <= 'f')) {
        h += 10 + (*str) - 'a';
    } else /* invalid */ {
        return 0;
    }

    /* second digit */
    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9')) {
        h += (*str) - '0';
    } else if ((*str >= 'A') && (*str <= 'F')) {
        h += 10 + (*str) - 'A';
    } else if ((*str >= 'a') && (*str <= 'f')) {
        h += 10 + (*str) - 'a';
    } else {  /* invalid */
        return 0;
    }

    /* third digit */
    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9')) {
        h += (*str) - '0';
    } else if ((*str >= 'A') && (*str <= 'F')) {
        h += 10 + (*str) - 'A';
    } else if ((*str >= 'a') && (*str <= 'f')) {
        h += 10 + (*str) - 'a';
    } else { /* invalid */
        return 0;
    }

    /* fourth digit */
    h = h << 4;
    str++;
    if ((*str >= '0') && (*str <= '9')) {
        h += (*str) - '0';
    } else if ((*str >= 'A') && (*str <= 'F')) {
        h += 10 + (*str) - 'A';
    } else if ((*str >= 'a') && (*str <= 'f')) {
        h += 10 + (*str) - 'a';
    } else /* invalid */ {
        return 0;
    }

    return h;
}

/* first bytes of UTF8 encoding for a given length in bytes */
static const unsigned char firstByteMark[7] =
{
    0x00, /* should never happen */
    0x00, /* 0xxxxxxx */
    0xC0, /* 110xxxxx */
    0xE0, /* 1110xxxx */
    0xF0, /* 11110xxx */
    0xF8,
    0xFC
};

/* Parse the input text into an unescaped cstring, and populate item. */
static const char *parse_string(cjson_t *item, const char *str, const char **ep)
{
    const char *ptr = str + 1;
    const char *end_ptr =str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc;
    unsigned uc2;

    /* not a string! */
    if (*str != '\"') {
        *ep = str;
        return 0;
    }

    while ((*end_ptr != '\"') && *end_ptr && ++len) {
        if (*end_ptr++ == '\\') {
            if (*end_ptr == '\0') {
                /* prevent buffer overflow when last input character is a backslash */
                return 0;
            }
            /* Skip escaped quotes. */
            end_ptr++;
        }
    }

    /* This is at most how long we need for the string, roughly. */
    out = (char*)cjson_malloc(len + 1);
    if (!out) {
        return 0;
    }
    item->valuestring = out; /* assign here so out will be deleted during cjson_delete() later */
    item->type = CJSON_STRING;

    ptr = str + 1;
    ptr2 = out;
    /* loop through the string literal */
    while (ptr < end_ptr) {
        if (*ptr != '\\') {
            *ptr2++ = *ptr++;
        } else { /* escape sequence */
            ptr++;
            switch (*ptr) {
                case 'b':
                    *ptr2++ = '\b';
                    break;
                case 'f':
                    *ptr2++ = '\f';
                    break;
                case 'n':
                    *ptr2++ = '\n';
                    break;
                case 'r':
                    *ptr2++ = '\r';
                    break;
                case 't':
                    *ptr2++ = '\t';
                    break;
                case 'u':
                    /* transcode utf16 to utf8. See RFC2781 and RFC3629. */
                    uc = parse_hex4(ptr + 1); /* get the unicode char. */
                    ptr += 4;
                    if (ptr >= end_ptr) {
                        /* invalid */
                        *ep = str;
                        return 0;
                    }
                    /* check for invalid. */
                    if (((uc >= 0xDC00) && (uc <= 0xDFFF)) || (uc == 0)) {
                        *ep = str;
                        return 0;
                    }

                    /* UTF16 surrogate pairs. */
                    if ((uc >= 0xD800) && (uc<=0xDBFF)) {
                        if ((ptr + 6) > end_ptr) {
                            /* invalid */
                            *ep = str;
                            return 0;
                        }
                        if ((ptr[1] != '\\') || (ptr[2] != 'u')) {
                            /* missing second-half of surrogate. */
                            *ep = str;
                            return 0;
                        }
                        uc2 = parse_hex4(ptr + 3);
                        ptr += 6; /* \uXXXX */
                        if ((uc2 < 0xDC00) || (uc2 > 0xDFFF)) {
                            /* invalid second-half of surrogate. */
                            *ep = str;
                            return 0;
                        }
                        /* calculate unicode codepoint from the surrogate pair */
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }

                    /* encode as UTF8
                     * takes at maximum 4 bytes to encode:
                     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
                    len = 4;
                    if (uc < 0x80) {
                        /* normal ascii, encoding 0xxxxxxx */
                        len = 1;
                    } else if (uc < 0x800) {
                        /* two bytes, encoding 110xxxxx 10xxxxxx */
                        len = 2;
                    } else if (uc < 0x10000) {
                        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
                        len = 3;
                    }
                    ptr2 += len;

                    switch (len) {
                        case 4:
                            /* 10xxxxxx */
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 3:
                            /* 10xxxxxx */
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2:
                            /* 10xxxxxx */
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1:
                            /* depending on the length in bytes this determines the
                             * encoding ofthe first UTF8 byte */
                            *--ptr2 = (uc | firstByteMark[len]);
                    }
                    ptr2 += len;
                    break;
                default:
                    *ptr2++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *ptr2 = '\0';
    if (*ptr == '\"')
    {
        ptr++;
    }

    return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr(const char *str, printbuffer *p)
{
    const char *ptr;
    char *ptr2;
    char *out;
    int len = 0;
    int flag = 0;
    unsigned char token;

    /* empty string */
    if (!str) {
        if (p) {
            out = ensure(p, 3);
        } else {
            out = (char*)cjson_malloc(3);
        }
        if (!out) {
            return 0;
        }
        strcpy(out, "\"\"");

        return out;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (ptr = str; *ptr; ptr++) {
        flag |= (((*ptr > 0) && (*ptr < 32)) /* unprintable characters */
                || (*ptr == '\"') /* double quote */
                || (*ptr == '\\')) /* backslash */
            ? 1
            : 0;
    }
    /* no characters have to be escaped */
    if (!flag) {
        len = ptr - str;
        if (p) {
            out = ensure(p, len + 3);
        } else {
            out = (char*)cjson_malloc(len + 3);
        }
        if (!out) {
            return 0;
        }

        ptr2 = out;
        *ptr2++ = '\"';
        strcpy(ptr2, str);
        ptr2[len] = '\"';
        ptr2[len + 1] = '\0';

        return out;
    }

    ptr = str;
    /* calculate additional space that is needed for escaping */
    while ((token = *ptr) && ++len) {
        if (strchr("\"\\\b\f\n\r\t", token)) {
            len++; /* +1 for the backslash */
        } else if (token < 32) {
            len += 5; /* +5 for \uXXXX */
        }
        ptr++;
    }

    if (p) {
        out = ensure(p, len + 3);
    } else {
        out = (char*)cjson_malloc(len + 3);
    }
    if (!out) {
        return 0;
    }

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    /* copy the string */
    while (*ptr) {
        if (((unsigned char)*ptr > 31) && (*ptr != '\"') && (*ptr != '\\')) {
            /* normal character, copy */
            *ptr2++ = *ptr++;
        } else {
            /* character needs to be escaped */
            *ptr2++ = '\\';
            switch (token = *ptr++) {
                case '\\':
                    *ptr2++ = '\\';
                    break;
                case '\"':
                    *ptr2++ = '\"';
                    break;
                case '\b':
                    *ptr2++ = 'b';
                    break;
                case '\f':
                    *ptr2++ = 'f';
                    break;
                case '\n':
                    *ptr2++ = 'n';
                    break;
                case '\r':
                    *ptr2++ = 'r';
                    break;
                case '\t':
                    *ptr2++ = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    sprintf(ptr2, "u%04x", token);
                    ptr2 += 5;
                    break;
            }
        }
    }
    *ptr2++ = '\"';
    *ptr2++ = '\0';

    return out;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static char *print_string(const cjson_t *item, printbuffer *p)
{
    return print_string_ptr(item->valuestring, p);
}

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
    while (in && *in && ((unsigned char)*in<=32)) {
        in++;
    }

    return in;
}

/* Parse an object - create a new root, and populate. */
cjson_t *cjson_parse_with_opts(const char *value, const char **return_parse_end, int require_null_terminated)
{
    const char *end = 0;
    /* use global error pointer if no specific one was given */
    const char **ep = return_parse_end ? return_parse_end : &global_ep;
    cjson_t *c = cjson_new_item();
    *ep = 0;
    if (!c) {/* memory fail */
        return 0;
    }

    end = parse_value(c, skip(value), ep);
    if (!end) {
        /* parse failure. ep is set. */
        cjson_delete(c);
        return 0;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated) {
        end = skip(end);
        if (*end) {
            cjson_delete(c);
            *ep = end;
            return 0;
        }
    }
    if (return_parse_end) {
        *return_parse_end = end;
    }

    return c;
}

/* Default options for cjson_parse */
cjson_t *cjson_parse(const char *value)
{
    return cjson_parse_with_opts(value, 0, 0);
}

/* Render a cjson_t item/entity/structure to text. */
char *cjson_print(const cjson_t *item)
{
    return print_value(item, 0, 1, 0);
}

char *cjson_print_unformatted(const cjson_t *item)
{
    return print_value(item, 0, 0, 0);
}

char *cjson_print_bufferd(const cjson_t *item, int prebuffer, int fmt)
{
    printbuffer p;
    p.buffer = (char*)cjson_malloc(prebuffer);
    if (!p.buffer) {
        return 0;
    }
    p.length = prebuffer;
    p.offset = 0;

    return print_value(item, 0, fmt, &p);
}


/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(cjson_t *item, const char *value, const char **ep)
{
    if (!value) {
        /* Fail on null. */
        return 0;
    }

    /* parse the different types of values */
    if (!strncmp(value, "null", 4)) {
        item->type = CJSON_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5)) {
        item->type = CJSON_FALSE;
        return value + 5;
    }
    if (!strncmp(value, "true", 4)) {
        item->type = CJSON_TRUE;
        item->valueint = 1;
        return value + 4;
    }
    if (*value == '\"') {
        return parse_string(item, value, ep);
    }
    if ((*value == '-') || ((*value >= '0') && (*value <= '9'))) {
        return parse_number(item, value);
    }
    if (*value == '[') {
        return parse_array(item, value, ep);
    }
    if (*value == '{') {
        return parse_object(item, value, ep);
    }

    *ep=value;return 0; /* failure. */
}

/* Render a value to text. */
static char *print_value(const cjson_t *item, int depth, int fmt, printbuffer *p)
{
    char *out = 0;

    if (!item) {
        return 0;
    }
    if (p) {
        switch ((item->type) & 0xFF) {
            case CJSON_NULL:
                out = ensure(p, 5);
                if (out) {
                    strcpy(out, "null");
                }
                break;
            case CJSON_FALSE:
                out = ensure(p, 6);
                if (out) {
                    strcpy(out, "false");
                }
                break;
            case CJSON_TRUE:
                out = ensure(p, 5);
                if (out) {
                    strcpy(out, "true");
                }
                break;
            case CJSON_NUMBER:
                out = print_number(item, p);
                break;
            case CJSON_STRING:
                out = print_string(item, p);
                break;
            case CJSON_ARRAY:
                out = print_array(item, depth, fmt, p);
                break;
            case CJSON_OBJECT:
                out = print_object(item, depth, fmt, p);
                break;
        }
    } else {
        switch ((item->type) & 0xFF) {
            case CJSON_NULL:
                out = cjson_strdup("null");
                break;
            case CJSON_FALSE:
                out = cjson_strdup("false");
                break;
            case CJSON_TRUE:
                out = cjson_strdup("true");
                break;
            case CJSON_NUMBER:
                out = print_number(item, 0);
                break;
            case CJSON_STRING:
                out = print_string(item, 0);
                break;
            case CJSON_ARRAY:
                out = print_array(item, depth, fmt, 0);
                break;
            case CJSON_OBJECT:
                out = print_object(item, depth, fmt, 0);
                break;
        }
    }

    return out;
}

/* Build an array from input text. */
static const char *parse_array(cjson_t *item,const char *value,const char **ep)
{
    cjson_t *child;
    if (*value != '[') {
        /* not an array! */
        *ep = value;
        return 0;
    }

    item->type = CJSON_ARRAY;
    value = skip(value + 1);
    if (*value == ']') {
        /* empty array. */
        return value + 1;
    }

    item->child = child = cjson_new_item();
    if (!item->child) {
        /* memory fail */
        return 0;
    }
    /* skip any spacing, get the value. */
    value = skip(parse_value(child, skip(value), ep));
    if (!value) {
        return 0;
    }

    /* loop through the comma separated array elements */
    while (*value == ',') {
        cjson_t *new_item;
        if (!(new_item = cjson_new_item())) {
            /* memory fail */
            return 0;
        }
        /* add new item to end of the linked list */
        child->next = new_item;
        new_item->prev = child;
        child = new_item;

        /* go to the next comma */
        value = skip(parse_value(child, skip(value + 1), ep));
        if (!value) {
            /* memory fail */
            return 0;
        }
    }

    if (*value == ']') {
        /* end of array */
        return value + 1;
    }

    /* malformed. */
    *ep = value;

    return 0;
}

/* Render an array to text */
static char *print_array(const cjson_t *item, int depth, int fmt, printbuffer *p)
{
    char **entries;
    char *out = 0;
    char *ptr;
    char *ret;
    int len = 5;
    cjson_t *child = item->child;
    int numentries = 0;
    int i = 0;
    int fail = 0;
    size_t tmplen = 0;

    /* How many entries in the array? */
    while (child) {
        numentries++;
        child = child->next;
    }

    /* Explicitly handle numentries == 0 */
    if (!numentries) {
        if (p) {
            out = ensure(p, 3);
        } else {
            out = (char*)cjson_malloc(3);
        }
        if (out) {
            strcpy(out,"[]");
        }

        return out;
    }

    if (p) {
        /* Compose the output array. */
        /* opening square bracket */
        i = p->offset;
        ptr = ensure(p, 1);
        if (!ptr) {
            return 0;
        }
        *ptr = '[';
        p->offset++;

        child = item->child;
        while (child && !fail) {
            print_value(child, depth + 1, fmt, p);
            p->offset = update(p);
            if (child->next) {
                len = fmt ? 2 : 1;
                ptr = ensure(p, len + 1);
                if (!ptr) {
                    return 0;
                }
                *ptr++ = ',';
                if(fmt) {
                    *ptr++ = ' ';
                }
                *ptr = 0;
                p->offset += len;
            }
            child = child->next;
        }
        ptr = ensure(p, 2);
        if (!ptr) {
            return 0;
        }
        *ptr++ = ']';
        *ptr = '\0';
        out = (p->buffer) + i;
    } else {
        /* Allocate an array to hold the pointers to all printed values */
        entries = (char**)cjson_malloc(numentries * sizeof(char*));
        if (!entries) {
            return 0;
        }
        memset(entries, 0, numentries * sizeof(char*));

        /* Retrieve all the results: */
        child = item->child;
        while (child && !fail) {
            ret = print_value(child, depth + 1, fmt, 0);
            entries[i++] = ret;
            if (ret) {
                len += strlen(ret) + 2 + (fmt ? 1 : 0);
            } else {
                fail = 1;
            }
            child = child->next;
        }

        /* If we didn't fail, try to malloc the output string */
        if (!fail) {
            out = (char*)cjson_malloc(len);
        }
        /* If that fails, we fail. */
        if (!out) {
            fail = 1;
        }

        /* Handle failure. */
        if (fail) {
            /* free all the entries in the array */
            for (i = 0; i < numentries; i++) {
                if (entries[i]) {
                    cjson_free(entries[i]);
                }
            }
            cjson_free(entries);
            return 0;
        }

        /* Compose the output array. */
        *out='[';
        ptr = out + 1;
        *ptr = '\0';
        for (i = 0; i < numentries; i++) {
            tmplen = strlen(entries[i]);
            memcpy(ptr, entries[i], tmplen);
            ptr += tmplen;
            if (i != (numentries - 1)) {
                *ptr++ = ',';
                if(fmt) {
                    *ptr++ = ' ';
                }
                *ptr = 0;
            }
            cjson_free(entries[i]);
        }
        cjson_free(entries);
        *ptr++ = ']';
        *ptr++ = '\0';
    }

    return out;
}

/* Build an object from the text. */
static const char *parse_object(cjson_t *item, const char *value, const char **ep)
{
    cjson_t *child;
    if (*value != '{') {
        /* not an object! */
        *ep = value;
        return 0;
    }

    item->type = CJSON_OBJECT;
    value = skip(value + 1);
    if (*value == '}') {
        /* empty object. */
        return value + 1;
    }

    child = cjson_new_item();
    item->child = child;
    if (!item->child) {
        return 0;
    }
    /* parse first key */
    value = skip(parse_string(child, skip(value), ep));
    if (!value) {
        return 0;
    }
    /* use string as key, not value */
    child->string = child->valuestring;
    child->valuestring = 0;

    if (*value != ':') {
        /* invalid object. */
        *ep = value;
        return 0;
    }
    /* skip any spacing, get the value. */
    value = skip(parse_value(child, skip(value + 1), ep));
    if (!value) {
        return 0;
    }

    while (*value == ',') {
        cjson_t *new_item;
        if (!(new_item = cjson_new_item())) {
            /* memory fail */
            return 0;
        }
        /* add to linked list */
        child->next = new_item;
        new_item->prev = child;

        child = new_item;
        value = skip(parse_string(child, skip(value + 1), ep));
        if (!value) {
            return 0;
        }

        /* use string as key, not value */
        child->string = child->valuestring;
        child->valuestring = 0;

        if (*value != ':') {
            /* invalid object. */
            *ep = value;
            return 0;
        }
        /* skip any spacing, get the value. */
        value = skip(parse_value(child, skip(value + 1), ep));
        if (!value) {
            return 0;
        }
    }
    /* end of object */
    if (*value == '}') {
        return value + 1;
    }

    /* malformed */
    *ep = value;
    return 0;
}

/* Render an object to text. */
static char *print_object(const cjson_t *item, int depth, int fmt, printbuffer *p)
{
    char **entries = 0;
    char **names = 0;
    char *out = 0;
    char *ptr;
    char *ret;
    char *str;
    int len = 7;
    int i = 0;
    int j;
    cjson_t *child = item->child;
    int numentries = 0;
    int fail = 0;
    size_t tmplen = 0;

    /* Count the number of entries. */
    while (child) {
        numentries++;
        child = child->next;
    }

    /* Explicitly handle empty object case */
    if (!numentries) {
        if (p) {
            out = ensure(p, fmt ? depth + 4 : 3);
        } else {
            out = (char*)cjson_malloc(fmt ? depth + 4 : 3);
        }
        if (!out) {
            return 0;
        }
        ptr = out;
        *ptr++ = '{';
        if (fmt) {
            *ptr++ = '\n';
            for (i = 0; i < depth; i++) {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr++ = '\0';

        return out;
    }

    if (p) {
        /* Compose the output: */
        i = p->offset;
        len = fmt ? 2 : 1; /* fmt: {\n */
        ptr = ensure(p, len + 1);
        if (!ptr) {
            return 0;
        }

        *ptr++ = '{';
        if (fmt) {
            *ptr++ = '\n';
        }
        *ptr = '\0';
        p->offset += len;

        child = item->child;
        depth++;
        while (child) {
            if (fmt) {
                ptr = ensure(p, depth);
                if (!ptr) {
                    return 0;
                }
                for (j = 0; j < depth; j++) {
                    *ptr++ = '\t';
                }
                p->offset += depth;
            }

            /* print key */
            print_string_ptr(child->string, p);
            p->offset = update(p);

            len = fmt ? 2 : 1;
            ptr = ensure(p, len);
            if (!ptr) {
                return 0;
            }
            *ptr++ = ':';
            if (fmt) {
                *ptr++ = '\t';
            }
            p->offset+=len;

            /* print value */
            print_value(child, depth, fmt, p);
            p->offset = update(p);

            /* print comma if not last */
            len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
            ptr = ensure(p, len + 1);
            if (!ptr) {
                return 0;
            }
            if (child->next) {
                *ptr++ = ',';
            }

            if (fmt) {
                *ptr++ = '\n';
            }
            *ptr = '\0';
            p->offset += len;

            child = child->next;
        }

        ptr = ensure(p, fmt ? (depth + 1) : 2);
        if (!ptr) {
            return 0;
        }
        if (fmt) {
            for (i = 0; i < (depth - 1); i++) {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr = '\0';
        out = (p->buffer) + i;
    } else {
        /* Allocate space for the names and the objects */
        entries = (char**)cjson_malloc(numentries * sizeof(char*));
        if (!entries) {
            return 0;
        }
        names = (char**)cjson_malloc(numentries * sizeof(char*));
        if (!names) {
            cjson_free(entries);
            return 0;
        }
        memset(entries,0, sizeof(char*) * numentries);
        memset(names, 0, sizeof(char*) * numentries);

        /* Collect all the results into our arrays: */
        child = item->child;
        depth++;
        if (fmt) {
            len += depth;
        }
        while (child && !fail) {
            names[i] = str = print_string_ptr(child->string, 0); /* print key */
            entries[i++] = ret = print_value(child, depth, fmt, 0);
            if (str && ret) {
                len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
            } else {
                fail = 1;
            }
            child = child->next;
        }

        /* Try to allocate the output string */
        if (!fail) {
            out = (char*)cjson_malloc(len);
        }
        if (!out) {
            fail = 1;
        }

        /* Handle failure */
        if (fail) {
            /* free all the printed keys and values */
            for (i = 0; i < numentries; i++) {
                if (names[i]) {
                    cjson_free(names[i]);
                }
                if (entries[i]) {
                    cjson_free(entries[i]);
                }
            }
            cjson_free(names);
            cjson_free(entries);
            return 0;
        }

        /* Compose the output: */
        *out = '{';
        ptr = out + 1;
        if (fmt) {
            *ptr++ = '\n';
        }
        *ptr = 0;
        for (i = 0; i < numentries; i++) {
            if (fmt) {
                for (j = 0; j < depth; j++) {
                    *ptr++='\t';
                }
            }
            tmplen = strlen(names[i]);
            memcpy(ptr, names[i], tmplen);
            ptr += tmplen;
            *ptr++ = ':';
            if (fmt) {
                *ptr++ = '\t';
            }
            strcpy(ptr, entries[i]);
            ptr += strlen(entries[i]);
            if (i != (numentries - 1)) {
                *ptr++ = ',';
            }
            if (fmt) {
                *ptr++ = '\n';
            }
            *ptr = 0;
            cjson_free(names[i]);
            cjson_free(entries[i]);
        }

        cjson_free(names);
        cjson_free(entries);
        if (fmt) {
            for (i = 0; i < (depth - 1); i++) {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr++ = '\0';
    }

    return out;
}

/* Get Array size/item / object item. */
int cjson_get_array_size(const cjson_t *array)
{
    cjson_t *c = array->child;
    int i = 0;
    while(c) {
        i++;
        c = c->next;
    }
    return i;
}

cjson_t *cjson_get_array_item(const cjson_t *array, int item)
{
    cjson_t *c = array ? array->child : 0;
    while (c && item > 0) {
        item--;
        c = c->next;
    }

    return c;
}

cjson_t *cjson_get_object_item(const cjson_t *object, const char *string)
{
    cjson_t *c = object ? object->child : 0;
    while (c && cjson_strcasecmp(c->string, string)) {
        c = c->next;
    }
    return c;
}

int cjson_has_object_item(const cjson_t *object,const char *string)
{
    return cjson_get_object_item(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(cjson_t *prev, cjson_t *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static cjson_t *create_reference(const cjson_t *item)
{
    cjson_t *ref = cjson_new_item();
    if (!ref) {
        return 0;
    }
    memcpy(ref, item, sizeof(cjson_t));
    ref->string = 0;
    ref->type |= CJSON_ISREFERENCE;
    ref->next = ref->prev = 0;
    return ref;
}

/* Add item to array/object. */
void   cjson_add_item_to_array(cjson_t *array, cjson_t *item)
{
    cjson_t *c = array->child;
    if (!item) {
        return;
    }
    if (!c) {
        /* list is empty, start new one */
        array->child = item;
    } else {
        /* append to the end */
        while (c->next) {
            c = c->next;
        }
        suffix_object(c, item);
    }
}

void   cjson_add_item_to_object(cjson_t *object, const char *string, cjson_t *item)
{
    if (!item) {
        return;
    }

    /* free old key and set new one */
    if (item->string) {
        cjson_free(item->string);
    }
    item->string = cjson_strdup(string);

    cjson_add_item_to_array(object,item);
}

/* Add an item to an object with constant string as key */
void   cjson_add_item_to_object_cs(cjson_t *object, const char *string, cjson_t *item)
{
    if (!item) {
        return;
    }
    if (!(item->type & CJSON_STRINGISCONST) && item->string) {
        cjson_free(item->string);
    }
    item->string = (char*)string;
    item->type |= CJSON_STRINGISCONST;
    cjson_add_item_to_array(object, item);
}

void cjson_add_item_reference_to_array(cjson_t *array, cjson_t *item)
{
    cjson_add_item_to_array(array, create_reference(item));
}

void cjson_add_item_reference_to_object(cjson_t *object, const char *string, cjson_t *item)
{
    cjson_add_item_to_object(object, string, create_reference(item));
}

cjson_t *cjson_detach_item_from_array(cjson_t *array, int which)
{
    cjson_t *c = array->child;
    while (c && (which > 0)) {
        c = c->next;
        which--;
    }
    if (!c) {
        /* item doesn't exist */
        return 0;
    }
    if (c->prev) {
        /* not the first element */
        c->prev->next = c->next;
    }
    if (c->next) {
        c->next->prev = c->prev;
    }
    if (c==array->child) {
        array->child = c->next;
    }
    /* make sure the detached item doesn't point anywhere anymore */
    c->prev = c->next = 0;

    return c;
}

void cjson_delete_item_from_array(cjson_t *array, int which)
{
    cjson_delete(cjson_detach_item_from_array(array, which));
}

cjson_t *cjson_detach_item_from_object(cjson_t *object, const char *string)
{
    int i = 0;
    cjson_t *c = object->child;
    while (c && cjson_strcasecmp(c->string,string)) {
        i++;
        c = c->next;
    }
    if (c) {
        return cjson_detach_item_from_array(object, i);
    }

    return 0;
}

void cjson_delete_item_from_object(cjson_t *object, const char *string)
{
    cjson_delete(cjson_detach_item_from_object(object, string));
}

/* Replace array/object items with new ones. */
void cjson_insert_item_in_array(cjson_t *array, int which, cjson_t *newitem)
{
    cjson_t *c = array->child;
    while (c && (which > 0)) {
        c = c->next;
        which--;
    }
    if (!c) {
        cjson_add_item_to_array(array, newitem);
        return;
    }
    newitem->next = c;
    newitem->prev = c->prev;
    c->prev = newitem;
    if (c == array->child) {
        array->child = newitem;
    } else {
        newitem->prev->next = newitem;
    }
}

void cjson_replace_item_in_array(cjson_t *array, int which, cjson_t *newitem)
{
    cjson_t *c = array->child;
    while (c && (which > 0)) {
        c = c->next;
        which--;
    }
    if (!c) {
        return;
    }
    newitem->next = c->next;
    newitem->prev = c->prev;
    if (newitem->next) {
        newitem->next->prev = newitem;
    }
    if (c == array->child) {
        array->child = newitem;
    } else {
        newitem->prev->next = newitem;
    }
    c->next = c->prev = 0;
    cjson_delete(c);
}

void cjson_replace_item_in_object(cjson_t *object, const char *string, cjson_t *newitem)
{
    int i = 0;
    cjson_t *c = object->child;
    while(c && cjson_strcasecmp(c->string, string)) {
        i++;
        c = c->next;
    }
    if(c) {
        newitem->string = cjson_strdup(string);
        cjson_replace_item_in_array(object, i, newitem);
    }
}

/* Create basic types: */
cjson_t *cjson_create_null(void)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = CJSON_NULL;
    }

    return item;
}

cjson_t *cjson_create_true(void)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = CJSON_TRUE;
    }

    return item;
}

cjson_t *cjson_create_false(void)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = CJSON_FALSE;
    }

    return item;
}

cjson_t *cjson_create_bool(int b)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = b ? CJSON_TRUE : CJSON_FALSE;
    }

    return item;
}

cjson_t *cjson_create_number(double num)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = CJSON_NUMBER;
        item->valuedouble = num;
        item->valueint = (int)num;
    }

    return item;
}

cjson_t *cjson_create_string(const char *string)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type = CJSON_STRING;
        item->valuestring = cjson_strdup(string);
        if(!item->valuestring) {
            cjson_delete(item);
            return 0;
        }
    }

    return item;
}

cjson_t *cjson_create_array(void)
{
    cjson_t *item = cjson_new_item();
    if(item) {
        item->type=CJSON_ARRAY;
    }

    return item;
}

cjson_t *cjson_create_object(void)
{
    cjson_t *item = cjson_new_item();
    if (item) {
        item->type = CJSON_OBJECT;
    }

    return item;
}

/* Create Arrays: */
cjson_t *cjson_create_int_array(const int *numbers, int count)
{
    int i;
    cjson_t *n = 0;
    cjson_t *p = 0;
    cjson_t *a = cjson_create_array();
    for(i = 0; a && (i < count); i++) {
        n = cjson_create_number(numbers[i]);
        if (!n) {
            cjson_delete(a);
            return 0;
        }
        if(!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

cjson_t *cjson_create_float_array(const float *numbers, int count)
{
    int i;
    cjson_t *n = 0;
    cjson_t *p = 0;
    cjson_t *a = cjson_create_array();
    for(i = 0; a && (i < count); i++) {
        n = cjson_create_number(numbers[i]);
        if(!n) {
            cjson_delete(a);
            return 0;
        }
        if(!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

cjson_t *cjson_create_double_array(const double *numbers, int count)
{
    int i;
    cjson_t *n = 0;
    cjson_t *p = 0;
    cjson_t *a = cjson_create_array();
    for(i = 0;a && (i < count); i++) {
        n = cjson_create_number(numbers[i]);
        if(!n) {
            cjson_delete(a);
            return 0;
        }
        if(!i) {
            a->child = n;
        } else {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

cjson_t *cjson_create_string_array(const char **strings, int count)
{
    int i;
    cjson_t *n = 0;
    cjson_t *p = 0;
    cjson_t *a = cjson_create_array();
    for (i = 0; a && (i < count); i++) {
        n = cjson_create_string(strings[i]);
        if(!n) {
            cjson_delete(a);
            return 0;
        }
        if(!i) {
            a->child = n;
        } else {
            suffix_object(p,n);
        }
        p = n;
    }

    return a;
}

/* Duplication */
cjson_t *cjson_duplicate(const cjson_t *item, int recurse)
{
    cjson_t *newitem;
    cjson_t *cptr;
    cjson_t *nptr = 0;
    cjson_t *newchild;

    /* Bail on bad ptr */
    if (!item) {
        return 0;
    }
    /* Create new item */
    newitem = cjson_new_item();
    if (!newitem) {
        return 0;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~CJSON_ISREFERENCE);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring) {
        newitem->valuestring = cjson_strdup(item->valuestring);
        if (!newitem->valuestring) {
            cjson_delete(newitem);
            return 0;
        }
    }
    if (item->string) {
        newitem->string = cjson_strdup(item->string);
        if (!newitem->string) {
            cjson_delete(newitem);
            return 0;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse) {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    cptr = item->child;
    while (cptr) {
        newchild = cjson_duplicate(cptr, 1); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild) {
            cjson_delete(newitem);
            return 0;
        }
        if (nptr) {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            nptr->next = newchild;
            newchild->prev = nptr;
            nptr = newchild;
        } else {
            /* Set newitem->child and move to it */
            newitem->child = newchild; nptr = newchild;
        }
        cptr = cptr->next;
    }

    return newitem;
}

void cjson_minify(char *json)
{
    char *into = json;
    while (*json) {
        if (*json == ' ') {
            json++;
        } else if (*json == '\t') {
            /* Whitespace characters. */
            json++;
        } else if (*json == '\r') {
            json++;
        } else if (*json=='\n') {
            json++;
        } else if ((*json == '/') && (json[1] == '/')) {
            /* double-slash comments, to end of line. */
            while (*json && (*json != '\n')) {
                json++;
            }
        } else if ((*json == '/') && (json[1] == '*')) {
            /* multiline comments. */
            while (*json && !((*json == '*') && (json[1] == '/'))) {
                json++;
            }
            json += 2;
        } else if (*json == '\"') {
            /* string literals, which are \" sensitive. */
            *into++ = *json++;
            while (*json && (*json != '\"')) {
                if (*json == '\\') {
                    *into++=*json++;
                }
                *into++ = *json++;
            }
            *into++ = *json++;
        } else {
            /* All other characters. */
            *into++ = *json++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}
