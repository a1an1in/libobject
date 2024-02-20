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

	bus_t *bus;
    busd_t *busd;
	char *host, *service;
    int bus_deamon_flag; /* 1表示要运行bus deamon */
    int node_flag; /* flag == 1 表示退出node */
};

extern struct bus_object node_object;

#endif
