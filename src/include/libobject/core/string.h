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
    
    // toupper converts String to upperString into s but original String
    // keep stable.uppper1 impacts original String.keep unstable
    void (*toupper)(String *,String *);
    void (*toupper_impact)(String*);
    void (*tolower)(String *,String *);
    void (*tolower_impact)(String *);
    // remove free space from left side of String
    
    void (*ltrim)(String *);
    void (*rtrim)(String *);
    void (*trim)(String *);

    /**
     * @brief split_string 将字符串string以separator分割保存至vector
     *
     * @param str 待转换的字符串
     * @param separator 
     * @param vector 
     * @see
     * @note
     * @author  wuyujie[1683358846@q.com]
     * @date    2018/11/21
     */
    void (*split_string)(String *,String *separator,Vector *vector);
    /**
     * @brief find   查找子串
     *
     * @param substr 待查找的子串
     * @param pos    起始查找位置
     * @param string 
     * @return        失败返回-1
     * @see
     * @note
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    int  (*find)(String *string,String *substr,int pos);
    int  (*find_cstr)(String *string,char *substr,int pos);
    /**
     * @brief substr  截取子串
     *
     * @param string  母串 
     * @param pos     起始位置
     * @param len     查找长度
     * @return        
     * @see
     * @note
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    String * (*substr)(String  *string,int pos,int len);  
     /**
     * @brief c_str  String串到C风格串 
     *
     * @param string  母串 
     * @return        
     * @see
     * @note    ref_value. don't delete p=c_str() otherwise oringal String 's avction may unvalid
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    char *(*c_str)(String *);
    /**
     * @brief append_str  追加c风格串
     *
     * @param string  母串 
     * @return        
     * @see
     * @note    
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    void (*append_str)(String *,char *);
     /**
     * @brief append_str_len  追加c风格串指定长度的串
     *
     * @param string  母串 
     * @return        
     * @see
     * @note    
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    void (*append_str_len)(String *,char *,int );
    /**
     * @brief append_Str  追加String风格的串
     *
     * @param string  母串 
     * @return        
     * @see
     * @note    
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    void (*append_objective_string)(String *,String *);
    /**
     * @brief size     获取String实际存储串有效长度
     *
     * @param string  母串 
     * @return        
     * @see
     * @note    
     * @author  wuyujie[1683358846@qq.com]
     * @date    2018/11/21
     */
    size_t  (*size)(String *);
    void    (*clear)(String *);
    int     (*is_empty)(String *);
    String *(*replace_all)(String *,char *oldstr,char * newstr);
    String *(*replace)(String *,char *oldstr,char *newstr);

    String *(*insert_char_count)(String *,size_t pos,char c,size_t count);
    String *(*insert_cstr_normal)(String *,size_t pos,char *cstr);
    String *(*insert_str_normal)(String *,size_t pos,String *cstr);
    String *(*insert_cstr)(String * dest,size_t index,char * src,size_t pos,size_t count);
    String *(*insert_str)(String * dest,size_t index,String * src,size_t pos,size_t count);
    int    (*compare)(String *self,String *);
    int    (*compare_ignore)(String *self,String *);
    int    (*equal)(String *self,String *);
    int    (*equal_ignore)(String *self,String *);

    int    (*compare_cstr)(String *self,char *);
    int    (*compare_ignore_cstr)(String *self,char *);
    int    (*equal_cstr)(String *self,char *);
    int    (*equal_ignore_cstr)(String *self,char *);

    String *(*copy)(String *);
	/*virtual methods reimplement*/

    
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    char *value;
    int value_max_len;
    int value_len;
};

#endif
