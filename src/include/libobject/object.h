#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <libobject/class_info.h>
#include <libobject/obj.h>
#include <libjson/cjson.h>

#define OBJECT_FALSE   CJSON_FALSE  
#define OBJECT_TRUE    CJSON_TRUE   
#define OBJECT_NULL    CJSON_NULL   
#define OBJECT_NUMBER  CJSON_NUMBER 
#define OBJECT_STRING  CJSON_STRING 
#define OBJECT_ARRAY   CJSON_ARRAY  

void * object_get_set_func_pointer(void *class_info_addr);
int object_init_func_pointer(void *obj,void *class_info_addr);
class_info_entry_t * object_get_subclass_info(void *class_info_addr);
int object_init(void *obj, char *type_name);
int object_set(void *obj, char *type_name, char *set_str);
int object_dump(void *obj, char *type_name, char *buf, int max_len);
int object_destroy(void *obj);

#define OBJECT_ALLOC(alloc, type) \
({\
	type * obj; \
	\
	obj = (type *)allocator_mem_alloc(alloc,sizeof(type));\
	if(obj == NULL) {\
		dbg_str(DBG_DETAIL,"allocator_mem_alloc");\
		obj = NULL;\
	} else { \
		memset(obj,0, sizeof(type));\
		((Obj *)obj)->allocator = alloc;\
        strcpy(((Obj *)obj)->name,#type);\
	}\
	\
	obj;\
 })

#define OBJECT_NEW(allocator,type,set_str) \
({\
    void *obj;\
    int ret;\
    obj = OBJECT_ALLOC(allocator,type);\
    ret = object_set(obj, #type, set_str);\
    if( ret < 0) {\
        dbg_str(DBG_ERROR,"object_set error");\
        obj = NULL;\
    } else {\
        object_init(obj,#type);\
    }\
    obj;\
})

#endif
