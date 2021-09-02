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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "String_Test.h"

static int __construct(Test *test, char *init_str)
{
    String_Test *t = (String_Test *)test;
    allocator_t *allocator = test->obj.allocator;

    dbg_str(DBG_DETAIL,"String_Test construct");

    t->str = object_new(allocator, "String", NULL);
    return 0;
}

static int __deconstrcut(Test *test)
{
    String_Test *t = (String_Test *)test;

    dbg_str(DBG_DETAIL,"String_Test deconstruct");
    if (t->str != NULL) {
        object_destroy(t->str);
    }

    return 0;
}

static int __setup(String_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;

    dbg_str(DBG_DETAIL,"String_Test set up");

    return 1;
}

static int __teardown(String_Test *test)
{
    dbg_str(DBG_DETAIL,"String_Test teardown");
    test->str->reset(test->str);

    return 1;
}

static int __test_get_cstr(String_Test *test)
{
    String *string = test->str;
    char *demo = "abcdefg";

    string->assign(string, demo);  

    ASSERT_EQUAL(test, string->get_cstr(string), demo, strlen(demo));
}

static int __test_sprintf(String_Test *test)
{
    String *string = test->str;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];

    sprintf(test3, "%s, %s", test1, test2);

    string->reset(string);  
    string->format(string, 1024, "%s, %s", test1, test2);

    ASSERT_EQUAL(test, string->get_cstr(string), test3, strlen(test3));
}

static int __test_append(String_Test *test)
{
    String *parent = test->str;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];

    sprintf(test3, "%s%s", test1, test2);

    parent->assign(parent, test1);  
    parent->append(parent, test2, -1);

    ASSERT_EQUAL(test, parent->get_cstr(parent), test3, strlen(test3));
}

static int __test_len(String_Test *test)
{
    String *parent = test->str;
    char *test1 = "abcdefg";

    parent->assign(parent, test1);  
    
    ASSERT_EQUAL(test, parent->get_cstr(parent), test1, strlen(test1));
}

static int __test_get_substring(String_Test *test)
{
    String *string = test->str;
    char *t = "Content-Disposition: filename=\"u=1573948710,2851629614&fm=26&fmt=auto&gp=0.webp\"";
    char *regex = "filename=\"([a-z0-9A-Z\.,&=]+)\"";
    int ret, start = 0, len;

    TRY {
        string->assign(string, t);
        string->get_substring(string, regex, 0, &start, &len);
        THROW_IF(start != 31, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    }

    return ret;
}

static int __test_insert(String_Test *test)
{
    String *string = test->str;
    char *test1 = "@@@vvvvvvv@@@@";

    string->assign(string, "@@@@@@@");

    string->insert(string, 3, "vvvvvvv");
    ASSERT_EQUAL(test, string->get_cstr(string), test1, strlen(test1));
}

static int __test_split_case0(String_Test *test)
{
    //test find and split function
    String *str = test->str;
    int i, cnt, expect_count = 19;
    char *p;

    dbg_str(DBG_DETAIL, "test_split_case0");
    str->assign(str, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
            "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
            "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
            "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
            "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  

    cnt = str->split(str, "&", -1);

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_DETAIL, "%s", p);
        }
    }

    ASSERT_EQUAL(test, &cnt, &expect_count, sizeof(expect_count));
}

static int __test_split_case1(String_Test *test)
{
    //test find and split function
    String *str = test->str;
    int i, cnt, expect_count = 2, ret = 0;
    char *p;
    char *expect0 = "https";
    char *expect1 = "www.baidu.com/s?ie=utf-8";

    dbg_str(DBG_DETAIL, "test_split_case1");
    str->assign(str, "https://www.baidu.com/s?ie=utf-8");  

    cnt = str->split(str, "://", 1);

    dbg_str(DBG_DETAIL, "split count=%d", cnt);
    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_SUC, "%d:%s", i, p);
        }
    }

    ret = ASSERT_EQUAL(test, &cnt, &expect_count, sizeof(expect_count));
    if (ret != 1) {
        return ret;
    }
    ret = ASSERT_EQUAL(test, str->get_splited_cstr(str, 0), expect0, strlen(expect0));
    if (ret != 1) {
        return ret;
    }
    ret = ASSERT_EQUAL(test, str->get_splited_cstr(str, 1), expect1, strlen(expect1));
    if (ret != 1) {
        return ret;
    }
}

static int __test_split_case2(String_Test *test)
{
    //test find and split function
    String *str = test->str;
    int i, cnt, expect_count = 2, ret;
    char *p;
    char *expect0 = "http";
    char *expect1 = "mirrors.163.com/debian-archive/debian/tools/src/md5sum-w32_1.1.tar.gz";

    dbg_str(DBG_DETAIL, "test_split_case2");
    str->assign(str, "http/mirrors.163.com/debian-archive/debian/tools/src/md5sum-w32_1.1.tar.gz");  

    cnt = str->split(str, "/", 1);

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_SUC, "%d:%s", i, p);
        }
    }

    ASSERT_EQUAL(test, &cnt, &expect_count, sizeof(expect_count));
    ASSERT_EQUAL(test, str->get_splited_cstr(str, 0), expect0, strlen(expect0));
    ASSERT_EQUAL(test, str->get_splited_cstr(str, 1), expect1, strlen(expect1));
}

static int __test_split_using_reg_case1(String_Test *test)
{
    String *str = test->str;
    int i, cnt, expect_count = 2;
    char *p;
    char *expect0 = "https://www.";
    char *expect1 = ".com/s?ie=utf-8";
    int ret;

    dbg_str(DBG_DETAIL, "__test_split_using_reg_case1");
    str->assign(str, "https://www.baidu.com/s?ie=utf-8");  

    cnt = str->split(str, "b(.*)du", -1);

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_SUC, "%d:%s", i, p);
        }
    }

    ASSERT_EQUAL(test, &cnt, &expect_count, sizeof(expect_count));
    ASSERT_EQUAL(test, str->get_splited_cstr(str, 0), expect0, strlen(expect0));
    ASSERT_EQUAL(test, str->get_splited_cstr(str, 1), expect1, strlen(expect1));
}
static int __test_split_using_reg_case2(String_Test *test)
{
    String *str = test->str;
    int i, cnt, expect_count = 10;
    char *p;
    int ret;

    dbg_str(DBG_DETAIL, "__test_split_using_reg_case2");
    str->assign(str, "cat dog,desk push last, this is what. must be");  

    cnt = str->split(str, "[, .]", -1);

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_SUC, "%d:%s", i, p);
        }
    }

    ret = ASSERT_EQUAL(test, &cnt, &expect_count, sizeof(expect_count));
    if (ret != 1) {
        dbg_str(DBG_ERROR, "expect_count = %d, count=%d", expect_count, cnt);
    }

    return ret;
}

static int __test_split(String_Test *test)
{
    int ret;

    ret = __test_split_case0(test);
    if (ret != 1) return ret;
    ret = __test_split_case1(test);
    if (ret != 1) return ret;
    ret = __test_split_case2(test);
    if (ret != 1) return ret;
    ret = __test_split_using_reg_case1(test);
    if (ret != 1) return ret;
    ret =__test_split_using_reg_case2(test);

    return ret;
}


static int __test_find(String_Test *test)
{
    String *string = test->str;
    int expect_count = 3, count;

    string->assign(string, "rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");

    count = string->find(string, "&", 0, -1);
    ASSERT_EQUAL(test, &count, &expect_count, sizeof(count));
}

static int __test_replace_case0(String_Test *test)
{
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug2 = 100&rsv_sug3 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107####rsv_sug2 = 100####rsv_sug3 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    string->assign(string, test1);
    string->replace(string, "&", "####", 0);

    ret = ASSERT_EQUAL(test, string->get_cstr(string), test2, strlen(test2));
    if (ret != 1) {
        dbg_str(DBG_ERROR, "expect:%s, real:%s",  test2, string->get_cstr(string));
    }

    return ret;
}

static int __test_replace_case1(String_Test *test)
{
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    /*
     *char *test2 = "####rsv//_sug1 = 107####rsv_sug7 = 100####rsv_sug2 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
     */
    char *test2 = "####rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    string->assign(string, test1);
    string->replace(string, "&", "####", 1);

    ret = ASSERT_EQUAL(test, string->get_cstr(string), test2, strlen(test2));
    if (ret != 1) {
        dbg_str(DBG_ERROR, "expect:%s, real:%s",  test2, string->get_cstr(string));
    }

    return ret;
}

static int __test_replace(String_Test *test)
{
    int ret;

    ret = __test_replace_case0(test);
    if (ret != 1) return ret;
    ret = __test_replace_case1(test);

    return ret;
}

static int __test_empty(String_Test *test)
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret, expect_ret = 1;

    string->assign(string, test1);
    string->reset(string);
    ret = string->is_empty(string);

    ret = ASSERT_EQUAL(test, &ret, &expect_ret, sizeof(ret));
}

static int __test_ltrim(String_Test *test)
{
    String *string = test->str;
    char *t = "  hello";
    int ret, expect_ret = 0;

    string->assign(string, t);
    string->ltrim(string);

    ret = ASSERT_EQUAL(test, string->get_cstr(string), t + 2, strlen(t + 2));
}

static int __test_rtrim(String_Test *test)
{
    String *string = test->str;
    char *t = "hello  ";
    char *expect = "hello";
    int ret;

    string->assign(string, t);
    string->rtrim(string);

    ret = ASSERT_EQUAL(test, string->get_cstr(string), expect, strlen(expect));

}

static class_info_entry_t string_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , String_Test, construct, __construct),
    Init_Nfunc_Entry(2 , String_Test, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , String_Test, set, NULL),
    Init_Vfunc_Entry(4 , String_Test, get, NULL),
    Init_Vfunc_Entry(5 , String_Test, setup, __setup),
    Init_Vfunc_Entry(6 , String_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , String_Test, test_get_cstr, __test_get_cstr),
    Init_Vfunc_Entry(8 , String_Test, test_sprintf, __test_sprintf),
    Init_Vfunc_Entry(9 , String_Test, test_append, __test_append),
    Init_Vfunc_Entry(10, String_Test, test_len, __test_len),
    Init_Vfunc_Entry(11, String_Test, test_get_substring, __test_get_substring),
    Init_Vfunc_Entry(12, String_Test, test_insert, __test_insert),
    Init_Vfunc_Entry(13, String_Test, test_split, __test_split),
    Init_Vfunc_Entry(14, String_Test, test_find, __test_find),
    Init_Vfunc_Entry(15, String_Test, test_replace, __test_replace),
    Init_Vfunc_Entry(16, String_Test, test_empty, __test_empty),
    Init_Vfunc_Entry(17, String_Test, test_ltrim, __test_ltrim),
    Init_Vfunc_Entry(18, String_Test, test_rtrim, __test_rtrim),
    Init_End___Entry(19, String_Test),
};
REGISTER_CLASS("String_Test", string_test_class_info);
