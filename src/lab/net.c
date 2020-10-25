#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

#if (defined(WINDOWS_USER_MODE))
static char *
strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL)
        s = *save_ptr;

    /* Scan leading delimiters. */
    s += strspn (s, delim);
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }

    /* Find the end of the token. */
    token = s;
    s = strpbrk (token, delim);
    if (s == NULL)
        /* This token finishes the string. */
        *save_ptr = strchr (token, '\0');
    else {
        /* Terminate the token and make *SAVE_PTR point past it. */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}
#endif

static int mystr_split(char *str, char *delims, char ***out)
{
    int index = 0;
    char *ptr = NULL, **addr, *p;
    int i, j, count = 0, delim_len = strlen(delims);
    int str_len;

    str_len = strlen(str);

    for (i = 0; i < delim_len; i++) {
        for (j = 0; j < str_len; j++) {
            if (str[j] == delims[i]) {
                count++;
            }
        }
    }

    if (count == 0) return 0;

    addr = malloc(sizeof(void *) * (count + 1));
    memset(addr, 0, sizeof(void *) * (count + 1));
    p = malloc(str_len + 1);
    strcpy(p, str);

    for (p = strtok_r(p, delims, &ptr); p;
         p = strtok_r(NULL, delims, &ptr)) {
        *(addr + index) = p;
        index++;
    }

    if (index > 0) {
        *out = addr;
    }

    free(p);

    return count;
}

extern int bubble_sort(int *array, int len);
int test_mystr_split(TEST_ENTRY *entry)
{
    char *str = "ASCE 789 123 456";
    char **out;
    int count, i;
    int *array;

    dbg_str(DBG_SUC, "test str split");
    count = mystr_split(str, " ", &out);

    array = malloc(sizeof(int) * (count - 1));

    for (i = 1; i < count; i++) {
        printf("%d %s\n", i, out[i]);
        array[i - 1] = atoi(out[i]);
        printf("array %d %d\n", i, array[i - 1]);
    }

    for (i = 0; i < count - 1; i++) {
        printf("%d %d\n", i, array[i]);
    }

    bubble_sort(array, count - 1);

    for (i = 0; i < count - 1; i++) {
        printf("%d %d\n", i, array[i]);
    }

    free(array);

    return 1;
}
REGISTER_TEST_CMD(test_mystr_split);

int inet_check_ipv4(char *ip)
{
    char **out = NULL;
    int count = 0;
    int i;
    int len;
    int j;
    int ret = 1;
    int num;

    TRY {
        count = mystr_split(ip, ".", &out);
        THROW_IF(count != 3 || out == NULL, 0);

        for (i = 0; i < count + 1; i++) {
            THROW_IF(out[i] == NULL, 0);
            len = strlen(out[i]);
            THROW_IF(len == 0 || (len > 3) || (len > 1 && out[i][0] == '0'), 0);
            for (j = 0; j < len; j++) {
                THROW_IF(out[i][j] < '0' || out[i][j] > '9', 0);
            }
            num = atoi(out[i]);
            THROW_IF(num < 0 || num > 255, 0);
        }
        dbg_str(DBG_DETAIL,"%s is IPv4", ip);
    } CATCH(ret) {
        if (ret < 0) {
            dbg_str(DBG_ERROR, "[%s] error_line:%d, error_code:%d",
                    ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        }
    }

    return ret;
}

int inet_check_ipv6(char *ip)
{
    char **out = NULL;
    int count = 0;
    int i;
    int j;
    int len;
    int ret = 1;
    int num;

    TRY {
        count = mystr_split(ip, ":", &out);
        THROW_IF(count != 7 || out == NULL, 0);

        for (i = 0; i < count + 1; i++) {
            THROW_IF(out[i] == NULL, 0);
            len = strlen(out[i]);
            THROW_IF(len== 0 || (len > 4), 0);

            for (j = 0; j < len; j++) {
                THROW_IF((!(out[i][j] >= '0' && out[i][j] <= '9') &&
                          !(out[i][j] >= 'A' && out[i][j] <= 'F') &&
                          !(out[i][j] >= 'a' && out[i][j] <= 'f')), 0);
            }
            num = atoi(out[i]);
            THROW_IF(num < 0 || num > 0xffff, 0);
        }

        dbg_str(DBG_DETAIL,"%s is IPv4", ip);
    } CATCH(ret) {
        if (ret < 0) {
            dbg_str(DBG_ERROR, "[%s] error_line:%d, error_code:%d",
                    ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        }
    }

    return ret;
}

int inet_check_ip(char *ip)
{
    int ret = 1;

    TRY {
        if (strchr(ip, '.')){
            EXEC(ret = inet_check_ipv4(ip));
        } else if (strchr(ip, ':')) {
            EXEC(ret = inet_check_ipv6(ip));
        }

        if (ret == 0) {
            dbg_str(DBG_DETAIL,"%s is not IP address", ip);
        }
    } CATCH(ret) {
        dbg_str(DBG_ERROR, "[%s] error_line:%d, error_code:%d",
                ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

int test_check_ip(TEST_ENTRY *entry)
{
    inet_check_ip("172.16.254.2");
    inet_check_ip(":::::::");
    inet_check_ip("02.16.254.1");
    inet_check_ip("2.16.2254.1");
    inet_check_ip("2.16.2254.");
    inet_check_ip("2001:0db8:85a3:0:0:8A2E:0370:7334");
    inet_check_ip("2001:0db8:85a3:0:0:8A2E:0370:7334:");
    inet_check_ip("2001:0dG8:85a3:0:0:8A2E:0370:7334");
    inet_check_ip("2001:0dG8:85a3:0:0:8A2E:0370:");
    inet_check_ip("2001:0dG8:85a3:0:0:8A2E:0370");

    return 1;
}
REGISTER_TEST_CMD(test_check_ip);

int test_strtok_r(TEST_ENTRY *entry)
{
    char str[] = "alan is very best";
    char *temp;
    char *p;

    p = str;
    while( (p = strtok_r(p, " ", &temp)) != NULL) {
        printf("%s\n", p);
        p = NULL;
    }
}
REGISTER_TEST_CMD(test_strtok_r);
