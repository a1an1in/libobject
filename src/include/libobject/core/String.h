#ifndef __STRING_H__
#define __STRING_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>


/**
 * note:this class String is not thread-safe. if u
 * wanna put it into multi-thread-env.u should keep it
 * thread safe by uself.
 * ***/
typedef struct string_s String;

struct string_s{
    Obj obj;

    int (*construct)(String *string,char *init_str);
    int (*deconstruct)(String *string);
    int (*set)(String *string, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*modulate_capacity)(String *string, int write_len);
    String *(*pre_alloc)(String *string,uint32_t size);
    String *(*assign)(String *string,char *s);
    int (*equal)(String *string,char *s);
    void (*format)(String *, int max_len, char *fmt, ...);
    void (*append)(String *string, char *sub, int len);
    String *(*append_char)(String *string,char c);
    String *(*replace_char)(String *string,int index, char c);
    char (*at)(String *string,int index);
    void (*toupper)(String*);
    void (*tolower)(String *);
    void (*ltrim)(String *);
    void (*rtrim)(String *);
    void (*trim)(String *);
    int  (*find)(String *string, char *, int pos, int max);
    String * (*get_substring)(String  *string,int pos,int len);  
    char *(*get_cstr)(String *);
    size_t (*get_len)(String *);
    void (*reset)(String *);
    int (*is_empty)(String *);
    int (*replace)(String *,char *oldstr,char * newstr, int max);
    String *(*insert)(String * dest,size_t index,char * src);
    int (*split)(String *string, char *delims, int num);
    char *(*get_splited_cstr)(String *, int);
    char *(*get_found_cstr)(String *, int);

    char *value;
    int value_max_len;
    int value_len;
    void *pieces;
};

#define STR2A(string) string->get_cstr(string)  

#endif
