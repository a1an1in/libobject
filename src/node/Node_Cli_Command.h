#ifndef __NODE_CLI_COMMAND_H__
#define __NODE_CLI_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Node_Cli_Command_s Node_Cli_Command;

struct Node_Cli_Command_s {
    Command parent;

    int (*construct)(Node_Cli_Command *command,char *init_str);
    int (*deconstruct)(Node_Cli_Command *command);
    int (*set)(Node_Cli_Command *command, char *attrib, void *value);
    void *(*get)(Node_Cli_Command *command, char *attrib);
    char *(*to_json)(Node_Cli_Command *command); 

    int (*run_command)(Node_Cli_Command *command);

    char *host, *service;
};

#endif
