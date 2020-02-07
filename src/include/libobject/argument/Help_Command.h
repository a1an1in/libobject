#ifndef __TEST_COMMAND_H__
#define __TEST_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Help_Command_s Help_Command;

struct Help_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	int (*run_action)(Command *command);
};

#endif
