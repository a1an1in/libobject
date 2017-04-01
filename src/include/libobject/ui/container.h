#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/ui/subject.h>
#include <libobject/map.h>

typedef struct container_s Container;

typedef struct position_s{
	int x;
	int y;
}position_t;

struct container_s{
	Subject subject;

	int (*construct)(Container *container,char *init_str);
	int (*deconstruct)(Container *container);
	int (*set)(Container *container, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
	int (*move)(Container *container);
    int (*add_component)(Container *obj, void *pos, void *component);
    int (*update_component_position)(void *component, void *arg);
    int (*reset_component_position)(void *component, void *arg);
    void *(*search_component)(Container *obj, char *key);
	void (*for_each_component)(Container *obj, void (*func)(Iterator *iter, void *args), void *arg);

#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    Map *map;
    char *map_construct_str;
    uint8_t map_type;
};

#endif
