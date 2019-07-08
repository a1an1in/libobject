#ifndef __OBJ_LIST_H__
#define __OBJ_LIST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Iterator.h>

typedef struct _list_s List;

struct _list_s{
	Obj obj;

	int (*construct)(List *list,char *init_str);
	int (*deconstruct)(List *list);

	/*virtual methods reimplement*/
	int (*set)(List *list, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add)(List *list, void *value);
    int (*add_front)(List *list,void *value);
    int (*add_back)(List *list,void *value);
    int (*delete)(List *list,Iterator *iter);
    int (*remove)(List *list, void **data);
    int (*remove_front)(List *list, void **data);
    int (*remove_back)(List *list, void **data);
    int (*remove_element)(List *list, void *data);
    int (*clear)(List *list);
    int (*count)(List *list);
    int (*is_empty)(List *list);
    int (*detach_front)(List *list,Iterator *iter);
    int (*free_detached)(List *list,Iterator *iter);
    void (*for_each)(List *list,void (*func)(void *element));
    void (*for_each_arg)(List *list,void (*func)(void *element, void *arg),void *arg);
    Iterator *(*begin)(List *list);
    Iterator *(*end)(List *list);
    int (*destroy)(List *list);

    Iterator *b, *e;
    uint8_t trustee_flag, value_type;

};

#endif
