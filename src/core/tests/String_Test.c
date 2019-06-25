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
#include <libobject/core/tests/String_Test.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>

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

    return 0;
}

static int __teardown(String_Test *test)
{
    dbg_str(DBG_DETAIL,"String_Test teardown");
    test->str->clear(test->str);

    return 0;
}

static int __test_get_cstr(String_Test *test)
{
    String *str = test->str;
    char *demo = "abcdefg";
    int ret;

    Init_Test_Case(test);
    str->assign(str, demo);  

    if (strcmp(str->get_cstr(str), demo) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

static int __test_append(String_Test *test)
{
    String *parent = test->str;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    Init_Test_Case(test);
    sprintf(test3, "%s%s", test1, test2);

    parent->assign(parent, test1);  
    parent->append(parent, test2);

    if (strcmp(parent->get_cstr(parent), test3) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

static int __test_append_string(String_Test *test)
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *substring;
    String *parent = test->str;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    Init_Test_Case(test);
    sprintf(test3, "%s%s", test1, test2);

    parent->assign(parent, test1);  

    substring = OBJECT_NEW(allocator, String, NULL);
    substring->assign(substring, test2);

    parent->append_string(parent, substring);
    if (strcmp(parent->get_cstr(parent), test3) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(substring);

    return ret;
}

static int __test_len(String_Test *test)
{
    String *parent = test->str;
    char *test1 = "abcdefg";
    int ret;

    Init_Test_Case(test);
    parent->assign(parent, test1);  
    
    if (parent->get_len(parent) != strlen(test1)) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_get_substring(String_Test *test)
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string = test->str, *sub_str;
    int ret;
    char *t = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";

    Init_Test_Case(test);
    string->assign(string, t);
    sub_str = string->get_substring(string, 3, 10);
    
    if (strcmp(sub_str->get_cstr(sub_str), "v//_sug1 =") == 0) {
        ret = 1;
    } else { 
        ret = 0;
    }

    object_destroy(sub_str);

    return ret;
}

static int __test_insert(String_Test *test)
{
    String *string = test->str;
    int ret;

    Init_Test_Case(test);
    string->assign(string, "@@@@@@@");

    string->insert(string, 3, "vvvvvvv");
    if (strcmp(string->get_cstr(string), "@@@vvvvvvv@@@@") == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

static int __test_insert_string(String_Test *test)
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string = test->str, *sub_str;
    int ret;

    Init_Test_Case(test);
    string->assign(string, "@@@@@@@");

    sub_str = OBJECT_NEW(allocator, String, NULL);
    sub_str->assign(sub_str, "vvvvvvv");

    string->insert_string(string, 3, sub_str);
    if (strcmp(string->get_cstr(string), "@@@vvvvvvv@@@@") == 0) {
        ret = 1;
    } else {
        dbg_str(DBG_ERROR, "%s", string->get_cstr(string));
        ret = 0;
    }

    object_destroy(sub_str);

    return ret;
}

static int __test_split(String_Test *test)
{
    //test find and split function
    String *str = test->str;
    int ret = 0;
    int i, cnt;
    char *p;

    Init_Test_Case(test);
    str->assign(str, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
            "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
            "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
            "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
            "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  

    cnt = str->split(str, "&");

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_cstr(str, i);
        if (p != NULL) {
            dbg_str(DBG_DETAIL, "%s", p);
        }
    }

    if (cnt != 19) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

static int __test_find(String_Test *test)
{
    String *string = test->str;
    int ret = 0;

    Init_Test_Case(test);
    string->assign(string, "rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");

    int pos = string->find(string, "&", 0);

    ret = assert_int_equal(pos, 16);
    dbg_str(DBG_DETAIL, "substring position: %d ", pos);

    return ret;

}

static int __test_replace(String_Test *test)
{
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    Init_Test_Case(test);
    string->assign(string, test1);
    string->replace(string, "&", "####");

    if (strcmp(string->get_cstr(string), test2) == 0){
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

static int __test_replace_all(String_Test *test)
{
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107####rsv_sug7 = 100####rsv_sug2 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    Init_Test_Case(test);
    string->assign(string, test1);
    string->replace_all(string, "&", "####");

    if (strcmp(string->get_cstr(string), test2) == 0){
        ret = 1;
    } else {
        ret = 0;
    }

    return ret;
}

static int __test_empty(String_Test *test)
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string = test->str;
    char *test1 = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    Init_Test_Case(test);
    string->assign(string, test1);
    string->clear(string);
    ret = string->is_empty(string);

    return ret;
}

static int __test_ltrim(String_Test *test)
{
    String *string = test->str;
    char *t = "  hello";
    int ret;

    Init_Test_Case(test);
    string->assign(string, t);

    string->ltrim(string);

    if((strcmp(string->get_cstr(string), t + 2) == 0)) {
        ret = 1;
    } else { 
        ret = 0;
    }

    return ret;
}

static int __test_rtrim(String_Test *test)
{
    String *string = test->str;
    char *t = "hello  ";
    char *expect = "hello";
    int ret;

    Init_Test_Case(test);
    string->assign(string, t);

    string->rtrim(string);

    if((strcmp(string->get_cstr(string), expect) == 0)) {
        ret = 1;
    } else { 
        ret = 0;
    }

    return ret;
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
    Init_Vfunc_Entry(8 , String_Test, test_append, __test_append),
    Init_Vfunc_Entry(9 , String_Test, test_append_string, __test_append_string),
    Init_Vfunc_Entry(10, String_Test, test_len, __test_len),
    Init_Vfunc_Entry(11, String_Test, test_get_substring, __test_get_substring),
    Init_Vfunc_Entry(12, String_Test, test_insert, __test_insert),
    Init_Vfunc_Entry(13, String_Test, test_insert_string, __test_insert_string),
    Init_Vfunc_Entry(14, String_Test, test_split, __test_split),
    Init_Vfunc_Entry(15, String_Test, test_find, __test_find),
    Init_Vfunc_Entry(16, String_Test, test_replace, __test_replace),
    Init_Vfunc_Entry(17, String_Test, test_replace_all, __test_replace_all),
    Init_Vfunc_Entry(18, String_Test, test_empty, __test_empty),
    Init_Vfunc_Entry(19, String_Test, test_replace_all, __test_replace_all),
    Init_Vfunc_Entry(20, String_Test, test_ltrim, __test_ltrim),
    Init_Vfunc_Entry(21, String_Test, test_rtrim, __test_rtrim),
    Init_End___Entry(22, String_Test),
};
REGISTER_CLASS("String_Test", string_test_class_info);
