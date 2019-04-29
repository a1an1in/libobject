/**
 * @file string.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/string.h>
#include <libobject/core/vector.h>

static int __modulate_capacity(String *string, int write_len)
{
    if (string->value_max_len == 0) {
        string->value_max_len = 100;
        if (write_len > string->value_max_len) {
            string->value_max_len = write_len;
        }

        string->value = (char *)allocator_mem_alloc(string->obj.allocator, 
                string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING, "string assign alloc error");
            return -1;
        }
    } else if ( 
            string->value_max_len > string->value_len + 1 &&
            string->value_max_len < string->value_len + write_len + 1)
    {
        char *new_buf;

        string->value_max_len = 2 * (string->value_len + write_len + 1);
        new_buf = (char *)allocator_mem_alloc(string->obj.allocator, 
                string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING, "string assign alloc error");
            return -1;
        }

        strncpy(new_buf, string->value, string->value_len);

        allocator_mem_free(string->obj.allocator, string->value);
        string->value = new_buf;
    }

    return 0;
}

static int __construct(String *string, char *init_str)
{
    dbg_str(OBJ_DETAIL, "string construct, string addr:%p", string);

    string->value = (char *)allocator_mem_alloc(string->obj.allocator, 256);
    string->value_max_len = 256;
    return 0;
}

static int __deconstrcut(String *string)
{
    dbg_str(OBJ_DETAIL, "string deconstruct, string addr:%p", string);
    if (string->value)
        allocator_mem_free(string->obj.allocator, string->value);
    if (string->splited_strings != NULL) {
        object_destroy(string->splited_strings);
    }

    return 0;
}

static String *__pre_alloc(String *string, uint32_t size)
{
    dbg_str(OBJ_DETAIL, "pre_alloc, size = %d", size);

    if (size < string->value_max_len) return string;
    else {
        allocator_mem_free(string->obj.allocator, string->value);
        string->value = (char *)allocator_mem_alloc(string->obj.allocator,
                size);
        string->value_max_len = size;
        memset(string->value, 0, size);
    }
    return string;
}

static char *__get_cstr(String * str) 
{
    return str->value;
}

static size_t  __get_len(String *string) 
{
    return string->value_len;
}

static size_t __is_empty(String *string)
{
    return string->get_len(string) ? 0 : 1;
}

static void __clear(String *string)
{   
    string->value[0] = '\0';
    string->value_len = 0;
}

static char __at(String *string, int index)
{
    return string->value[index];
}

static String *__assign(String *string, char *s)
{
    int len = strlen(s);
    int ret;

    ret = __modulate_capacity(string, len);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len  = len;
    string->value[len] = '\0';

    return string;
}

static String *
__assign_fixed_len(String *string, char *s, int len)
{
    int ret;

    ret = __modulate_capacity(string, len);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len  = len;
    string->value[len] = '\0';

    return string;
}

static int __equals(String *string, char *s)
{
    return strcmp(string->value, s) == 0;
}

static String *__append_char(String *string, char c)
{
    int ret;

    ret = __modulate_capacity(string, 1);
    if (ret < 0) {
        dbg_str(DBG_WARNNING, "string buf_auto_modulate have problem, please check");
        return string;
    }

    string->value[string->value_len] = c;
    string->value_len++;
    string->value[string->value_len] = '\0';

    return string;
}

static void __append(String *string, char *sub) 
{   
    int len;
    int ret;
    if (sub == NULL) {
        dbg_str(DBG_WARNNING, "appending-string is null unvalid sub stringing");
        return ;
    }
    len = strlen(sub);
    ret = __modulate_capacity(string, len);

    if (ret < 0 ) {
        return ;
    }

    strncpy(string->value + string->value_len, sub, len);
    string->value_len += len;
    string->value[string->value_len] = '\0';

}

static void __append_string(String *string, String *sub)
{
    char *value = sub->get_cstr(sub);
    string->append(string, value);
}

static String * 
__insert(String *self, int offset, char *cstr)
{
    int dest_len = self->get_len(self);
    int len = strlen(cstr);
    int ret = 0;

    if (cstr == NULL) {
        dbg_str(DBG_ERROR, "insert ctr, but cstr is NULL");
        goto end;
    }

    ret = __modulate_capacity(self, dest_len + len);
    if (ret < 0 ) {
        goto end;
    }

    memmove(self->value + offset + len, 
            self->value + offset, dest_len - offset);
    memmove(self->value + offset, cstr, strlen(cstr));

    self->value_len += len;
    self->value[self->value_len] = '\0';     

end:
    return self;
}

static String *
__insert_string(String *self, int offset, String *src)
{
    return self->insert(self, offset, src->get_cstr(src));
}

static String *
__replace_char(String *string, int index, char c)
{
    string->value[index] = c;

    return string;
}

static String * 
__replace(String *self, char *old, char *newstr)
{
    int start_pos = 0;
    int end_pos   = 0;
    int ret;
    int old_len = strlen(old);
    int new_len = strlen(newstr);
    int str_len = self->get_len(self);

    if (old == NULL || newstr == NULL) {
        return self;
    }

    start_pos = self->find(self, old, start_pos);

    if (start_pos < 0) {
        goto end;
    }

    if (old_len <= new_len) {
        ret = __modulate_capacity(self, new_len-old_len);
        if (ret < 0 ) {
            goto end;
        }
    }

    end_pos = start_pos + new_len;
    memmove(self->value+end_pos, self->value+start_pos+old_len,
            str_len-(start_pos+old_len));
    memmove(self->value+start_pos, newstr, new_len); 
    self->value_len += (new_len-old_len);
    self->value[self->value_len] = '\0';

end:
    return self;
}

static String * 
__replace_all(String *self, char *oldstr, char *newstr)
{
    String *pre = self;
    String *cur = NULL;
    if ( oldstr == NULL || newstr == NULL ) {
        return self;
    }
    while (cur == NULL || strcmp(cur->get_cstr(cur), pre->get_cstr(pre)) != 0 ) {
        if (cur != NULL ) { 
            object_destroy(cur);
            cur = NULL;
        }
        cur = OBJECT_NEW(self->obj.allocator, String, NULL);
        cur->assign(cur, pre->get_cstr(pre));
        pre = pre->replace(pre, oldstr, newstr);
    }
    object_destroy(cur);

    return self;
}

static void __toupper_(String *string)
{
    int size = string->value_len;
    int i;

    for (i = 0; i < size; i++) {
        if (islower(string->value[i])) {
            //toupper(string->value[i]);
            string->value[i] += 'A'-'a';
        }
    }       
}

static void __tolower_(String *string)
{
    int size = string->value_len;
    int i;

    for (i = 0; i < size; i++) {
        if (isupper(string->value[i])) {
            //tolower(string->value[i]);
            string->value[i] += 'a'-'A';
        }
    } 
}

static void __ltrim(String *string)
{
    int size = string->value_len;
    int i, cnt = 0;

    for (i = 0; i < size; i++) {
        if (isspace(string->value[i])) {
            cnt++;
        } else {
            break;
        }   
    }
    if (cnt) {
        string->value_len -= cnt;
        memmove(string->value, string->value + cnt, 
                string->value_len);
        string->value[string->value_len] = '\0';
    }
}

static void __rtrim(String *string)
{
    int size = string->value_len;
    int i;

    for (i = size - 1; i >= 0; i--) {
        if (isspace(string->value[i])) {
            string->value[i] = '\0';
        } else {
            break;
        }   
    }
}

static void __trim(String *string)
{ 
    if (NULL != string) {
        __ltrim(string);
        __rtrim(string);
    }
}

static String *__get_substring(String  *string, int pos, int len)
{
    int size = string->value_len;
    int i;
    String *str;

    str = OBJECT_NEW(string->obj.allocator, String, NULL);

    assert(pos <= size);

    for (i = pos;i < size && len; i++) {
        str->append_char(str, string->value[i]);
        len--;
    }

    return str;
}

static int __find(String *string, char *key, int pos)
{   
    int len1 = string->value_len;
    int len2 = strlen(key);
    char *p  = NULL;

    if (NULL == string || NULL == key || pos < 0 || len1 < len2) {
        return -1;//exception happened
    }
    
    p = strstr(string->value + pos, key);
    if(NULL !=  p) {
        return p - (string->value);
    }

    return -1;
}

static int __split_string(String *string, char *delims)
{
    int cnt = 0;
    char *ptr = NULL;
    char *p;
    Vector *v;

    if (string->splited_strings == NULL) {
        v = OBJECT_NEW(string->obj.allocator, Vector, NULL);
        string->splited_strings = v;
    } else {
        v = string->splited_strings;
    }

    for (   p = strtok_r(string->get_cstr(string), delims, &ptr);
            p;
            p = strtok_r(NULL, delims, &ptr)) 
    {
        /*
         *dbg_str(DBG_SUC, "addr:%p, slim :%s", p, p);
         */
        if (p != NULL) {
            *(p -1) = '\0';
        }
        cnt++;
        v->add_back(v, p);
    }

    return cnt;
}

static char *__get_splited_string(String *string, int index)
{
    Vector *v = string->splited_strings;
    char *d;

    if (v == NULL) {
        return NULL;
    }
    
    v->peek_at(v, index, (void **)&d);

    return d;
}

static class_info_entry_t string_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj), 
    Init_Nfunc_Entry(1 , String, construct, __construct), 
    Init_Nfunc_Entry(2 , String, deconstruct, __deconstrcut), 
    Init_Vfunc_Entry(3 , String, set, NULL), 
    Init_Vfunc_Entry(4 , String, get, NULL), 
    Init_Vfunc_Entry(5 , String, at, __at), 
    Init_Vfunc_Entry(6 , String, get_cstr, __get_cstr), 
    Init_Vfunc_Entry(7 , String, get_len, __get_len), 
    Init_Vfunc_Entry(8 , String, clear, __clear), 
    Init_Vfunc_Entry(9 , String, pre_alloc, __pre_alloc), 
    Init_Vfunc_Entry(10, String, assign, __assign), 
    Init_Vfunc_Entry(11, String, assign_fixed_len, __assign_fixed_len), 
    Init_Vfunc_Entry(12, String, equals, __equals), 
    Init_Vfunc_Entry(13, String, replace_char, __replace_char), 
    Init_Vfunc_Entry(14, String, replace, __replace), 
    Init_Vfunc_Entry(15, String, replace_all, __replace_all), 
    Init_Vfunc_Entry(16, String, append_char, __append_char), 
    Init_Vfunc_Entry(17, String, append, __append), 
    Init_Vfunc_Entry(18, String, append_string, __append_string), 
    Init_Vfunc_Entry(19, String, insert, __insert), 
    Init_Vfunc_Entry(20, String, insert_string, __insert_string), 
    Init_Vfunc_Entry(21, String, toupper, __toupper_), 
    Init_Vfunc_Entry(22, String, tolower, __tolower_), 
    Init_Vfunc_Entry(23, String, ltrim, __ltrim), 
    Init_Vfunc_Entry(24, String, rtrim, __rtrim), 
    Init_Vfunc_Entry(25, String, trim, __trim), 
    Init_Vfunc_Entry(26, String, get_substring, __get_substring), 
    Init_Vfunc_Entry(27, String, find, __find), 
    Init_Vfunc_Entry(28, String, is_empty, __is_empty), 
    Init_Vfunc_Entry(29, String, split_string, __split_string), 
    Init_Vfunc_Entry(30, String, get_splited_string, __get_splited_string), 
    Init_Str___Entry(31, String, name, NULL), 
    Init_Str___Entry(32, String, value, NULL), 
    Init_End___Entry(33, String), 
};
REGISTER_CLASS("String", string_class_info);

static int test_string_get_cstr() 
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *parent;
    char *test = "abcdefg";
    int ret;

    parent = OBJECT_NEW(allocator, String, NULL);
    parent->assign(parent, test);  

    if (strcmp(parent->get_cstr(parent), test) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(parent);

    return ret;
}

static int test_string_append()
{  
    allocator_t *allocator = allocator_get_default_alloc();
    String *parent;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    sprintf(test3, "%s%s", test1, test2);

    parent = OBJECT_NEW(allocator, String, NULL);
    parent->assign(parent, test1);  

    parent->append(parent, test2);

    if (strcmp(parent->get_cstr(parent), test3) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(parent);

    return ret;
}

static int test_string_append_string()
{ 
    allocator_t *allocator = allocator_get_default_alloc();
    String *parent, *substring;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    char test3[1024];
    int ret;

    sprintf(test3, "%s%s", test1, test2);

    parent = OBJECT_NEW(allocator, String, NULL);
    parent->assign(parent, test1);  

    substring = OBJECT_NEW(allocator, String, NULL);
    substring->assign(substring, test2);

    parent->append_string(parent, substring);
    if (strcmp(parent->get_cstr(parent), test3) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(parent);
    object_destroy(substring);

    return ret;
}

static int test_string_len()
{  
    allocator_t *allocator = allocator_get_default_alloc();
    String *parent, *substring;
    char *test1 = "abcdefg";
    char *test2 = "hello world";
    int ret;

    parent = OBJECT_NEW(allocator, String, NULL);
    parent->assign(parent, test1);  

    substring = OBJECT_NEW(allocator, String, NULL);
    substring->assign(substring, test2);

    parent->append_string(parent, substring);
    
    if (parent->get_len(parent) != (strlen(test1) + strlen(test2))) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(parent);
    object_destroy(substring);

    return ret;

}

int test_string_get_substring()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *sub_str;
    int ret;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";

    string = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    sub_str = string->get_substring(string, 3, 10);
    
    if (strcmp(sub_str->get_cstr(sub_str), "v//_sug1 =") == 0) {
        ret = 1;
    } else { 
        ret = 0;
    }

    object_destroy(string);
    object_destroy(sub_str);

    return ret;   
}

static int test_string_insert()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);
    string->assign(string, "@@@@@@@");

    string->insert(string, 3, "vvvvvvv");
    if (strcmp(string->get_cstr(string), "@@@vvvvvvv@@@@") == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(string);

    return ret;
}

static int test_string_insert_string()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *sub_str;
    int ret;
    char *test = "";

    string = OBJECT_NEW(allocator, String, NULL);
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

    object_destroy(string);
    object_destroy(sub_str);

    return ret;
}

int test_string_split()
{    

    allocator_t *allocator = allocator_get_default_alloc();
    //test find and split_string function
    String *str;
    Vector *vector;
    int ret = 0;
    int i, cnt;
    char *p;

    str = OBJECT_NEW(allocator, String, NULL);

    str->assign(str, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
            "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
            "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
            "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
            "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  

    cnt = str->split_string(str, "&");

    for (i = 0; i < cnt; i++) {
        p = str->get_splited_string(str, i);
        if (p != NULL) {
            dbg_str(DBG_DETAIL, "%s", p);
        }
    }

    if (cnt != 19) {
        ret = 0;
    } else {
        ret = 1;
    }

    object_destroy(str);

    return ret;

}

int test_string_find()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    int ret = 0;

    string = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, "rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");

    int pos = string->find(string, "&", 0);

    ret = assert_int_equal(pos, 16);
    dbg_str(DBG_DETAIL, "substring position: %d ", pos);

    object_destroy(string);

    return ret;

}

static int test_string_replace()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    string->replace(string, "&", "####");

    if (strcmp(string->get_cstr(string), test2) == 0){
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(string);

    return ret;
}

static int test_string_replace_all()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    char *test2 = "####rsv//_sug1 = 107####rsv_sug7 = 100####rsv_sug2 = 0####prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    string->replace_all(string, "&", "####");

    if (strcmp(string->get_cstr(string), test2) == 0){
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(string);

    return ret;
}

static int test_string_empty()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    string->clear(string);
    ret = string->is_empty(string);

    object_destroy(string);

    return ret;
}

static int test_string_ltrim()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    char *test = "  hello";
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);
    string->assign(string, test);

    string->ltrim(string);

    if((strcmp(string->get_cstr(string), test + 2) == 0)) {
        ret = 1;
    } else { 
        ret = 0;
    }

    object_destroy(string);

    return ret;
}

static int test_string_rtrim()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string;
    char *test = "hello  ";
    char *expect = "hello";
    int ret;

    string = OBJECT_NEW(allocator, String, NULL);
    string->assign(string, test);

    string->rtrim(string);

    if((strcmp(string->get_cstr(string), expect) == 0)) {
        ret = 1;
    } else { 
        ret = 0;
    }

    object_destroy(string);

    return ret;
}

REGISTER_TEST_FUNC(test_string_get_cstr);
REGISTER_TEST_FUNC(test_string_append);
REGISTER_TEST_FUNC(test_string_append_string);
REGISTER_TEST_FUNC(test_string_len);
REGISTER_TEST_FUNC(test_string_find);
REGISTER_TEST_FUNC(test_string_get_substring);
REGISTER_TEST_FUNC(test_string_empty);
REGISTER_TEST_FUNC(test_string_replace);
REGISTER_TEST_FUNC(test_string_replace_all);
REGISTER_TEST_FUNC(test_string_insert);
REGISTER_TEST_FUNC(test_string_insert_string);
REGISTER_TEST_FUNC(test_string_split);
REGISTER_TEST_FUNC(test_string_ltrim);
REGISTER_TEST_FUNC(test_string_rtrim);
