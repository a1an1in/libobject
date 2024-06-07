#ifndef __Node_H__
#define __Node_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/core/Map.h>
#include <libobject/core/utils/data_structure/list.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>

typedef struct Node_s Node;

typedef enum target_type_e {
	TARGET_TYPE_NODE = 0,
	TARGET_TYPE_ATTACHER,
} target_type_t;

typedef struct node_malloc_variable_info_s {
	char name[32];
	uint32_t value_type;
	void *free_func;
	char addr[0];
} node_malloc_variable_info_t;

struct Node_s {
	Obj parent;

	int (*construct)(Node *node,char *init_str);
	int (*deconstruct)(Node *node);
	int (*set)(Node *node, char *attrib, void *value);
    void *(*get)(Node *node, char *attrib);
    char *(*to_json)(Node *node);
	int (*init)(Node *node);
	int (*loop)(Node *node);
	int (*call_bus)(Node *node, char *code, void *out, uint32_t *out_len);
	int (*call_fsh)(Node *node, char *code, void *out, uint32_t *out_len);
	int (*write_file)(Node *node, char *from, char *node_id, char *to);
	int (*read_file)(Node *node, char *node_id, char *from, char *to);
	int (*copy)(Node *node, char *from, char *to);
	int (*list)(Node *node, char *node_id, char *path, Vector *vector);
	int (*malloc)(Node *node, char *node_id, target_type_t type, int value_type, char *class_name, char *name, int size, void **addr);
	int (*mfree)(Node *node, char *node_id, target_type_t type, int value_type, void *addr, char *name);
	int (*mset)(Node *node, char *node_id, target_type_t type, void *addr, int offset, int len, void *value, int value_len);
	int (*mget)(Node *node, char *node_id, target_type_t type, void *addr, int offset, int len, void *value, int *value_len);

	bus_t *bus;
    busd_t *busd;
	char *host, *service;
	/* 1表示要运行bus deamon */
    int run_bus_deamon_flag;
	int disable_node_service_flag;
	/* flag == 1 表示退出node */
    int node_flag; 
	String *str;
	/*
	 * node分配的变量，node_cli可以共享，不允许名字重复。
	 **/
	Map *variable_map;
};

extern bus_object_t node_object;

int node_find_method_argument_template(bus_object_t *obj, allocator_t *allocator, char *method_name, 
                                       bus_method_args_t **args, int *cnt);
int node_free_argument_template(allocator_t *allocator, bus_method_args_t *args, int count);

#endif
