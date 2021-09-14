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
#include <regex.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>

typedef struct string_piece_s {
    void *start_pos;
    int len;
} string_piece_t;

string_piece_t *
__new_string_peice(allocator_t *allocator, void *start_pos, int len) 
{
    string_piece_t *piece;

    piece = (string_piece_t *) 
            allocator_mem_alloc(allocator, sizeof(string_piece_t));

    piece->start_pos = start_pos;
    piece->len = len;

    return piece;
}

static int __modulate_capacity(String *string, int write_len)
{
    if (string->value_max_len > string->value_len + 1 &&
        string->value_max_len < string->value_len + write_len + 1)
    {
        char *new_buf;
        int old = string->value_max_len;

        string->value_max_len = 2 * (string->value_len + write_len + 1);
        new_buf = (char *)allocator_mem_alloc(string->obj.allocator, 
                                              string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING, "string assign alloc error");
            return -1;
        } else {
            dbg_str(OBJ_WARNNING, "auto modulate string object max value len, "
                    "write_len =%d, value_max_len from %d to %d",
                    write_len, old, string->value_max_len);
        
        }

        memset(new_buf, 0, string->value_max_len);
        strncpy(new_buf, string->value, string->value_len);

        allocator_mem_free(string->obj.allocator, string->value);
        string->value = new_buf;
    }

    return 1;
}

static int __construct(String *string, char *init_str)
{
    dbg_str(OBJ_DETAIL, "string construct, string addr:%p", string);

    string->value_len = 0;
    string->value_max_len = 256;
    string->value = (char *)allocator_mem_alloc(string->obj.allocator, string->value_max_len);
    memset(string->value, 0, string->value_max_len);

    return 1;
}

static int __deconstrcut(String *string)
{
    dbg_str(OBJ_DETAIL, "string deconstruct, string addr:%p", string);
    if (string->value)
        allocator_mem_free(string->obj.allocator, string->value);
    if (string->pieces != NULL) {
        object_destroy(string->pieces);
        string->pieces = 0;
    }

    return 1;
}

static String *__alloc(String *string, uint32_t size)
{
    dbg_str(OBJ_DETAIL, "alloc, size = %d", size);

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

static void __reset(String *string)
{   
    Vector *v;

    string->value[0] = '\0';
    memset(string->value, 0, string->value_max_len);
    string->value_len = 0;

    if (string->pieces != NULL) {
        v = (Vector *)string->pieces;
        v->reset(v);
    }
}

static char __at(String *string, int index)
{
    return string->value[index];
}

static String *__assign(String *string, char *s)
{
    int l = strlen(s);
    int ret;

    ret = __modulate_capacity(string, l);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, l);
    string->value_len  = l;
    string->value[l] = '\0';

    return string;
}

static int __equal(String *string, char *s)
{
    return strcmp(string->value, s) == 0;
}

static String *__append_char(String *string, char c)
{
    int ret;

    ret = __modulate_capacity(string, 1);
    if (ret < 0) {
        dbg_str(STRING_WARNNING, "string buf_auto_modulate have problem, please check");
        return string;
    }

    string->value[string->value_len] = c;
    string->value_len++;
    string->value[string->value_len] = '\0';

    return string;
}

static int __format(String *string, int max_len, char *fmt, ...)
{
    allocator_t *allocator = string->obj.allocator;
    int ret = 0;
    va_list ap;
    char *buf_addr;

    buf_addr = allocator_mem_alloc(allocator, max_len);

    va_start(ap, fmt);
    ret = vsnprintf(buf_addr, max_len, fmt, ap);
    va_end(ap);

    string->append(string, buf_addr, -1);

    allocator_mem_free(allocator, buf_addr);

    return ret;
}

static void __append(String *string, char *sub, int len) 
{   
    int ret;
    if (sub == NULL) {
        dbg_str(STRING_WARNNING, "appending-string is null unvalid sub stringing");
        return ;
    }

    if (len <= 0) {
        len = strlen(sub);
    }

    ret = __modulate_capacity(string, len + string->value_len);

    if (ret < 0) {
        return ;
    }

    strncpy(string->value + string->value_len, sub, len);
    string->value_len += len;
    string->value[string->value_len] = '\0';
}

static String * __insert(String *self, int offset, char *cstr)
{
    int dest_len = self->get_len(self);
    int len = strlen(cstr);
    int ret = 0;

    if (cstr == NULL) {
        dbg_str(STRING_ERROR, "insert ctr, but cstr is NULL");
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

static String * __replace_char(String *string, int index, char c)
{
    string->value[index] = c;

    return string;
}

static int 
__replace(String *self, char *pattern, char *newstr, int max)
{
    int start_offset = 0, old_offset;
    int old_len, new_len, str_len;
    Vector *v;
    string_piece_t *piece = NULL;
    int count = 0, ret = 0;

    if (pattern == NULL || newstr == NULL) { return -1; }

    new_len = strlen(newstr);
    str_len = self->get_len(self);

    do {
        ret = self->find(self, pattern, start_offset, 1);
        if (ret <= 0) {
            dbg_str(STRING_DETAIL, "replace, not found");
            goto end;
        } else if (ret > 1) {
            ret = -1;
            goto end;
        }

        v = self->pieces;
        if (v == NULL) {
            ret = -1;
            goto end;
        }

        v->peek_at(v, 0, (void **)&piece);
        if (piece != NULL) {
            old_len = piece->len;
            old_offset = (int)(piece->start_pos - (void *)self->value);
        } else {
            dbg_str(STRING_WARNNING, "get_splited_cstr: not exist!!");
        }

        if (str_len <= str_len + new_len - old_len) {
            ret = __modulate_capacity(self, str_len + new_len - old_len);
            if (ret < 0 ) {
                ret = -1;
                goto end;
            }
        }

        memmove(self->value + old_offset + new_len, 
                self->value + old_offset + piece->len,
                strlen(self->value + old_offset + piece->len));
        memmove(self->value + old_offset, newstr, new_len); 
        self->value_len += (new_len - old_len);
        self->value[self->value_len] = '\0';
        count++;
        start_offset = old_offset + new_len;
    } while (count < max || max <= 0);

end:

    return count;
}

static void __to_upper(String *string)
{
    int size = string->value_len;
    int i;

    for (i = 0; i < size; i++) {
        if (islower(string->value[i])) {
            string->value[i] += 'A'-'a';
        }
    }       
}

static void __to_lower(String *string)
{
    int size = string->value_len;
    int i;

    for (i = 0; i < size; i++) {
        if (isupper(string->value[i])) {
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
        memmove(string->value, string->value + cnt, string->value_len);
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

static int __get_substring(String *string, char *pattern, int offset, int *start, int *len)
{
    char *pos;
    Vector *v;
    string_piece_t *piece;
    allocator_t *allocator = string->obj.allocator;
    regex_t regex;  
    regmatch_t pmatch[2] = {{-1,-1},{-1,-1}}; 
    int nmatch = 2, ret = -1;

    TRY {
        ret = regcomp_wrap(&regex, pattern, REG_EXTENDED|REG_ICASE);  
        if (ret) return -1;

        pos = string->get_cstr(string);
        if (strlen(pos) == 0) return 0;
        pos += offset;

        ret = regexec_wrap(&regex, pos, nmatch, pmatch, 0);  
        THROW_IF(ret == REG_NOMATCH, -1);
        THROW_IF(pmatch[1].rm_so == -1, -1);
        *start = (pmatch[1].rm_so + offset);
        *len = pmatch[1].rm_eo - pmatch[1].rm_so;
        ret = pmatch[0].rm_eo + offset; 

        dbg_str(DBG_DETAIL, "get subsring, pattern:%s, start:%d, len:%d, substr:%.*s", 
                pattern, *start, *len, *len, &string->value[pmatch[1].rm_so]);
        THROW(ret);
    } CATCH (ret) {
        *start = -1;
    } FINALLY {
        regfree_wrap(&regex);  
    }

    return ret;
}

static int __find(String *string, char *pattern, int offset, int num)
{
    int cnt = 0;
    char *pos;
    Vector *v;
    string_piece_t *piece;
    allocator_t *allocator = string->obj.allocator;
    regex_t regex;  
    regmatch_t pmatch[1];  
    int nmatch = 1, ret = -1;

    if (string->pieces == NULL) {
        uint8_t trustee_flag = 1;
        int value_type = VALUE_TYPE_ALLOC_POINTER;

        v = object_new(allocator, "Vector", NULL);
        v->set(v, "/Vector/trustee_flag", &trustee_flag);
        v->set(v, "/Vector/value_type", &value_type);

        string->pieces = v;
    } else {
        v = string->pieces;
        v->reset(v);
    }

    ret = regcomp_wrap(&regex, pattern, REG_EXTENDED);  
    if (ret) {
        dbg_str(STRING_ERROR, "regex error");
        return -1;
    }

    pos = string->get_cstr(string);
    if (strlen(pos) == 0) return 0;

    pos += offset;

    do {
        ret = regexec_wrap(&regex, pos, nmatch, pmatch, 0);  
        if (ret != REG_NOMATCH) { 
            cnt++;
            piece = __new_string_peice(allocator, pos + pmatch[0].rm_so ,
                                       pmatch[0].rm_eo - pmatch[0].rm_so); 
            dbg_str(STRING_DETAIL, "found count:%d, offset:%d", cnt, 
                    (int)(pos + pmatch[0].rm_so - string->get_cstr(string)));
            v->add_back(v, piece); 
            pos += pmatch[0].rm_eo; 
        } else {
            break;
        }

    } while ((cnt < num || num <= 0));

    regfree_wrap(&regex);  

    return cnt;
}

static int __split(String *string, char *delims, int num)
{
    int piece_count = 0, count = 0;
    char *pos;
    Vector *v;
    string_piece_t *piece;
    allocator_t *allocator = string->obj.allocator;
    regex_t regex;  
    regmatch_t pmatch[1];  
    int nmatch = 1, ret = -1;

    if (string->pieces == NULL) {
        uint8_t trustee_flag = 1;
        int value_type = VALUE_TYPE_ALLOC_POINTER;

        v = object_new(allocator, "Vector", NULL);
        v->set(v, "/Vector/trustee_flag", &trustee_flag);
        v->set(v, "/Vector/value_type", &value_type);

        string->pieces = v;
    } else {
        v = string->pieces;
        v->reset(v);
    }

    ret = regcomp_wrap(&regex, delims, REG_EXTENDED);  
    if (ret) {
        dbg_str(STRING_ERROR, "regex error, ret=%d", ret);
        return -1;
    }

    pos = string->get_cstr(string);
    if (strlen(pos) == 0) return 0;

    while ((count++ < num || num <= 0)) {
        ret = regexec_wrap(&regex, pos, nmatch, pmatch, 0);  
        if (ret != REG_NOMATCH) { 
            if (pmatch[0].rm_so != 0) {
                piece_count++;
                piece = __new_string_peice(allocator, pos, pmatch[0].rm_so); 
                v->add_back(v, piece); 
            }
            pos[pmatch[0].rm_eo - 1] = '\0';
            pos[pmatch[0].rm_so] = '\0';
            pos += pmatch[0].rm_eo; 
        } else {
            break;
        }
    }

    piece_count++;
    piece = __new_string_peice(allocator, pos, strlen(pos)); 
    v->add_back(v, piece); 

    regfree_wrap(&regex);  

    return piece_count;
}

static char *__get_splited_cstr(String *string, int index)
{
    Vector *v = string->pieces;
    string_piece_t *piece = NULL;
    char *d = NULL;

    if (v == NULL) {
        return NULL;
    }
    
    v->peek_at(v, index, (void **)&piece);
    if (piece != NULL) {
        d = piece->start_pos;
    } else {
        dbg_str(STRING_WARNNING, "get_splited_cstr: not exist!!");
    }

    return d;
}

static char *__get_found_cstr(String *string, int index)
{
    Vector *v = string->pieces;
    string_piece_t *piece = NULL;
    char *d = NULL;

    if (v == NULL) {
        return NULL;
    }
    
    v->peek_at(v, index, (void **)&piece);
    if (piece != NULL) {
        d = piece->start_pos;
    } else {
        dbg_str(STRING_WARNNING, "get_splited_cstr: not exist!!");
    }

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
    Init_Vfunc_Entry(8 , String, reset, __reset), 
    Init_Vfunc_Entry(9 , String, alloc, __alloc), 
    Init_Vfunc_Entry(10, String, modulate_capacity, __modulate_capacity), 
    Init_Vfunc_Entry(11, String, assign, __assign), 
    Init_Vfunc_Entry(12, String, equal, __equal), 
    Init_Vfunc_Entry(13, String, replace, __replace), 
    Init_Vfunc_Entry(14, String, replace_char, __replace_char), 
    Init_Vfunc_Entry(15, String, format, __format), 
    Init_Vfunc_Entry(16, String, append, __append), 
    Init_Vfunc_Entry(17, String, append_char, __append_char), 
    Init_Vfunc_Entry(18, String, insert, __insert), 
    Init_Vfunc_Entry(19, String, toupper, __to_upper), 
    Init_Vfunc_Entry(20, String, tolower, __to_lower), 
    Init_Vfunc_Entry(21, String, ltrim, __ltrim), 
    Init_Vfunc_Entry(22, String, rtrim, __rtrim), 
    Init_Vfunc_Entry(23, String, trim, __trim), 
    Init_Vfunc_Entry(24, String, get_substring, __get_substring), 
    Init_Vfunc_Entry(25, String, find, __find), 
    Init_Vfunc_Entry(26, String, is_empty, __is_empty), 
    Init_Vfunc_Entry(27, String, split, __split), 
    Init_Vfunc_Entry(28, String, get_splited_cstr, __get_splited_cstr), 
    Init_Vfunc_Entry(29, String, get_found_cstr, __get_found_cstr), 
    Init_Point_Entry(30, String, value, NULL), 
    Init_End___Entry(31, String), 
};
REGISTER_CLASS("String", string_class_info);
