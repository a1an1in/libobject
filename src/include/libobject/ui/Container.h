#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdio.h>
#include <libobject/core/String.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Subject.h>
#include <libobject/core/Map.h>

typedef struct container_s Container;

typedef struct position_s {
    int x;
    int y;
} position_t;

struct container_s {
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
    void (*for_each_component)(Container *obj, void (*func)(void *key, void *element, void *arg), void *arg);

    String *name;
    Map *map;
    char *map_construct_str;
    uint8_t map_type;
};

#endif
