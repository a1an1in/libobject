#ifndef __OBJ_DEAMON_H__
#define __OBJ_DEAMON_H__

#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/data_structure/map.h>
#include <libobject/core/class_info.h>
#include <libobject/attrib_priority.h>
#include <libobject/core/utils/registry/registry.h>

#define REGISTER_CLASS(class_name, class_info) \
__attribute__((constructor)) static void register##class_name()\
{\
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register class %s\n",\
                  REGISTRY_CTOR_PRIORITY_REGISTER_CLASS, #class_name);\
    \
    __register_ctor_func3(REGISTRY_CTOR_PRIORITY_REGISTER_CLASS,\
            (int (*)(void *, void * , void *))class_deamon_register_class, NULL, #class_name, class_info);\
}

#define PLUGIN_DEFINE_CLASS_REGISTER(class_name, class_info) \
__attribute__((constructor)) static void register##class_name()\
{\
	printf("plugin register class, name:%s, addr:%p\n", #class_name, class_info);\
	class_deamon_register_class(NULL, #class_name, class_info);\
}

#define PLUGIN_DEFINE_CLASS_DEREGISTER(class_name) \
__attribute__((destructor)) static void deregister##class_name()\
{\
	class_deamon_deregister_class(NULL, #class_name);\
}

typedef struct class_deamon_s{
	allocator_t *allocator;
	map_t *map;
	char map_type;
	uint8_t map_key_len;
	uint8_t map_value_size;
}class_deamon_t;

extern class_deamon_t *global_class_deamon;

int class_deamon_register_class(class_deamon_t *class_deamon,
							  char *class_name,
							  void *class_info_addr);
int class_deamon_deregister_class(class_deamon_t *class_deamon, char *class_name);
void *class_deamon_search_class(class_deamon_t *class_deamon, char *class_name);
void *class_deamon_search_class_name_addr(class_deamon_t *class_deamon, char *class_name);
class_deamon_t *class_deamon_get_global_class_deamon();
int class_deamon_info(class_deamon_t *class_deamon);
#endif
