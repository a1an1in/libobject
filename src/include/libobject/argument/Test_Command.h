#ifndef __TEST_SUB1_COMMAND_H__
#define __TEST_SUB1_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Test_Command_s Test_Command;

struct Test_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	/*virtual methods reimplement*/
    void * (*get_value)(Command *command,char *command_name, char *flag_name);

    int help;
    String *option;
};

#endif
