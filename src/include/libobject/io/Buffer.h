#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Stream.h>

typedef struct buffer_s Buffer;

enum buffer_operation_flag_s{
    BUFFER_WRITE_OPERATION = 1,
    BUFFER_READ_OPERATION,
};

struct buffer_s{
	Stream parent;

	int (*construct)(Buffer *,char *init_str);
	int (*deconstruct)(Buffer *);
	int (*set)(Buffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*buffer_read)(Stream *, void *dst, int len);
    int (*buffer_write)(Stream *, void *src, int len);
    int (*size)(Buffer *);
    int (*is_empty)(Buffer*);
    /*int (*printf)(Buffer *buffer, const char *fmt, ...);
     *int (*memcopy)(Buffer *buffer, void *addr, int len);
     */
    void (*clear)(Buffer*);
    int (*buffer_move_unref)(Buffer *,Buffer*,size_t len);
    int (*buffer_copy_ref)(Buffer*,Buffer *);
    int (*buffer_copy)(Buffer*,Buffer *,size_t len);
    int (*buffer_find)(Buffer*,u_char c);
    void (*buffer_destroy)(Buffer*);
    int (*buffer_used_size)(Buffer*);
    int (*buffer_free_size)(Buffer*);
    
    void (*buffer_adapter_internal)(Buffer*);
    int  (*buffer_expand_container)(Buffer*);
    /*attribs*/
    void   *buffer;
    int    r_offset;
    int    w_offset; 
    int    used_size;      //已经使用的空间
    int    available_size; //未使用的空间
    int    capacity;           //总大小
    size_t ref_count;
    uint8_t last_operation_flag;
};

#endif
