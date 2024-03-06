#ifndef __Node_H__
#define __Node_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>

typedef struct Node_s Node;

struct Node_s {
	Obj parent;

	int (*construct)(Node *node,char *init_str);
	int (*deconstruct)(Node *node);
	int (*set)(Node *node, char *attrib, void *value);
    void *(*get)(Node *node, char *attrib);
    char *(*to_json)(Node *node);
	int (*init)(Node *node);
	int (*loop)(Node *node);
	int (*call)(Node *node, char *code, void *out, uint32_t *out_len);
	int (*write)(Node *node, char *from, char *node_id, char *to);
	int (*read)(Node *node, char *node_id, char *from, char *to);
	int (*copy)(Node *node, char *from, char *to);
	int (*list)(Node *node, char *path);

	bus_t *bus;
    busd_t *busd;
	char *host, *service;
    int run_bus_deamon_flag; /* 1表示要运行bus deamon */
	int disable_node_service_flag;
    int node_flag; /* flag == 1 表示退出node */
	String *str;
};

extern bus_object_t node_object;

int node_find_method_argument_template(bus_object_t *obj, allocator_t *allocator, char *method_name, 
                                       bus_method_args_t **args, int *cnt);
int node_free_argument_template(allocator_t *allocator, bus_method_args_t *args, int count);
#endif
