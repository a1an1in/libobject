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

static int string_buf_auto_modulate(String *string, int write_len)
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
    } else if ( string->value_max_len > string->value_len + 1 &&
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
    //maxsize 256
    string->value = (char *)allocator_mem_alloc(string->obj.allocator, 256);
    string->value_max_len = 256;
    return 0;
}

static int __deconstrcut(String *string)
{
    dbg_str(OBJ_DETAIL, "string deconstruct, string addr:%p", string);
    if (string->value)
        allocator_mem_free(string->obj.allocator, string->value);

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

static String *__assign(String *string, char *s)
{
    int len = strlen(s);
    int ret;

    ret = string_buf_auto_modulate(string, len);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len  = len;
    string->value[len] = '\0';

    return string;
}

static String *__assign_char(String *string, char c, size_t count)
{
    int i = 0;
    int ret;
    char * ptmp = (char *)allocator_mem_alloc(string->obj.allocator, count);
    for (i = 0 ; i < count; i++) {
        *(ptmp+i) = c;
    }
    *(ptmp + i) = '\0';
    string->assign(string, ptmp);
    allocator_mem_free(string->obj.allocator, ptmp);
    ptmp = NULL;
    return string;
}

static String *__append_char(String *string, char c)
{
    int ret;

    ret = string_buf_auto_modulate(string, 1);
    if (ret < 0) {
        dbg_str(DBG_WARNNING, "string buf_auto_modulate have problem, please check");
        return string;
    }

    string->value[string->value_len] = c;
    string->value_len++;
    string->value[string->value_len] = '\0';

    return string;
}

static String *__replace_char(String *string, int index, char c)
{
    string->value[index] = c;

    return string;
}

static char __at(String *string, int index)
{
    return string->value[index];
}

static void __toupper_impact(String *string)
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

static void  __toupper_(String *string, String *str)
{
    str->assign(str, string->value);
    __toupper_impact(str);  
}

static void __tolower_impact(String *string)
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

static void __tolower_(String *string, String *str)
{
    str->assign(str, string->value);
    __tolower_impact(str);
}

static void __ltrim(String *string)
{
    int size = string->value_len;
    int i;

    for (i = 0; i < size; i++) {
        if (isspace(string->value[i])) {
            string->value++;
        } else {
            break;
        }   

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

static int __find(String *string, String *substr, int pos)
{   
    int len1 = string->value_len;
    int len2 = substr->value_len;
    char *p  = NULL;

    if (NULL == string || NULL == substr || pos < 0 || len1 < len2) {
        return -1;//exception happened
    }
    //a simple method
    p = strstr(string->value + pos, substr->value);
    if(NULL !=  p) {
        return p - (string->value);
    }
    return -1;
}

static String *__substr(String  *string, int pos, int len)
{
    int size = string->value_len;
    int i;
    String *str;

    str = OBJECT_NEW(string->obj.allocator, String, NULL);
    assert(pos <=  size);
    for ( i  = pos;i < size && len ;i++) {
        str->append_char(str, string->value[i]);
        len--;
    }
    return str;
}

static void 
__split_string(String *string, String *separator, Vector *vector)
{
    int start_pos = 0, pos;
    String *pstr = NULL;
    int i = 0;

    if (NULL == string|| NULL == separator) {
        vector->add_back(vector, (void *)string);
        return;
    }

    while(1) {
        pos = __find(string, separator, start_pos);
        if (pos == start_pos) {
            start_pos += separator->value_len;
            continue;
        }
        if (pos < 0) {   
            pstr = __substr(string, start_pos, string->value_len); 
            vector->add_at(vector, i, pstr);       
            break;
        } else {
            pstr = __substr(string, start_pos, pos-start_pos);
            vector->add_at(vector, i, pstr);
            start_pos = pos + separator->value_len;
        }
        i++;     
    }
}

static char *__c_str(String * str) 
{
    return str->value;
}

static void __append_str(String *string, char *sub) 
{   
    int len;
    int ret;
    if (sub == NULL) {
        dbg_str(DBG_WARNNING, "appending-string is null unvalid substring");
        return ;
    }
    len = strlen(sub);
    ret = string_buf_auto_modulate(string, len);

    if (ret < 0 ) {
        return ;
    }
   
    strncpy(string->value + string->value_len, sub, len);
    string->value_len += len;
    string->value[string->value_len] = '\0';

}

static void __append_str_len(String *string, char *sub, int len)
{
    int ret;
    if (sub == NULL) {
        dbg_str(DBG_WARNNING, "appending-string is null unvalid substring");
        return ;
    }

    len = strlen(sub) > len ? len:strlen(sub);
    ret = string_buf_auto_modulate(string, len);
    if (ret < 0 ) {
        return ;
    }
    strncpy(string->value+string->value_len, sub, len);
    string->value_len += len;
    string->value[string->value_len] = '\0';

}

static void __append_objective_string(String *string, String *sub)
{
    char *value = sub->c_str(sub);
    string->append_str(string, value);
}


static size_t  __size(String *string) 
{
    return string->value_len;
}

static size_t __is_empty(String *string)
{
    return string->size(string) ? 0 : 1;
}

static void __clear(String *string)
{   
    string->value[0] = '\0';
    string->value_len = 0;
}

static String * 
__replace(String *self, char *old, char *newstr)
{
    int start_pos = 0;
    int end_pos   = 0;
    int ret;
    int old_len = strlen(old);
    int new_len = strlen(newstr);
    int str_len = self->size(self);

    if (old == NULL || newstr == NULL) {
        return self;
    }

    String *old_str = OBJECT_NEW(self->obj.allocator, String, NULL);
    old_str->assign(old_str, old);
    start_pos = self->find(self, old_str, start_pos);

    if (start_pos < 0) {
        goto end;
    }

    if (old_len <= new_len) {
        ret = string_buf_auto_modulate(self, new_len-old_len);
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
    object_destroy(old_str);
    return self;
}

static String * __replace_all(String *self, char *oldstr, char *newstr)
{
    String *pre = self;
    String *cur = NULL;
    if ( oldstr == NULL || newstr == NULL ) {
        return self;
    }
    while (cur == NULL || strcmp(cur->c_str(cur), pre->c_str(pre)) != 0 ) {
        if (cur != NULL ) { 
            object_destroy(cur);
            cur = NULL;
        }
        cur = OBJECT_NEW(self->obj.allocator, String, NULL);
        cur->assign(cur, pre->c_str(pre));
        pre = pre->replace(pre, oldstr, newstr);
    }
    object_destroy(cur);

    return self;
}

static String * 
__insert_char_count(String *self, size_t pos, char c, size_t count)
{
    String * ptem = OBJECT_NEW(self->obj.allocator, String, NULL);
    ptem->assign_char(ptem, c, count);
    self->insert_str_normal(self, pos, ptem);
    object_destroy(ptem);
}

static String * 
__insert_cstr(String *self, size_t index, 
              char *src, size_t pos, size_t len)
{
    int size = strlen(src);
    int dest_size = self->size(self);
    int ret = 0;
    if ( src == NULL || pos > (size - 1) ) {
        dbg_str(DBG_ERROR, "src == NULL or pos data is unvalid pos %d src size:%d", pos, size);
        goto end;
    }

    if (pos + len > size) {
        len = size - pos;
    }
     
    ret = string_buf_auto_modulate(self, len);
    if (ret < 0 ) {
            goto end;
    }
    
    memmove(self->value+index+len, self->value+index, dest_size-index);
    memmove(self->value+index, src+pos, len);
    self->value_len += len;
    self->value[self->value_len] = '\0';     
end:
    return  self;
}

static String *
__insert_str(String *self, size_t index, 
             String *src, size_t pos, size_t len)
{
    return self->insert_cstr(self, index, src->c_str(src), pos, len);
}

static String * 
__insert_str_normal(String * self, size_t index, String *cstr)
{
    return self->insert_str(self, index, cstr, 0, cstr->size(cstr));
}

static String * 
__insert_cstr_normal(String * self, size_t index, char *cstr)
{
    return self->insert_cstr(self, index, cstr, 0, strlen(cstr));
}

static class_info_entry_t string_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj), 
    Init_Nfunc_Entry(1 , String, construct, __construct), 
    Init_Nfunc_Entry(2 , String, deconstruct, __deconstrcut), 
    Init_Vfunc_Entry(3 , String, set, NULL), 
    Init_Vfunc_Entry(4 , String, get, NULL), 
    Init_Vfunc_Entry(5 , String, pre_alloc, __pre_alloc), 
    Init_Vfunc_Entry(6 , String, assign, __assign), 
    Init_Vfunc_Entry(7 , String, replace_char, __replace_char), 
    Init_Vfunc_Entry(8 , String, append_char, __append_char), 
    Init_Vfunc_Entry(9 , String, at, __at), 
    Init_Vfunc_Entry(10, String, toupper, __toupper_), 
    Init_Vfunc_Entry(11, String, toupper_impact, __toupper_impact), 
    Init_Vfunc_Entry(12, String, tolower_impact, __tolower_impact), 
    Init_Vfunc_Entry(13, String, tolower, __tolower_), 
    Init_Vfunc_Entry(14, String, ltrim, __ltrim), 
    Init_Vfunc_Entry(15, String, rtrim, __rtrim), 
    Init_Vfunc_Entry(16, String, trim, __trim), 
    Init_Vfunc_Entry(17, String, split_string, __split_string), 
    Init_Vfunc_Entry(18, String, substr, __substr), 
    Init_Vfunc_Entry(19, String, find, __find), 
    Init_Vfunc_Entry(20, String, c_str, __c_str), 
    Init_Vfunc_Entry(21, String, size, __size), 
    Init_Vfunc_Entry(22, String, append_str, __append_str), 
    Init_Vfunc_Entry(23, String, append_str_len, __append_str_len), 
    Init_Vfunc_Entry(24, String, append_objective_string, __append_objective_string), 
    Init_Vfunc_Entry(25, String, clear, __clear), 
    Init_Vfunc_Entry(26, String, is_empty, __is_empty), 
    Init_Vfunc_Entry(27, String, replace, __replace), 
    Init_Vfunc_Entry(28, String, replace_all, __replace_all), 
    Init_Vfunc_Entry(29, String, insert_cstr, __insert_cstr), 
    Init_Vfunc_Entry(30, String, insert_cstr_normal, __insert_cstr_normal), 
    Init_Vfunc_Entry(31, String, insert_str_normal, __insert_str_normal), 
    Init_Vfunc_Entry(32, String, insert_str, __insert_str), 
    Init_Vfunc_Entry(33, String, assign_char, __assign_char), 
    Init_Vfunc_Entry(34, String, insert_char_count, __insert_char_count), 
    Init_Str___Entry(35, String, name, NULL), 
    Init_Str___Entry(36, String, value, NULL), 
    Init_End___Entry(37), 
};
REGISTER_CLASS("String", string_class_info);

static void print_vector_data(int index, void *element)
{  
    if (element !=  NULL) {
        dbg_str(DBG_DETAIL, "index:%d value:%s  type_name:%s", 
                index, ((String*)element)->value, 
                ((String*)element)->obj.name);
    }
}

static int test_c_str() 
{
   allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count = allocator->alloc_count;
   String *parent;

   parent = OBJECT_NEW(allocator, String, NULL);
   parent->assign(parent, "abcdebf");  

   printf(" c format value: %s \n", parent->c_str(parent));

   object_destroy(parent);

   return 1;
}

static int test_append_str()
{  
    allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count = allocator->alloc_count;
   String *parent, *substr;

   parent = OBJECT_NEW(allocator, String, NULL);
   substr = OBJECT_NEW(allocator, String, NULL);
   parent->assign(parent, "abcdebf");  
   substr->assign(substr, ">>>>>>>>>>>>>>>>>>>>>");
    
   parent->append_str(parent, ">>>>>>>>>>>>>>>>>>>>>#########$$$$$$$$$$$$");
   printf(" c format value: %s \n", parent->c_str(parent));

   object_destroy(parent);
   object_destroy(substr);
   return 1;
}

static int test_append_str_len()
{
    allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count=allocator->alloc_count;
   String *parent, *substr;

   parent = OBJECT_NEW(allocator, String, NULL);
   substr = OBJECT_NEW(allocator, String, NULL);
   parent->assign(parent, "abcdebf");  
   substr->assign(substr, ">>>>>>>>>>>>>>>>>>>>>");
    
   parent->append_str_len(parent, ">>>>>>>>>>>>>>>>>>>>>#########$$$$$$$$$$$$", 4);
   printf(" c format value: %s \n", parent->c_str(parent));

   parent->append_str_len(parent, ">>>>>>>>>>>>>>>>>>>>>#########$$$$$$$$$$$$", 20);
   printf(" c format value: %s \n", parent->c_str(parent));

   parent->append_str_len(parent, ">>>>>>>>>>>>>>>>>>>>>#########$$$$$$$$$$$$", 40);
   printf(" c format value: %s \n", parent->c_str(parent));

   parent->append_str_len(parent, ">>>>>>>>>>>>>>>>>>>>>#########$$$$$$$$$$$$", 400000);
   printf(" c format value: %s \n", parent->c_str(parent));

   object_destroy(parent);
   object_destroy(substr);
   return 1;
}

static int test_append_objective_string()
{ 
   allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count=allocator->alloc_count;
   String *parent, *substr;

   parent = OBJECT_NEW(allocator, String, NULL);
   substr = OBJECT_NEW(allocator, String, NULL);
   parent->assign(parent, "abcdebf");  
   substr->assign(substr, "{}>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>#########$$$$$$>>>>>>>>>>>>{}");
    
   parent->append_objective_string(parent, substr);
   printf(" c format value: %s \n", parent->c_str(parent));

   parent->append_str(parent, substr->c_str(substr));
   printf(" c format value: %s \n", parent->c_str(parent));


   object_destroy(parent);
   object_destroy(substr);
   return 1;
}

static int test_string_size()
{  
    allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count = allocator->alloc_count;
   String *parent, *substr;

   dbg_str(DBG_DETAIL,"string class size=%d", sizeof(String));
   parent = OBJECT_NEW(allocator, String, NULL);
   substr = OBJECT_NEW(allocator, String, NULL);
   parent->assign(parent, "abcdebf");  
   substr->assign(substr, "{}>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>#########$$$$$$>>>>>>>>>>>>{}");

   printf("parent size %zu child size %zu \n", parent->size(parent), substr->size(substr)); 
   
   parent->append_objective_string(parent, substr);
   printf(" c format value: %s \n", parent->c_str(parent));
   printf("parent size %zu child size %zu \n", parent->size(parent), substr->size(substr)); 
   
   object_destroy(parent);
   object_destroy(substr);
   return 1;
   
}

int test_string_split()
{    

    allocator_t *allocator = allocator_get_default_alloc();
    //test find and split_string function
    String *str_find, *str_separator;
    Vector *vector;
    int ret = 0;

    str_find      = OBJECT_NEW(allocator, String, NULL);
    vector        = OBJECT_NEW(allocator, Vector, NULL);
    str_separator = OBJECT_NEW(allocator, String, NULL);

    str_find->assign(str_find, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
            "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
            "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
            "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
            "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  


#if 1
    str_separator->assign(str_separator, "&");
    //str_separator->assign(str_separator, "//");
#else 
    str_separator->assign(str_separator, "//");
#endif 

    str_find->split_string(str_find, str_separator, vector);

    dbg_str(DBG_DETAIL, "vector len=%d", vector->size(vector));

    ret = assert_int_equal(vector->size(vector), 19);

    vector->for_each(vector, print_vector_data);
    vector->free_vector_elements(vector);

    object_destroy(str_find);
    object_destroy(str_separator);
    object_destroy(vector);

    return ret;

}

int test_string_find()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret = 0;

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, "rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");
    pstr->assign(pstr, "&");

    int pos = string->find(string, pstr, 0);

    ret = assert_int_equal(pos, 16);
    dbg_str(DBG_DETAIL, "substr position: %d ", pos);
    object_destroy(string);
    object_destroy(pstr);

    return ret;

}

int test_string_substr()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr, *substr;
    int ret;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "&");
    substr = string->substr(string, 3, 10);
    printf("substr %s\n ", pstr->value);

    object_destroy(string);
    object_destroy(pstr);
    object_destroy(substr);
    return 1;   
}

static int test_string_empty()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr, *substr;
    int ret;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "&");
    substr = string->substr(string, 3, 10);
    
    dbg_str(DBG_SUC, "current string empty:%d str:%s", string->is_empty(string), string->c_str(string));
    string->clear(string);
    dbg_str(DBG_SUC, "after clear string operation. current is_empty %d str:%s", string->is_empty(string), string->c_str(string));
    ret = string->is_empty(string);

    object_destroy(string);
    object_destroy(pstr);
    object_destroy(substr);  

    return ret;
}

static int test_string_replace()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "&");
    dbg_str(DBG_SUC, "before replaced :%s\n size:%d", string->c_str(string), string->size(string));
    string->replace(string, "&", "<#####>");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));

    string->replace(string, "<#####>", "&");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));

    string->replace(string, "rsv_sug", "@@@@@");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));

    string->replace(string, "rsv_sug", "@@@@@##############################");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));
    
    string->replace(string, "2520%2520%25E6%25A8%25A1", "<@@@@>");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));

    object_destroy(string);
    object_destroy(pstr);

    return 1;
}

static int test_string_replace_all()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "#");

    int pos = string->find(string, pstr, 0);
    dbg_str(DBG_SUC, "position:%d", pos);

    dbg_str(DBG_SUC, "before replaced :%s\n size:%d", string->c_str(string), string->size(string));
    string->replace_all(string, "<##>", "@");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));
    
    string->replace_all(string, "@", "<>");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", string->c_str(string), string->size(string));
     

    object_destroy(string);
    object_destroy(pstr);

    return 1;
}

static int test_string_replace_complex()
{
   allocator_t *allocator = allocator_get_default_alloc();
   String * state_info = OBJECT_NEW(allocator, String , NULL);    
   state_info->assign(state_info, "[extractor_ready_failed] [video_codec_ready_failed] "   
            "[audio_codec_ready_failed] [seek_ready_failed]");

   state_info->replace(state_info, "extractor_ready_failed", "extractor_ready_ok");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", state_info->c_str(state_info), state_info->size(state_info));  

   state_info->replace(state_info, "video_codec_ready_failed", "video_codec_ready_ok");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", state_info->c_str(state_info), state_info->size(state_info));

   state_info->replace(state_info, "audio_codec_ready_failed", "audio_codec_ready_ok");   
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", state_info->c_str(state_info), state_info->size(state_info));

   state_info->replace(state_info, "seek_ready_failed", "seek_ready_ok");
    dbg_str(DBG_SUC, "current replaced:%s\n size:%d", state_info->c_str(state_info), state_info->size(state_info));
    
   object_destroy(state_info);
   return 1;
}

static int test_string_insert_cstr()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "@@@@@@@");
    
    
    string->insert_cstr(string, 3, "@@@@vvvvvvv@@@", 0, 2000);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    dbg_str(DBG_SUC, "before insert:%s\n size:%d", string->c_str(string), string->size(string));
    string->insert_cstr(string, 3, "@@@@@@@", 0, 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));
    
    string->insert_cstr(string, 3, "@@@@fdasfasdf@@@", 0, 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    string->insert_cstr(string, 3, "@@@@vvvvvvv@@@", 0, 2000);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    string->insert_cstr(string, 3, "@@@vvvvvvvv@@@@", 5, 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    string->insert_cstr(string, 3, "@@@@@@@", 0, 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

     string->insert_cstr(string, 3, "@@@@@@@", 0, 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

static int test_string_insert_cstr_normal()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "@@@@@@@");
    
    dbg_str(DBG_SUC, "before insert:%s\n size:%d", string->c_str(string), string->size(string));
    string->insert_cstr_normal(string, 3, "@@@@vvvvvvv@@@");
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));
    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

static int test_string_insert_str()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "@@@@@@@");
    
    dbg_str(DBG_SUC, "before insert:%s\n size:%d", string->c_str(string), string->size(string));
    string->insert_str(string, 3, pstr, 0, 7);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));
    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

static int test_string_insert_str_normal()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, test);
    pstr->assign(pstr, "@@@@@@@");
    
    dbg_str(DBG_SUC, "before insert:%s\n size:%d", string->c_str(string), string->size(string));
    string->insert_str_normal(string, 3, pstr);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));
    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

static int test_string_assign_char_count()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign_char(string, 'c', 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

static int test_string_insert_char_count()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";

    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);

    string->assign(string, ">>>>>>>>>");
    string->insert_char_count(string, 4, 'c', 4);
    dbg_str(DBG_SUC, "after insert:%s\n size:%d", string->c_str(string), string->size(string));

    
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

REGISTER_TEST_FUNC(test_c_str);
REGISTER_TEST_FUNC(test_append_str);
REGISTER_TEST_FUNC(test_append_str_len);
REGISTER_TEST_FUNC(test_string_size);
REGISTER_TEST_FUNC(test_string_split);
REGISTER_TEST_FUNC(test_string_find);
REGISTER_TEST_FUNC(test_string_substr);
REGISTER_STANDALONE_TEST_FUNC(test_string_empty);
REGISTER_STANDALONE_TEST_FUNC(test_string_replace);
REGISTER_STANDALONE_TEST_FUNC(test_string_replace_all);
REGISTER_STANDALONE_TEST_FUNC(test_string_replace_complex);
REGISTER_STANDALONE_TEST_FUNC(test_string_insert_cstr);
REGISTER_STANDALONE_TEST_FUNC(test_string_insert_cstr_normal);
REGISTER_STANDALONE_TEST_FUNC(test_string_insert_str);
REGISTER_STANDALONE_TEST_FUNC(test_string_insert_str_normal);
REGISTER_STANDALONE_TEST_FUNC(test_string_assign_char_count);
REGISTER_STANDALONE_TEST_FUNC(test_string_insert_char_count);
