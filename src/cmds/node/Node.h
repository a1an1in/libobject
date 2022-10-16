#ifndef __NODE_COMMAND_H__
#define __NODE_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Node_Command_s Node_Command;

struct Node_Command_s{
	Command parent;

	int (*construct)(Node_Command *command,char *init_str);
	int (*deconstruct)(Node_Command *command);
	int (*set)(Node_Command *command, char *attrib, void *value);
    void *(*get)(Node_Command *command, char *attrib);
    char *(*to_json)(Node_Command *command); 

	int (*run_action)(Node_Command *command);
};

#endif
