#ifndef __STRING_H__
#define __STRING_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/vector.h>



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
    String *(*pre_alloc)(String *string,uint32_t size);
    String *(*assign)(String *string,char *s);
    String *(*assign_char)(String *,char c,size_t count );
    String *(*append_char)(String *string,char c);
	String *(*replace_char)(String *string,int index, char c);
    char (*at)(String *string,int index);
    void (*toupper)(String *,String *);
    void (*toupper_impact)(String*);
    void (*tolower)(String *,String *);
    void (*tolower_impact)(String *);
    void (*ltrim)(String *);
    void (*rtrim)(String *);
    void (*trim)(String *);
    int  (*find)(String *string, char *substr,int pos);
    String * (*substr)(String  *string,int pos,int len);  
    char *(*get_cstr)(String *);
    void (*append_cstr)(String *,char *);
    void (*append_string)(String *,String *);
    size_t (*len)(String *);
    void (*clear)(String *);
    int (*is_empty)(String *);
    String *(*replace_all)(String *,char *oldstr,char * newstr);
    String *(*replace)(String *,char *oldstr,char *newstr);
    String *(*insert_cstr)(String * dest,size_t index,char * src);
    String *(*insert_str)(String * dest,size_t index,String * src);
    int (*split_string)(String *, char *);
    char * (*get_splited_string)(String *, int);
    
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    char *value;
    int value_max_len;
    int value_len;
    Vector *splited_strings;
};

#endif
