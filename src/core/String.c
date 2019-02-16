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

static int __set(String *string, char *attrib, void *value)
{

    if (strcmp(attrib, "set") == 0) {
        string->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        string->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        string->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        string->deconstruct = value;
    } else if (strcmp(attrib, "pre_alloc") == 0) {
        string->pre_alloc = value;
    } else if (strcmp(attrib, "assign") == 0) {
        string->assign = value;
    } else if (strcmp(attrib, "append_char") == 0) {
        string->append_char = value;
    } else if (strcmp(attrib, "replace_char") == 0) {
        string->replace_char = value;
    } else if (strcmp(attrib, "toupper") == 0) {
        string->toupper = value;
    } else if (strcmp(attrib, "toupper_impact") == 0) {
        string->toupper_impact = value;
    } else if (strcmp(attrib, "lower") == 0) {
        string->tolower = value;
    } else if (strcmp(attrib, "lower_impact") == 0) {
        string->tolower_impact = value;
    } else if (strcmp(attrib, "at") == 0) {
        string->at = value;
    } else if (strcmp(attrib, "ltrim") == 0) {
        string->ltrim = value; 
    } else if (strcmp(attrib, "rtrim") == 0) {
        string->rtrim = value;
    } else if (strcmp(attrib, "trim") == 0) {
        string->trim = value;
    } else if (strcmp(attrib, "split_string") == 0) {
        string->split_string = value;
    } else if (strcmp(attrib, "find") == 0) {
        string->find = value;
    } else if (strcmp(attrib, "substr") == 0) {
        string->substr = value;
    } else if(strcmp(attrib, "c_str")==0){
         string->c_str=value;
    } else if(strcmp(attrib, "append_str")==0){
         string->append_str=value;
    } else if(strcmp(attrib, "append_str_len")==0){
         string->append_str_len=value;
    } else if(strcmp(attrib, "append_objective_string")==0){
         string->append_objective_string=value;
    } else if(strcmp(attrib, "size")==0){
         string->size=value;
    } else if(strcmp(attrib, "is_empty")==0){
      string->is_empty=value;
    } else if(strcmp(attrib, "clear")==0){
      string->clear=value;
    } else if(strcmp(attrib, "replace")==0){
      string->replace=value;
    } else if(strcmp(attrib, "replace_all")==0){
      string->replace_all=value;
    } else if(strcmp(attrib, "insert_cstr")==0){
      string->insert_cstr=value;
    } else if(strcmp(attrib, "insert_str")==0){
      string->insert_str=value;
    } else if(strcmp(attrib, "insert_str_normal")==0){
      string->insert_str_normal=value;
    } else if(strcmp(attrib, "insert_cstr_normal")==0){
      string->insert_cstr_normal=value;
    } else if(strcmp(attrib, "assign_char")==0){
      string->assign_char=value;
    } else if(strcmp(attrib, "insert_char_count")==0){
      string->insert_char_count=value;
    } else if(strcmp(attrib, "compare")==0){
      string->compare=value;
    } else if(strcmp(attrib, "compare_ignore")==0){
       string->compare_ignore=value;
    } else if(strcmp(attrib, "equal_ignore")==0){
       string->equal_ignore=value;
    } else if(strcmp(attrib, "equal")==0){
       string->equal=value;
    } else if(strcmp(attrib, "copy")==0){
       string->copy=value;
    } else if(strcmp(attrib, "compare_cstr")==0){
       string->compare_cstr=value;
    } else if(strcmp(attrib, "compare_ignore_cstr")==0){
       string->compare_ignore_cstr=value;
    } else if(strcmp(attrib, "equal_cstr")==0){
       string->equal_cstr=value;
    } else if(strcmp(attrib, "equal_ignore_cstr")==0){
       string->equal_ignore_cstr=value;
    } else if(strcmp(attrib, "find_cstr")==0){
       string->find_cstr=value;
    } else if (strcmp(attrib, "name") == 0) {
       strncpy(string->name, value, strlen(value));
    } else {
         dbg_str(OBJ_DETAIL, "string set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(String *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "value") == 0) {
        return obj->value;
    } else {
        dbg_str(OBJ_WARNNING, "string get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static String *__pre_alloc(String *string, uint32_t size)
{
    dbg_str(OBJ_DETAIL, "pre_alloc, size = %d", size);

    if (size < string->value_max_len) return string;
    else {
        allocator_mem_free(string->obj.allocator, string->value);
        string->value         = (char *)allocator_mem_alloc(string->obj.allocator, size);
        string->value_max_len = size;
        memset(string->value, 0, size);
    }
    return string;
}

static String *__assign(String *string, char *s)
{
    int len = strlen(s);
    int ret;
    string->clear(string);
    ret = string_buf_auto_modulate(string, len);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len  = len;
    string->value[len] = '\0';

    return string;
}

static String *__assign_char(String *string,char c,size_t count)
{
    int i = 0;
    int ret;
    string->clear(string);
    char * ptmp = (char *)allocator_mem_alloc(string->obj.allocator,count);
    for (i = 0 ; i < count; i++) {
        *(ptmp+i) = c;
    }
    *(ptmp + i) = '\0';
    string->assign(string,ptmp);
    allocator_mem_free(string->obj.allocator,ptmp);
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

static void __split_string(String *string, String *separator, Vector *vector)
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

static String * __replace(String *self,char *old,char *newstr)
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

    String *old_str = OBJECT_NEW(self->obj.allocator,String,NULL);
    old_str->assign(old_str,old);
    start_pos = self->find(self,old_str,start_pos);

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
    memmove(self->value+end_pos,self->value+start_pos+old_len,str_len-(start_pos+old_len));
    memmove(self->value+start_pos,newstr,new_len); 
    self->value_len += (new_len-old_len);
    self->value[self->value_len] = '\0';

end:
    object_destroy(old_str);
    return self;
}

static String * __replace_all(String *self,char *oldstr,char *newstr)
{
    String *pre = self;
    String *cur = NULL;
    if ( oldstr == NULL || newstr == NULL ) {
        return self;
    }
    while (cur == NULL || strcmp(cur->c_str(cur),pre->c_str(pre)) != 0 ) {
        if (cur != NULL ) { 
            object_destroy(cur);
            cur = NULL;
        }
        cur = OBJECT_NEW(self->obj.allocator,String,NULL);
        cur->assign(cur,pre->c_str(pre));
        pre = pre->replace(pre,oldstr,newstr);
    }
    object_destroy(cur);

    return self;
}

static String * __insert_char_count(String *self,size_t pos,char c,size_t count)
{
    String * ptem = OBJECT_NEW(self->obj.allocator,String,NULL);
    ptem->assign_char(ptem,c,count);
    self->insert_str_normal(self,pos,ptem);
    object_destroy(ptem);
}

static String * __insert_cstr(String *self,size_t index,char *src,size_t pos,size_t len)
{
    int size = strlen(src);
    int dest_size = self->size(self);
    int ret = 0;
    if ( src == NULL || pos > (size - 1) ) {
        dbg_str(DBG_ERROR,"src == NULL or pos data is unvalid pos %d src size:%d",pos,size);
        goto end;
    }

    if (pos + len > size) {
        len = size - pos;
    }
     
    ret = string_buf_auto_modulate(self, len);
    if (ret < 0 ) {
            goto end;
    }
    
    memmove(self->value+index+len,self->value+index,dest_size-index);
    memmove(self->value+index,src+pos,len);
    self->value_len += len;
    self->value[self->value_len] = '\0';     
end:
    return  self;
}

static String * __insert_str(String *self,size_t index,String *src,size_t pos,size_t len)
{
    return self->insert_cstr(self,index,src->c_str(src),pos,len);
}

static String * __insert_str_normal(String * self,size_t index,String *cstr)
{
    return self->insert_str(self,index,cstr,0,cstr->size(cstr));
}

static String * __insert_cstr_normal(String * self,size_t index,char *cstr)
{
    return self->insert_cstr(self,index,cstr,0,strlen(cstr));
}

static int   __compare(String *self,String *obj)
{
    int max_size = self->size(self) > obj->size(obj) ? self->size(self):obj->size(obj);
    return strncmp(self->c_str(self),obj->c_str(obj),max_size);
}

static int __compare_ignore(String *self,String *obj)
{
    return strcasecmp(self->c_str(self),obj->c_str(obj));
}

static int  __equal(String *self,String *obj)
{
    return self->compare(self,obj) == 0 ? 1:0;
}

static int __equal_ignore(String *self,String *obj)
{
    return self->compare_ignore(self,obj) ==0 ? 1:0;
}

static String * __copy(String *self)
{
    String *ts = OBJECT_NEW(self->obj.allocator,String,NULL);
    ts->assign(ts,self->c_str(self));
    return ts;
}

static int  __compare_cstr(String *self,char *value)
{   
    int max_size,len1;
    len1 = strlen(value);
    max_size = self->size(self) > len1 ? self->size(self):len1;
    return strncmp(self->c_str(self),value,max_size);
}

static int  __compare_ignore_cstr(String *self,char *value)
{
    return strcasecmp(self->c_str(self),value);
}

static int  __equal_cstr(String *self,char *value)
{
    return self->compare_cstr(self,value) == 0?1:0;
}

static int  __equal_ignore_cstr(String *self,char *value)
{
    return self->compare_ignore_cstr(self,value) == 0 ? 1 : 0;
}

static int  __find_cstr(String *string,char *substr,int pos)
{
    int len1 = string->value_len;
    int len2 = strlen(substr);
    char *p  = NULL;

    if (NULL == string || NULL == substr || pos < 0 || len1 < len2) {
        return -1;//exception happened
    }
    //a simple method
    p = strstr(string->value + pos, substr);
    if(NULL !=  p) {
        return p - (string->value);
    }
    return -1;
}
    
static class_info_entry_t string_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "pre_alloc", __pre_alloc, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "assign", __assign, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_FUNC_POINTER, "", "append_char", __append_char, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_FUNC_POINTER, "", "replace_char", __replace_char, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_FUNC_POINTER, "", "at", __at, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_FUNC_POINTER, "", "toupper", __toupper_, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_FUNC_POINTER, "", "toupper_impact", __toupper_impact, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_FUNC_POINTER, "", "tolower_impact", __tolower_impact, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_FUNC_POINTER, "", "tolower", __tolower_, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_FUNC_POINTER, "", "ltrim", __ltrim, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_FUNC_POINTER, "", "rtrim", __rtrim, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_FUNC_POINTER, "", "trim", __trim, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_FUNC_POINTER, "", "split_string", __split_string, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_FUNC_POINTER, "", "substr", __substr, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_FUNC_POINTER, "", "find", __find, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_FUNC_POINTER, "", "c_str", __c_str, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_FUNC_POINTER, "", "size", __size, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_FUNC_POINTER, "", "append_str", __append_str, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_FUNC_POINTER, "", "append_str_len", __append_str_len, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_FUNC_POINTER, "", "append_objective_string", __append_objective_string, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_FUNC_POINTER, "", "clear", __clear, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_FUNC_POINTER, "", "is_empty", __is_empty, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_FUNC_POINTER, "", "replace", __replace, sizeof(void *)}, 
    [28] = {ENTRY_TYPE_FUNC_POINTER, "", "replace_all", __replace_all, sizeof(void *)}, 
    [29] = {ENTRY_TYPE_FUNC_POINTER, "", "insert_cstr", __insert_cstr, sizeof(void *)}, 
    [30] = {ENTRY_TYPE_FUNC_POINTER, "", "insert_cstr_normal", __insert_cstr_normal, sizeof(void *)}, 
    [31] = {ENTRY_TYPE_FUNC_POINTER, "", "insert_str_normal", __insert_str_normal, sizeof(void *)}, 
    [32] = {ENTRY_TYPE_FUNC_POINTER, "", "insert_str", __insert_str, sizeof(void *)}, 
    [33] = {ENTRY_TYPE_FUNC_POINTER, "", "assign_char", __assign_char, sizeof(void *)}, 
    [34] = {ENTRY_TYPE_FUNC_POINTER, "", "insert_char_count", __insert_char_count, sizeof(void *)}, 
    [35] = {ENTRY_TYPE_FUNC_POINTER, "", "compare", __compare, sizeof(void *)}, 
    [36] = {ENTRY_TYPE_FUNC_POINTER, "", "compare_ignore", __compare_ignore, sizeof(void *)}, 
    [37] = {ENTRY_TYPE_FUNC_POINTER, "", "equal", __equal, sizeof(void *)}, 
    [38] = {ENTRY_TYPE_FUNC_POINTER, "", "equal_ignore", __equal_ignore, sizeof(void *)}, 
    [39] = {ENTRY_TYPE_FUNC_POINTER, "", "copy", __copy, sizeof(void *)}, 
    [40] = {ENTRY_TYPE_FUNC_POINTER, "", "compare_cstr", __compare_cstr, sizeof(void *)}, 
    [41] = {ENTRY_TYPE_FUNC_POINTER, "", "compare_ignore_cstr", __compare_ignore_cstr, sizeof(void *)}, 
    [42] = {ENTRY_TYPE_FUNC_POINTER, "", "equal_cstr", __equal_cstr, sizeof(void *)}, 
    [43] = {ENTRY_TYPE_FUNC_POINTER, "", "equal_ignore_cstr", __equal_ignore_cstr, sizeof(void *)}, 
    [44] = {ENTRY_TYPE_FUNC_POINTER, "", "find_cstr", __find_cstr, sizeof(void *)}, 
    [45] = {ENTRY_TYPE_STRING, "char *", "name", NULL, 0}, 
    [46] = {ENTRY_TYPE_STRING, "char *", "value", NULL, 0}, 
    [47] = {ENTRY_TYPE_END}, 
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

static int test_size()
{  
    allocator_t *allocator = allocator_get_default_alloc();
   //test find and split_string function
   int count = allocator->alloc_count;
   String *parent, *substr;

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
    
    dbg_str(DBG_SUC,"current string empty:%d str:%s",string->is_empty(string),string->c_str(string));
    string->clear(string);
    dbg_str(DBG_SUC,"after clear string operation. current is_empty %d str:%s",string->is_empty(string),string->c_str(string));
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
    dbg_str(DBG_SUC,"before replaced :%s\n size:%d",string->c_str(string),string->size(string));
    string->replace(string,"&","<#####>");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));

    string->replace(string,"<#####>","&");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));

    string->replace(string,"rsv_sug","@@@@@");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));

    string->replace(string,"rsv_sug","@@@@@##############################");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));
    
    string->replace(string,"2520%2520%25E6%25A8%25A1","<@@@@>");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));

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

    int pos = string->find(string,pstr,0);
    dbg_str(DBG_SUC,"position:%d",pos);

    dbg_str(DBG_SUC,"before replaced :%s\n size:%d",string->c_str(string),string->size(string));
    string->replace_all(string,"<##>","@");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));
    
    string->replace_all(string,"@","<>");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",string->c_str(string),string->size(string));
     

    object_destroy(string);
    object_destroy(pstr);

    return 1;
}

static int test_string_replace_complex()
{
   allocator_t *allocator = allocator_get_default_alloc();
   String * state_info = OBJECT_NEW(allocator,String ,NULL);    
   state_info->assign(state_info,"[extractor_ready_failed] [video_codec_ready_failed] "   
            "[audio_codec_ready_failed] [seek_ready_failed]");

   state_info->replace(state_info,"extractor_ready_failed","extractor_ready_ok");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",state_info->c_str(state_info),state_info->size(state_info));  

   state_info->replace(state_info,"video_codec_ready_failed","video_codec_ready_ok");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",state_info->c_str(state_info),state_info->size(state_info));

   state_info->replace(state_info,"audio_codec_ready_failed","audio_codec_ready_ok");   
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",state_info->c_str(state_info),state_info->size(state_info));

   state_info->replace(state_info,"seek_ready_failed","seek_ready_ok");
    dbg_str(DBG_SUC,"current replaced:%s\n size:%d",state_info->c_str(state_info),state_info->size(state_info));
    
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
    
    
    string->insert_cstr(string,3,"@@@@vvvvvvv@@@",0,2000);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    dbg_str(DBG_SUC,"before insert:%s\n size:%d",string->c_str(string),string->size(string));
    string->insert_cstr(string,3,"@@@@@@@",0,4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));
    
    string->insert_cstr(string,3,"@@@@fdasfasdf@@@",0,4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    string->insert_cstr(string,3,"@@@@vvvvvvv@@@",0,2000);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    string->insert_cstr(string,3,"@@@vvvvvvvv@@@@",5,4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    string->insert_cstr(string,3,"@@@@@@@",0,4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

     string->insert_cstr(string,3,"@@@@@@@",0,4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    
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
    
    dbg_str(DBG_SUC,"before insert:%s\n size:%d",string->c_str(string),string->size(string));
    string->insert_cstr_normal(string,3,"@@@@vvvvvvv@@@");
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));
    
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
    
    dbg_str(DBG_SUC,"before insert:%s\n size:%d",string->c_str(string),string->size(string));
    string->insert_str(string,3,pstr,0,7);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));
    
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
    
    dbg_str(DBG_SUC,"before insert:%s\n size:%d",string->c_str(string),string->size(string));
    string->insert_str_normal(string,3,pstr);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));
    
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

    string->assign_char(string,'c',4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));

    
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

    string->assign(string,">>>>>>>>>");
    string->insert_char_count(string,4,'c',4);
    dbg_str(DBG_SUC,"after insert:%s\n size:%d",string->c_str(string),string->size(string));
    object_destroy(string);
    object_destroy(pstr);
    return 1;
}

static int test_string_compare()
{
    allocator_t *allocator = allocator_get_default_alloc();
    String *string, *pstr;
    int ret;
    char *test = "fasdkfj<##>asdkolfj<##>asdlfkjasd<##>ld";
    string = OBJECT_NEW(allocator, String, NULL);
    pstr   = OBJECT_NEW(allocator, String, NULL);
    string->assign(string,">>>>>>>>>");
    pstr->assign(pstr,">>>>>>>>>");
    ret = string->equal(string,pstr);
    dbg_str(DBG_SUC," compare :%d ", ret );
    pstr->assign(pstr,">>>>>>>>>fdsafdsaf");
    ret = string->equal(string,pstr);
    dbg_str(DBG_SUC," compare :%d ", ret );
    pstr->clear(pstr);
    string->clear(string);
    pstr->assign(pstr,"abc");
    string->assign(string,"ABC");
    ret = string->equal_ignore(string,pstr);
    dbg_str(DBG_SUC,"%s %s  compare :%d ", string->c_str(string),pstr->c_str(pstr),ret);
    pstr->clear(pstr);
    string->clear(string);
    pstr->assign(pstr,"abc");
    string->assign(string,"ABCd");
    ret = string->equal_ignore(string,pstr);
    dbg_str(DBG_SUC,"%s %s  compare :%d ", string->c_str(string),pstr->c_str(pstr),ret);
    object_destroy(string);
    object_destroy(pstr);
    
    return 1;
}

REGISTER_TEST_FUNC(test_c_str);
REGISTER_TEST_FUNC(test_append_str);
REGISTER_TEST_FUNC(test_append_str_len);
REGISTER_TEST_FUNC(test_size);
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
REGISTER_STANDALONE_TEST_FUNC(test_string_compare);