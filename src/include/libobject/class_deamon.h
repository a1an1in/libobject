#ifndef __OBJ_DEAMON_H__
#define __OBJ_DEAMON_H__

#include <liballoc/allocator.h>
#include <libdata_structure/map.h>
#include <libobject/class_info.h>
#include <attrib_priority.h>

#define REGISTER_CLASS(class_name, class_info) \
    __attribute__((constructor(ATTRIB_PRIORITY_REGISTER_CLASS))) static void\
    register_class()\
    {\
        class_deamon_t *deamon = class_deamon_get_global_class_deamon();\
        \
        ATTRIB_PRINT("constructor ATTRIB_PRIORITY_REGISTER_CLASS=%d,class name %s\n",\
                     ATTRIB_PRIORITY_REGISTER_CLASS, class_name);\
        \
        class_deamon_register_class(deamon,class_name, class_info);\
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
