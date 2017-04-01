/**
 * @file text.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-03
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
#include <libdbg/debug.h>
#include <libobject/ui/text.h>
#include <libobject/ui/character.h>
#include <libobject/list_linked.h>

extern char *global_text;

static Iterator *get_iterator_of_nth_line(Iterator *cur,Iterator *end, int n)
{
    int i;

    for ( i = 0; i < n && !end->equal(end,cur); i++) {
        cur->next(cur);
    }

    return cur;
}

int delete_nth_line(Text *text, int line_num)
{
    Iterator *cur = NULL, *end;
    List *list  = text->line_info;

    cur = text->line_info->begin(text->line_info);
    end = text->line_info->end(text->line_info);
    cur = get_iterator_of_nth_line(cur, end, line_num);

    list->del(list,cur);

    object_destroy(cur);
    object_destroy(end);
}

int extract_text_disturbed_by_inserting(Text *text, int line_num,
                                        int offset,  char *str,
                                        int len, Font *font)
{
    text_line_t *line_info = NULL;
    Iterator *cur          = NULL, *end;
    int find_flag          = -1, line_count = 0;
    int i                  = 0;

    cur = text->line_info->begin(text->line_info);
    end = text->line_info->end(text->line_info);

    for (i = 0; !end->equal(end,cur); cur->next(cur), i++) {
        if (i == line_num) {
            line_info = cur->get_vpointer(cur);
            if (offset < 0) return -1;
            strncpy(str, line_info->head + offset, line_info->tail -line_info->head - offset + 1);
            line_count ++;
            find_flag = 1;
            /*
             *dbg_str(DBG_DETAIL,"insert start from:%s", line_info->head + offset);
             */
            if (line_info->head[offset] == '\n') {
                continue;
            };

            if ((*line_info->tail) == '\n') {
                break;
            } else  
                continue;
        }
        if (find_flag == 1) { 
            line_count ++;
            line_info = cur->get_vpointer(cur);
            if (strlen(str) + line_info->tail - line_info->head + 1 > len) {
                dbg_str(DBG_WARNNING,"buffer too small, please check");
                return -1;
            }
            /*
             *dbg_str(DBG_DETAIL,"%s", line_info->head);
             */
            strncpy(str + strlen(str), line_info->head, line_info->tail - line_info->head + 1);
            if ((*line_info->tail) == '\n') {
                break;
            }
            /*
             *} else if (*(line_info->tail + 1) == '\0') {
             *    break;
             */
        }
    }

    object_destroy(cur);
    object_destroy(end);

    return line_count;

}

/**
 * @synopsis            rewrite the text, we dont realloc the structure to store text. 
 *
 * @param text          
 * @param start_line
 * @param offset        write at the offset of the start line
 * @param width         write a the width of the start line
 * @param count         line count to write
 * @param str           string to write
 * @param out_len       write back the len writen
 * @param out_line_cnt  write back the line count writen
 * @param font
 *
 * @returns   
 */
int rewrite_text(Text *text, int start_line,int offset,
                 int width, int count,
                 char *str,int *out_len, int *out_line_cnt, void *font)
{
#define MAX_TEXT_LINE_LENTH 256
    int line_width          = text->width;
    int line_num            = 0;
    int line_lenth          = 0;
    int x                   = 0, y = 0;
    Font *f                 = (Font *)font;
    allocator_t *allocator  = ((Obj *)text)->allocator;
    List *list              = text->line_info;
    int len, i, line_offset = 0;
    Iterator *end, *cur;
    text_line_t *li;
    char c;
    int c_witdh;
    int ret = 0;

    cur = text->line_info->begin(text->line_info);
    end = text->line_info->end(text->line_info);
    cur = get_iterator_of_nth_line(cur, end, start_line);
    len = strlen(str);

    /*
     *dbg_str(DBG_DETAIL,"rewrite line_count:%d, text:%s", count, str);
     */
    /*
     *dbg_str(DBG_DETAIL,"rewrite_text:offset=%d",offset);
     */

    for (i = 0; line_num < count && i < len; i++) {
        c       = str[i];
        c_witdh = f->get_character_width(f,c);

        if (i == 0) {
            li           = cur->get_vpointer(cur);
            line_offset  = offset;
            x            = width;
        }

        if (x + c_witdh <= line_width) {
            li->string->replace_char(li->string,line_offset, c);
            x  += c_witdh;
        } else if (x + c_witdh > line_width) {//line end
            li->line_lenth = x;
            li->head       = li->string->value;
            li->tail       = li->head + line_offset - 1;
            line_num++;
            /*
             *dbg_str(DBG_DETAIL,"rewrite line:%s", li->head);
             */
            if (line_num < count) {
                cur->next(cur);
                li             = cur->get_vpointer(cur);
                memset(li->string->value, 0 , li->string->value_len);
                line_offset    = 0;
                li->string->replace_char(li->string,line_offset, c);
                x              = c_witdh;

            } else if ( line_num == count) {
                ret = i;
                break;
            }
        }

        if (c == '\n') {
            li->line_lenth  = x;
            li->head        = li->string->value;
            li->tail        = li->head + line_offset;
            ret             = i + 1;
            line_num++;
            break;
        } else if (i == len -1) {
            li->line_lenth  = x;
            li->head        = li->string->value;
            li->tail        = li->head + line_offset;
            ret             = i + 1;
            line_num++;
            break;
        }

        line_offset++;
    }

    if (ret == 0) {
        dbg_str(DBG_WARNNING, "rewrite_text warnning, i=%d, line_num=%d, count=%d, len=%d",
                i, line_num,count, len);
    }

    if (out_line_cnt != NULL)
        *out_line_cnt = line_num;
    if (out_len != NULL)
        *out_len = i;

    object_destroy(cur);
    object_destroy(end);

    return ret;
#undef MAX_TEXT_LINE_LENTH
}

static int __construct(Text *text,char *init_str)
{
    allocator_t *allocator = ((Obj *)text)->allocator;
    char *set_str          = "{\"Linked_List\":{\"List\":{ \"value_size\":%d}}}";
#define MAX_BUF_LEN 1024
    char buf[MAX_BUF_LEN];

    dbg_str(OBJ_DETAIL,"text construct, text addr:%p",text);
    snprintf(buf, MAX_BUF_LEN, set_str, sizeof(text_line_t));

    text->line_info  = OBJECT_NEW(allocator, Linked_List,buf);
    if (!text->line_info) {
        dbg_str(DBG_WARNNING, "create line_info err");
        return -1;
    }

    /*
     *text->content = global_text;
     */
    text->width   = 600;
    object_dump(text->line_info, "Linked_List", buf, MAX_BUF_LEN);
    dbg_str(DBG_DETAIL,"line info dump: %s",buf);

#undef MAX_BUF_LEN
    return 0;
}

static int __deconstrcut(Text *text)
{
    dbg_str(OBJ_DETAIL,"text deconstruct,text addr:%p",text);
    if (text->line_info)
        object_destroy(text->line_info);

    return 0;
}

static int __set(Text *text, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        text->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        text->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        text->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        text->deconstruct = value;
    } else if (strcmp(attrib, "write_text") == 0) {
        text->write_text = value;
    } else if (strcmp(attrib, "write_char") == 0) {
        text->write_char = value;
    } else if (strcmp(attrib, "delete_char") == 0) {
        text->delete_char = value;
    } else if (strcmp(attrib, "get_line_count") == 0) {
        text->get_line_count = value;
    } else if (strcmp(attrib, "get_text_line_info") == 0) {
        text->get_text_line_info = value;
    }
    else {
        dbg_str(OBJ_WARNNING,"text set,  \"%s\" setting is not support",attrib);
    }

    return 0;
}

static void * __get(Text *text, char *attrib)
{
    if (strcmp(attrib, "") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING,"text get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

/**
 * @synopsis __write_text 
 *
 * @param text          
 * @param start_line    write at the line of text
 * @param str           the string to write
 * @param font
 *
 * @returns   
 */
int __write_text(Text *text, int start_line,char *str, void *font)
{
#define MAX_TEXT_LINE_LENTH 256
    int line_width          = text->width;
    int head_offset         = 0;
    int tail_offset         = 0;
    int write_count         = 0;
    int line_lenth          = 0;
    int x                   = 0, y = 0;
    Font *f                 = (Font *)font;
    allocator_t *allocator  = ((Obj *)text)->allocator;
    List *list              = text->line_info;
    int len, i, line_offset = 0;
    Iterator *head ,*end, *cur;
    text_line_t line_info;
    char c;
    int c_witdh, c_height;

    cur = text->line_info->begin(text->line_info);
    end = text->line_info->end(text->line_info);
    cur = get_iterator_of_nth_line(cur, end, start_line);

    memset(&line_info, 0, sizeof(line_info));
    len = strlen(str);

    for (i = 0; i < len; i++) {
        c       = str[i];
        c_witdh = f->get_character_width(f,c);
        if (x == 0) {
            memset(&line_info, 0, sizeof(line_info));
            line_offset           = 0;
            line_info.string      = OBJECT_NEW(allocator, String,NULL);
            line_info.string->pre_alloc(line_info.string, MAX_TEXT_LINE_LENTH);
        }

        if (x + c_witdh > line_width) {//line end
            line_info.line_lenth  = x;
            write_count++;
            line_info.head        = line_info.string->value;
            line_info.tail        = line_info.head + line_offset - 1;
            list->insert_after(list,cur, &line_info);
            cur->next(cur);

            x                     = c_witdh;
            memset(&line_info, 0, sizeof(line_info));
            line_offset           = 0;
            line_info.string      = OBJECT_NEW(allocator, String,NULL);
            line_info.string->pre_alloc(line_info.string, MAX_TEXT_LINE_LENTH);
            line_info.string->append_char(line_info.string,c);

        } else {
            x                        += c_witdh;
            line_info.string->append_char(line_info.string,c);
        }

        if (c == '\n') {
            line_info.line_lenth  = x;
            write_count++;
            line_info.head        = line_info.string->value;
            line_info.tail        = line_info.head + line_offset;
            list->insert_after(list,cur, &line_info);
            cur->next(cur);
            x                     = 0;
        } else if ( i == len - 1) {
            line_info.line_lenth  = x;
            write_count++;
            line_info.head        = line_info.string->value;
            line_info.tail        = line_info.head + line_offset;
            list->insert_after(list,cur, &line_info);
            cur->next(cur);
        }

        line_offset++;
    }

    object_destroy(cur);
    object_destroy(end);

    return write_count;
#undef MAX_TEXT_LINE_LENTH
}

int __write_char(Text *text,int line_num,  int offset, int width, char c,void *font)
{
#define MAX_MODULATE_STR_LEN 1024 * 2
    Iterator *cur, *end;
    text_line_t *line_info;
    int i   = 0;
    int ret = -1;
    char str[MAX_MODULATE_STR_LEN] = {0};
    int line_count, new_line_count;
    int total_len, write_len;

    str[0] = c;
    line_count = extract_text_disturbed_by_inserting(text, line_num, offset, str + 1, MAX_MODULATE_STR_LEN, font);

    total_len = strlen(str);
    /*
     *dbg_str(DBG_DETAIL,"text_line_disturbed_by_inserting, line_count=%d, value:%s",line_count, str);
     */
    write_len = rewrite_text(text, line_num, offset, width,
                             line_count, str,NULL, NULL, font);

#if 1
    /*
     *dbg_str(DBG_DETAIL,"rewrite len=%d, total_len=%d", write_len, total_len);
     */
    /*
     *dbg_str(DBG_DETAIL,"ini str:%s", str);
     *dbg_str(DBG_DETAIL,"left str:%s", str + write_len + 1);
     */
    if (total_len - write_len > 0) {
        new_line_count = text->write_text(text, line_num + line_count -1 ,str + write_len, font);
        text->last_line_num += new_line_count;
        /*
         *ret = line_count + new_line_count;
         */
        ret = text->last_line_num - line_num + 1;
        dbg_str(DBG_IMPORTANT,
                "new a line, line_num=%d,last_line_num=%d new_line_count=%d, line_count=%d,write_len=%d, total_len=%d, value:%s",
                line_num ,text->last_line_num, new_line_count, line_count,write_len, total_len,  str + write_len);
    } else {
        ret = line_count;
    }
#endif

    return ret;
#undef MAX_MODULATE_STR_LEN
}

int __delete_char(Text *text,int line_num,  int offset, int width, void *font)
{
#define MAX_MODULATE_STR_LEN 1024 * 2
    Iterator *cur, *end;
    text_line_t *line_info;
    int i   = 0;
    int ret = -1;
    char str[MAX_MODULATE_STR_LEN] = {0};
    int line_count, new_line_count = 0;
    int total_len, write_len;
    char c;

    line_count = extract_text_disturbed_by_inserting(text, line_num, offset, str, MAX_MODULATE_STR_LEN, font);

    dbg_str(DBG_DETAIL,"delete_char, text_disturbed:%s",str + 1);
    rewrite_text(text, line_num, offset, width, line_count, str + 1, NULL, &new_line_count,  font);

    dbg_str(DBG_DETAIL,"line_count=%d, new_line_count=%d", line_count, new_line_count);
    if (line_count == new_line_count) {
        ret = line_count;
    } else if (line_count - new_line_count == 1) {
        delete_nth_line(text, line_num + line_count - 1);
        ret = text->last_line_num - line_num +  1;
        text->last_line_num--;
    } else {
        dbg_str(DBG_DETAIL,"run at here");
    }

    return ret;
#undef MAX_MODULATE_STR_LEN
}

void *__get_text_line_info(Text *text, int line_num)
{
    Iterator *cur, *end;
    text_line_t *line_info = NULL;
    int i   = 0;

    cur = text->line_info->begin(text->line_info);
    end = text->line_info->end(text->line_info);

    for (i = 0; !end->equal(end,cur); cur->next(cur), i++) {
        if (i == line_num) {
            line_info = cur->get_vpointer(cur);
            break;
        }
    }

    object_destroy(cur);
    object_destroy(end);

    return line_info;
}

int __get_line_count(Text *text)
{
    return text->last_line_num;
}

static class_info_entry_t text_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","write_text",__write_text,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","write_char",__write_char,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","delete_char",__delete_char,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_line_count",__get_line_count,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_text_line_info",__get_text_line_info,sizeof(void *)},
    [10] = {ENTRY_TYPE_NORMAL_POINTER,"","List",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Text",text_class_info);

void print_line_info(Iterator *iter)
{
    LList_Iterator *i      = (LList_Iterator *)iter;
    text_line_t *line_info = i->get_vpointer(iter);

    dbg_str(DBG_DETAIL,"head=%p,tail =%p,data =%s", 
            line_info->head, line_info->tail, line_info->head);
}

void test_obj_text()
{
    Text *text;
    allocator_t *allocator = allocator_get_default_alloc();

    dbg_str(DBG_DETAIL,"test_obj_text");
    text = OBJECT_NEW(allocator, Text,"");

}


