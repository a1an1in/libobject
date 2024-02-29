#ifndef __NODE_CENTER_COMMAND_H__
#define __NODE_CENTER_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include "Node.h"

typedef struct Node_Server_Command_s Node_Server_Command;

struct Node_Server_Command_s {
    Command parent;

    int (*construct)(Node_Server_Command *command,char *init_str);
    int (*deconstruct)(Node_Server_Command *command);
    int (*set)(Node_Server_Command *command, char *attrib, void *value);
    void *(*get)(Node_Server_Command *command, char *attrib);
    char *(*to_json)(Node_Server_Command *command); 

    int (*run_command)(Node_Server_Command *command);

    Node *node;
};

#endif
