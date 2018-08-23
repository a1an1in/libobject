#ifndef __OBJ_DEAMON_H__
#define __OBJ_DEAMON_H__

#include <libobject/utils/alloc/allocator.h>
#include <libobject/utils/data_structure/map.h>
#include <libobject/core/class_info.h>
#include <libobject/attrib_priority.h>

#define REGISTER_CLASS(class_name, class_info) \
    __attribute__((constructor)) static void register_class()\
    {\
        ATTRIB_PRINT("constructor ATTRIB_PRIORITY_REGISTER_CLASS=%d,class name %s\n",\
                     ATTRIB_PRIORITY_REGISTER_CLASS, class_name);\
        \
        __register_init_func3(ATTRIB_PRIORITY_REGISTER_CLASS,\
                class_deamon_register_class, NULL, class_name, class_info);\
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

void * class_deamon_search_class(class_deamon_t *class_deamon, char *class_name);
class_deamon_t *class_deamon_get_global_class_deamon();
#endif
