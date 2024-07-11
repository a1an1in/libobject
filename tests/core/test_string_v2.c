/**
 * @file String_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>
#include <libobject/core/String.h>

static int test_string_v2_get_cstr(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *demo = "abcdefg";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, demo);  

        THROW_IF(strcmp(string->get_cstr(string), demo) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_get_cstr);

static int test_string_v2_sprintf(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        sprintf(test3, "%s, %s", test1, test2);

        string->reset(string);  
        string->format(string, 1024, "%s, %s", test1, test2);

        THROW_IF(strcmp(string->get_cstr(string), test3) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_sprintf);

static int test_string_v2_append(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        sprintf(test3, "%s%s", test1, test2);

        string->assign(string, test1);  
        string->append(string, test2, -1);

        THROW_IF(strcmp(string->get_cstr(string), test3) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_append);

static int test_string_v2_len(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "abcdefg";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, test1);

        THROW_IF(strcmp(string->get_cstr(string), test1) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_len);

static int test_string_v2_get_substring_case0()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    /*
     *char *t = "Content-Disposition: filename=\"u=1573948710,2851629614&fm=26&fmt=auto&gp=0.webp\"";
     */
    char *t = "str:content-disposition: form-data; name=\"file\"; filename=\"snipaste_20210907_14-25-03.png\"";
    char *regex = "filename=\"([a-z0-9A-Z._,&=-]+)\"";
    char buffer[1024] = {0};
    int ret, start = -1, len;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, t);
        EXEC(string->get_substring(string, regex, 0, &start, &len));
        strncpy(buffer, &string->value[start], len);
        dbg_str(DBG_DETAIL, "start:%d", start);
        dbg_str(DBG_INFO, "sub:%s", buffer);
        THROW_IF(start == -1, -1);
        THROW_IF(strcmp(buffer, "snipaste_20210907_14-25-03.png") != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_get_substring_case1()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *t = "userName=alan; userId=4; token=3cb1257cb544ccca19df70cfa327392123ccdce; maxAge=900";
    char *regex = "userName=([a-z0-9A-Z._,&=-]+);";
    int ret, start = -1, len;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, t);
        EXEC(string->get_substring(string, regex, 0, &start, &len));
        string->value[start + len] = '\0';
        dbg_str(DBG_INFO, "sub:%s", &string->value[start]);
        THROW_IF(start == -1, -1);
        THROW_IF(strcmp(&string->value[start], "alan") != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_get_substring(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_string_v2_get_substring_case0());
        EXEC(test_string_v2_get_substring_case1());
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_get_substring);

static int test_string_v2_insert(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "@@@vvvvvvv@@@@";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, "@@@@@@@");

        string->insert(string, 3, "vvvvvvv");
        THROW_IF(strcmp(string->get_cstr(string), test1) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_insert);

static int test_string_v2_split_case0()
{
    //test find and split function
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int i, cnt, expect_count = 19;
    char *p;
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        dbg_str(DBG_DETAIL, "test_split_case0");
        string->assign(string, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
                "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
                "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
                "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
                "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  

        cnt = string->split(string, "&", -1);

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            if (p != NULL) {
                dbg_str(DBG_DETAIL, "%s", p);
            }
        }

        THROW_IF(cnt != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_split_case1()
{
    //test find and split function
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int i, cnt, expect_count = 2, ret = 0;
    char *p;
    char *expect0 = "https";
    char *expect1 = "www.baidu.com/s?ie=utf-8";

    TRY {
        string = object_new(allocator, "String", NULL);
        dbg_str(DBG_DETAIL, "test_split_case1");
        string->assign(string, "https://www.baidu.com/s?ie=utf-8");  

        cnt = string->split(string, "://", 1);

        dbg_str(DBG_DETAIL, "split count=%d", cnt);
        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            if (p != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, p);
            }
        }

        THROW_IF(cnt != expect_count, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 0), expect0) != 0, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 1), expect1) != 0, -1);

    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_split_case2()
{
    //test find and split function
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int i, cnt, expect_count = 2, ret;
    char *p;
    char *expect0 = "http";
    char *expect1 = "mirrors.163.com/debian-archive/debian/tools/src/md5sum-w32_1.1.tar.gz";

    TRY {
        string = object_new(allocator, "String", NULL);
        dbg_str(DBG_DETAIL, "test_split_case2");
        string->assign(string, "http/mirrors.163.com/debian-archive/debian/tools/src/md5sum-w32_1.1.tar.gz");  

        cnt = string->split(string, "/", 1);

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            if (p != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, p);
            }
        }

        THROW_IF(cnt != expect_count, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 0), expect0) != 0, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 1), expect1) != 0, -1);

    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_split_case3_using_reg()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int i, cnt, expect_count = 2;
    char *p;
    char *expect0 = "https://www.";
    char *expect1 = ".com/s?ie=utf-8";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        dbg_str(DBG_DETAIL, "__test_split_using_reg_case1");
        string->assign(string, "https://www.baidu.com/s?ie=utf-8");  

        cnt = string->split(string, "b(.*)du", -1);

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            if (p != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, p);
            }
        }

        THROW_IF(cnt != expect_count, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 0), expect0) != 0, -1);
        THROW_IF(strcmp(string->get_splited_cstr(string, 1), expect1) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_split_case4_using_reg()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int i, cnt, expect_count = 10;
    char *p;
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        dbg_str(DBG_DETAIL, "__test_split_using_reg_case2");
        string->assign(string, "cat dog,desk push last, this is what. must be");  

        cnt = string->split(string, "[, .]", -1);

        for (i = 0; i < cnt; i++) {
            p = string->get_splited_cstr(string, i);
            if (p != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, p);
            }
        }

        THROW_IF(cnt != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_split(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_string_v2_split_case0());
        EXEC(test_string_v2_split_case1());
        EXEC(test_string_v2_split_case2());
        EXEC(test_string_v2_split_case3_using_reg());
        EXEC(test_string_v2_split_case4_using_reg());
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_split);

static int test_string_v2_find(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    int expect_count = 3, count;
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, "rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");

        count = string->find(string, "&", 0, -1);
        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_find);

static int test_string_v2_replace_case0()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "&rsv//_sug1 = 107&rsv_sug2 = 100&rsv_sug3 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107####rsv_sug2 = 100####rsv_sug3 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, test1);
        string->replace(string, "&", "####", 0);

        THROW_IF(strcmp(string->get_cstr(string), test2) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_replace_case1()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    /*
     *char *test2 = "####rsv//_sug1 = 107####rsv_sug7 = 100####rsv_sug2 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
     */
    char *test2 = "####rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, test1);
        string->replace(string, "&", "####", 1);

        THROW_IF(strcmp(string->get_cstr(string), test2) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_replace_case2()
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *test1 = "<1:#abc:2:\"abc\">";
    char *test2 = "<1:0x123:2:\"abc\">";
    char *regex = ":(#[a-z0-9A-Z._-]+):";
    char buffer[32];
    int ret, start, len;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, test1);
        EXEC(string->get_substring(string, regex, 0, &start, &len));
        strncpy(buffer, test1 + start, len);
        string->replace(string, buffer, "0x123", 1);

        THROW_IF(strcmp(string->get_cstr(string), test2) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}

static int test_string_v2_replace(TEST_ENTRY *entry)
{
    int ret;

    TRY {
        EXEC(test_string_v2_replace_case0());
        EXEC(test_string_v2_replace_case1());
        EXEC(test_string_v2_replace_case2());
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_replace);

static int test_string_v2_is_empty(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    String *string;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret, expect_ret = 1;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, test1);
        string->reset(string);
        ret = string->is_empty(string);

        THROW_IF(ret != expect_ret, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_is_empty);

static int test_string_v2_ltrim(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *t = "  hello";
    int ret, expect_ret = 0;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, t);
        string->ltrim(string);

        THROW_IF(strncmp(string->get_cstr(string), t + 2, strlen(t + 2)) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_ltrim);

static int test_string_v2_rtrim(TEST_ENTRY *entry)
{
    String *string;
    allocator_t *allocator = allocator_get_default_instance();
    char *t = "hello  ";
    char *expect = "hello";
    int ret;

    TRY {
        string = object_new(allocator, "String", NULL);
        string->assign(string, t);
        string->rtrim(string);

        THROW_IF(strncmp(string->get_cstr(string), expect, strlen(expect)) != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(string);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_string_v2_rtrim);

