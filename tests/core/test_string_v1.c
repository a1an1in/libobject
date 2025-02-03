#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/string.h>
#include <libobject/mockery/mockery.h>

static int test_string_v1_hex_to_int()
{
    char buf[1024] = "0x123456789abc";
    long long expect = 0x123456789abc, d;
    int ret;

    TRY {
        d = str_hex_to_integer(buf);
        THROW_IF(d != expect, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "expect:%llx, real:%llx", expect, d);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v1_hex_to_int);

int test_string_v1_strchr_n(TEST_ENTRY *entry, int argc, void **argv)
{
    char *str = "lbrnsepcfjzcpfgzqdiujo";
    char *p;
    int len, i, ret;

    TRY {
        p = strchr_n(str, 'l', 2);
        THROW_IF(p != NULL, -1);
        p = strchr_n(str, 'j', 2);
        THROW_IF(p == NULL, -1);

    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_string_v1_strchr_n);

int test_string_v1_remove_spaces_around_comma(TEST_ENTRY *entry, int argc, void **argv)
{
    char str[1024] = "  hello ,  world ,  this  is , a test  ";
    char expect[1024] = "  hello,world,this  is,a test  ";
    int ret;

    TRY {
        str_remove_spaces_around_comma(str);
        THROW_IF(strcmp(str, expect) != 0, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_string_v1_remove_spaces_around_comma);