#ifndef __NODE_CLI_COMMAND_H__
#define __NODE_CLI_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/node/Node.h>

typedef struct Node_Cli_Command_s Node_Cli_Command;

enum command_type_e {
    COMMAND_TYPE_BUS_CALL = 0,
    COMMAND_TYPE_COPY,
    COMMAND_TYPE_LIST,
    COMMAND_TYPE_EXIT,
    COMMAND_TYPE_FSH_CALL,
    COMMAND_TYPE_MAX
};

struct Node_Cli_Command_s {
    Command parent;

    int (*construct)(Node_Cli_Command *command,char *init_str);
    int (*deconstruct)(Node_Cli_Command *command);
    int (*set)(Node_Cli_Command *command, char *attrib, void *value);
    void *(*get)(Node_Cli_Command *command, char *attrib);
    char *(*to_json)(Node_Cli_Command *command); 

    int (*run_command)(Node_Cli_Command *command);

    char *host, *service, *code;
    char *arg1, *arg2;
    Node *node;
    int command_type;
};

#endif
