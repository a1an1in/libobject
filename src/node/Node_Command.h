#ifndef __NODE_CENTER_COMMAND_H__
#define __NODE_CENTER_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>

typedef struct Node_Command_s Node_Command;

struct Node_Command_s {
    Command parent;

    int (*construct)(Node_Command *command,char *init_str);
    int (*deconstruct)(Node_Command *command);
    int (*set)(Node_Command *command, char *attrib, void *value);
    void *(*get)(Node_Command *command, char *attrib);
    char *(*to_json)(Node_Command *command); 

    int (*run_command)(Node_Command *command);

    char *host, *service;
    int bus_deamon_flag; /* 1表示要运行bus deamon */
    int node_flag; /* flag == 1 表示退出node */
    bus_t *bus;
    busd_t *busd;
};


extern struct bus_object debug_object;
extern struct bus_object test_object;

#endif
