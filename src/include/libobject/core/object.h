#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <libobject/basic_types.h>
#include <libobject/core/class_info.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/json/cjson.h>

#define OBJECT_FALSE   CJSON_FALSE  
#define OBJECT_TRUE    CJSON_TRUE   
#define OBJECT_NULL    CJSON_NULL   
#define OBJECT_NUMBER  CJSON_NUMBER 
#define OBJECT_STRING  CJSON_STRING 
#define OBJECT_ARRAY   CJSON_ARRAY  

#define MAX_OBJECT_INIT_DATA_LEN 1024 * 10 * 2

class_info_entry_t * __object_get_entry_of_class(void *class_info, char *entry_name);
class_info_entry_t * __object_get_entry_of_parent_class(void *class_info_addr);
void * __object_get_lastest_vitual_func(void *class_info_addr, char *func_name);

class_info_entry_t * object_get_subclass_info(void *class_info_addr);
class_info_entry_t *object_get_entry_of_class(char *class_name, char *entry_name);
int object_init(void *obj, char *type_name);
int object_set(void *obj, char *type_name, char *set_str);
int object_dump(void *obj, char *type_name, char *buf, int max_len);
void *object_new(allocator_t *allocator, const char *type, char *config);
int object_destroy(void *obj);
int object_override(void *obj, char *func_name, void *value);
void *object_get_member_of_class(char *class_name, char *func_name);
void *object_get_progeny_class_first_private_func(char *sub_class_name, char *root_class_name, char *func_name);
void *object_get_lastest_virtual_func(char *start_type_name, char *end_type_name, char *method_name);
int object_call_method(void *object, char *func_str);
void * object_get_lastest_vitual_func(void *class_name, char *func_name);

#define OBJECT_ALLOC(alloc, type)                                                   \
({                                                                                  \
	type * obj;                                                                     \
	                                                                                \
	obj = (type *)allocator_mem_alloc(alloc,sizeof(type));                          \
	if(obj == NULL) {                                                               \
		dbg_str(DBG_DETAIL,"allocator_mem_alloc");                                  \
		obj = NULL;                                                                 \
	} else {                                                                        \
		memset(obj,0, sizeof(type));                                                \
		((Obj *)obj)->allocator = alloc;                                            \
        strcpy(((Obj *)obj)->name,#type);                                           \
	}                                                                               \
	                                                                                \
	obj;                                                                            \
 })

#define OBJECT_NEW(allocator,type,set_str)                                          \
({                                                                                  \
    void *obj = NULL;                                                               \
    int ret;                                                                        \
    obj = OBJECT_ALLOC(allocator,type);                                             \
    ret = object_set(obj, #type, set_str);                                          \
    if( ret < 0) {                                                                  \
        dbg_str(DBG_ERROR,"object_set error");                                      \
        obj = NULL;                                                                 \
    } else {                                                                        \
        object_init(obj,#type);                                                     \
    }                                                                               \
    obj;                                                                            \
})

#define offset_of_class(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define OBJECT_DESTROY(obj)                                                         \
({                                                                                  \
    if (obj != NULL) {                                                              \
        object_destroy(obj);                                                        \
        obj = NULL;                                                                 \
    }                                                                               \
})


typedef struct object_config_s{
}object_config_t;

#endif
