/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef __CJSON_H__
#define __CJSON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* cjson_t Types: */
#define CJSON_FALSE  (1 << 0)
#define CJSON_TRUE   (1 << 1)
#define CJSON_NULL   (1 << 2)
#define CJSON_NUMBER (1 << 3)
#define CJSON_STRING (1 << 4)
#define CJSON_ARRAY  (1 << 5)
#define CJSON_OBJECT (1 << 6)

#define CJSON_ISREFERENCE 256
#define CJSON_STRINGISCONST 512

/* The cjson_t structure: */
typedef struct cjson_s cjson_t;
struct cjson_s
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/get_array_item/get_object_item */
    cjson_t *next;
    cjson_t *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    cjson_t *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==CJSON_STRING */
    char *valuestring;
    /* The item's number, if type==CJSON_NUMBER */
    int valueint;
    /* The item's number, if type==CJSON_NUMBER */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} ;

typedef struct cjson_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cjson_Hooks;

/* Supply malloc, realloc and free functions to cjson_t */
extern void cjson_init_hooks(cjson_Hooks* hooks);


/* Supply a block of JSON, and this returns a cjson_t object you can interrogate. Call cjson_delete when finished. */
extern cjson_t *cjson_parse(const char *value);
/* Render a cjson_t entity to text for transfer/storage. Free the char* when finished. */
extern char  *cjson_print(const cjson_t *item);
/* Render a cjson_t entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *cjson_print_unformatted(const cjson_t *item);
/* Render a cjson_t entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *cjson_print_bufferd(const cjson_t *item, int prebuffer, int fmt);
/* Delete a cjson_t entity and all subentities. */
extern void   cjson_delete(cjson_t *c);

/* Returns the number of items in an array (or object). */
extern int	  cjson_get_array_size(const cjson_t *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern cjson_t *cjson_get_array_item(const cjson_t *array, int item);
/* Get item "string" from object. Case insensitive. */
extern cjson_t *cjson_get_object_item(const cjson_t *object, const char *string);
extern int cjson_has_object_item(const cjson_t *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cjson_parse() returns 0. 0 when cjson_parse() succeeds. */
extern const char *cjson_get_error_ptr(void);

/* These calls create a cjson_t item of the appropriate type. */
extern cjson_t *cjson_create_null(void);
extern cjson_t *cjson_create_true(void);
extern cjson_t *cjson_create_false(void);
extern cjson_t *cjson_create_bool(int b);
extern cjson_t *cjson_create_number(double num);
extern cjson_t *cjson_create_string(const char *string);
extern cjson_t *cjson_create_array(void);
extern cjson_t *cjson_create_object(void);

/* These utilities create an Array of count items. */
extern cjson_t *cjson_create_int_array(const int *numbers, int count);
extern cjson_t *cjson_create_float_array(const float *numbers, int count);
extern cjson_t *cjson_create_double_array(const double *numbers, int count);
extern cjson_t *cjson_create_string_array(const char **strings, int count);

/* Append item to the specified array/object. */
extern void cjson_add_item_to_array(cjson_t *array, cjson_t *item);
extern void	cjson_add_item_to_object(cjson_t *object, const char *string, cjson_t *item);
extern void	cjson_add_item_to_objectCS(cjson_t *object, const char *string, cjson_t *item);	/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the cjson_t object */
/* Append reference to item to the specified array/object. Use this when you want to add an existing cjson_t to a new cjson_t, but don't want to corrupt your existing cjson_t. */
extern void cjson_add_item_reference_to_array(cjson_t *array, cjson_t *item);
extern void	cjson_add_item_reference_to_object(cjson_t *object, const char *string, cjson_t *item);

/* Remove/Detatch items from Arrays/Objects. */
extern cjson_t *cjson_detach_item_from_array(cjson_t *array, int which);
extern void   cjson_delete_item_from_array(cjson_t *array, int which);
extern cjson_t *cjson_detach_item_from_object(cjson_t *object, const char *string);
extern void   cjson_delete_item_from_object(cjson_t *object, const char *string);

/* Update array items. */
extern void cjson_insert_item_in_array(cjson_t *array, int which, cjson_t *newitem); /* Shifts pre-existing items to the right. */
extern void cjson_replace_item_in_array(cjson_t *array, int which, cjson_t *newitem);
extern void cjson_replace_item_in_object(cjson_t *object,const char *string,cjson_t *newitem);

/* Duplicate a cjson_t item */
extern cjson_t *cjson_duplicate(const cjson_t *item, int recurse);
/* Duplicate will create a new, identical cjson_t item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error. If not, then cjson_get_error_ptr() does the job. */
extern cjson_t *cjson_parse_with_opts(const char *value, const char **return_parse_end, int require_null_terminated);

extern void cjson_minify(char *json);

/* Macros for creating things quickly. */
#define cjson_add_null_to_object(object,name) cjson_add_item_to_object(object, name, cjson_create_null())
#define cjson_add_true_to_object(object,name) cjson_add_item_to_object(object, name, cjson_create_true())
#define cjson_add_false_to_object(object,name) cjson_add_item_to_object(object, name, cjson_create_false())
#define cjson_add_bool_to_object(object,name,b) cjson_add_item_to_object(object, name, cjson_create_bool(b))
#define cjson_add_number_to_object(object,name,n) cjson_add_item_to_object(object, name, cjson_create_number(n))
#define cjson_add_string_to_object(object,name,s) cjson_add_item_to_object(object, name, cjson_create_string(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cjson_set_int_value(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))
#define cjson_set_number_value(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))

/* Macro for iterating over an array */
#define cjson_array_for_each(pos, head) for(pos = (head)->child; pos != NULL; pos = pos->next)

#ifdef __cplusplus
}
#endif

#endif
