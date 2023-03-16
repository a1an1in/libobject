#ifndef __NODE_CLI_COMMAND_H__
#define __NODE_CLI_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Node_Cli_s Node_Cli;

struct Node_Cli_s{
	Command parent;

	int (*construct)(Node_Cli *command,char *init_str);
	int (*deconstruct)(Node_Cli *command);
	int (*set)(Node_Cli *command, char *attrib, void *value);
    void *(*get)(Node_Cli *command, char *attrib);
    char *(*to_json)(Node_Cli *command); 

	int (*run_command)(Node_Cli *command);
};

#endif
